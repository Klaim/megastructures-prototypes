// Sources: 
// - https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2022/p2502r2.pdf
// - https://godbolt.org/z/5hcaPcfvP

#include <algorithm>
#include <cassert>
#include <coroutine>
#include <cstdint>
#include <cstring>
#include <exception>
#include <memory>
#include <new>
#include <ranges>
#include <type_traits>
#include <utility>

#ifdef _MSC_VER
#define EMPTY_BASES __declspec(empty_bases)
#ifdef __clang__
#define NO_UNIQUE_ADDRESS
#else
#define NO_UNIQUE_ADDRESS [[msvc::no_unique_address]]
#endif
#else
#define EMPTY_BASES
#define NO_UNIQUE_ADDRESS [[no_unique_address]]
#endif

namespace std { // NOTE: this is not allowed by the standard, DO NOT USE THIS IN A REAL PROJECT
    struct alignas(__STDCPP_DEFAULT_NEW_ALIGNMENT__) _Aligned_block {
        unsigned char _Pad[__STDCPP_DEFAULT_NEW_ALIGNMENT__];
    };

    template <class _Alloc>
    using _Rebind = typename allocator_traits<_Alloc>::template rebind_alloc<_Aligned_block>;

    template <class _Alloc>
    concept _Has_real_pointers =
        same_as<_Alloc, void> || is_pointer_v<typename allocator_traits<_Alloc>::pointer>;

    template <class _Allocator = void>
    class _Promise_allocator { // statically specified allocator type
    private:
        using _Alloc = _Rebind<_Allocator>;

        static void* _Allocate(_Alloc _Al, const size_t _Size) {
            if constexpr (default_initializable<
                              _Alloc> && allocator_traits<_Alloc>::is_always_equal::value) {
                // do not store stateless allocator
                const size_t _Count = (_Size + sizeof(_Aligned_block) - 1) / sizeof(_Aligned_block);
                return _Al.allocate(_Count);
            } else {
                // store stateful allocator
                static constexpr size_t _Align =
                    (::std::max)(alignof(_Alloc), sizeof(_Aligned_block));
                const size_t _Count =
                    (_Size + sizeof(_Alloc) + _Align - 1) / sizeof(_Aligned_block);
                void* const _Ptr = _Al.allocate(_Count);
                const auto _Al_address =
                    (reinterpret_cast<uintptr_t>(_Ptr) + _Size + alignof(_Alloc) - 1)
                    & ~(alignof(_Alloc) - 1);
                ::new (reinterpret_cast<void*>(_Al_address)) _Alloc(::std::move(_Al));
                return _Ptr;
            }
        }

    public:
        static void* operator new(const size_t _Size) requires default_initializable<_Alloc> {
            return _Allocate(_Alloc{}, _Size);
        }

        template <class _Alloc2, class... _Args>
            requires convertible_to<const _Alloc2&, _Allocator>
        static void* operator new(
            const size_t _Size, allocator_arg_t, const _Alloc2& _Al, const _Args&...) {
            return _Allocate(static_cast<_Alloc>(static_cast<_Allocator>(_Al)), _Size);
        }

        template <class _This, class _Alloc2, class... _Args>
            requires convertible_to<const _Alloc2&, _Allocator>
        static void* operator new(const size_t _Size, const _This&, allocator_arg_t,
            const _Alloc2& _Al, const _Args&...) {
            return _Allocate(static_cast<_Alloc>(static_cast<_Allocator>(_Al)), _Size);
        }

        static void operator delete(void* const _Ptr, const size_t _Size) noexcept {
            if constexpr (default_initializable<
                              _Alloc> && allocator_traits<_Alloc>::is_always_equal::value) {
                // make stateless allocator
                _Alloc _Al{};
                const size_t _Count = (_Size + sizeof(_Aligned_block) - 1) / sizeof(_Aligned_block);
                _Al.deallocate(static_cast<_Aligned_block*>(_Ptr), _Count);
            } else {
                // retrieve stateful allocator
                const auto _Al_address =
                    (reinterpret_cast<uintptr_t>(_Ptr) + _Size + alignof(_Alloc) - 1)
                    & ~(alignof(_Alloc) - 1);
                auto& _Stored_al = *reinterpret_cast<_Alloc*>(_Al_address);
                _Alloc _Al{::std::move(_Stored_al)};
                _Stored_al.~_Alloc();

                static constexpr size_t _Align =
                    (::std::max)(alignof(_Alloc), sizeof(_Aligned_block));
                const size_t _Count =
                    (_Size + sizeof(_Alloc) + _Align - 1) / sizeof(_Aligned_block);
                _Al.deallocate(static_cast<_Aligned_block*>(_Ptr), _Count);
            }
        }
    };

    template <>
    class _Promise_allocator<void> { // type-erased allocator
    private:
        using _Dealloc_fn = void (*)(void*, size_t);

        template <class _ProtoAlloc>
        static void* _Allocate(const _ProtoAlloc& _Proto, size_t _Size) {
            using _Alloc = _Rebind<_ProtoAlloc>;
            auto _Al     = static_cast<_Alloc>(_Proto);

            if constexpr (default_initializable<
                              _Alloc> && allocator_traits<_Alloc>::is_always_equal::value) {
                // don't store stateless allocator
                const _Dealloc_fn _Dealloc = [](void* const _Ptr, const size_t _Size) {
                    _Alloc _Al{};
                    const size_t _Count = (_Size + sizeof(_Dealloc_fn) + sizeof(_Aligned_block) - 1)
                                        / sizeof(_Aligned_block);
                    _Al.deallocate(static_cast<_Aligned_block*>(_Ptr), _Count);
                };

                const size_t _Count = (_Size + sizeof(_Dealloc_fn) + sizeof(_Aligned_block) - 1)
                                    / sizeof(_Aligned_block);
                void* const _Ptr = _Al.allocate(_Count);
                ::memcpy(static_cast<char*>(_Ptr) + _Size, &_Dealloc, sizeof(_Dealloc));
                return _Ptr;
            } else {
                // store stateful allocator
                static constexpr size_t _Align =
                    (::std::max)(alignof(_Alloc), sizeof(_Aligned_block));

                const _Dealloc_fn _Dealloc = [](void* const _Ptr, size_t _Size) {
                    _Size += sizeof(_Dealloc_fn);
                    const auto _Al_address =
                        (reinterpret_cast<uintptr_t>(_Ptr) + _Size + alignof(_Alloc) - 1)
                        & ~(alignof(_Alloc) - 1);
                    auto& _Stored_al = *reinterpret_cast<const _Alloc*>(_Al_address);
                    _Alloc _Al{::std::move(_Stored_al)};
                    _Stored_al.~_Alloc();

                    const size_t _Count =
                        (_Size + sizeof(_Al) + _Align - 1) / sizeof(_Aligned_block);
                    _Al.deallocate(static_cast<_Aligned_block*>(_Ptr), _Count);
                };

                const size_t _Count = (_Size + sizeof(_Dealloc_fn) + sizeof(_Al) + _Align - 1)
                                    / sizeof(_Aligned_block);
                void* const _Ptr = _Al.allocate(_Count);
                ::memcpy(static_cast<char*>(_Ptr) + _Size, &_Dealloc, sizeof(_Dealloc));
                _Size += sizeof(_Dealloc_fn);
                const auto _Al_address =
                    (reinterpret_cast<uintptr_t>(_Ptr) + _Size + alignof(_Alloc) - 1)
                    & ~(alignof(_Alloc) - 1);
                ::new (reinterpret_cast<void*>(_Al_address)) _Alloc{::std::move(_Al)};
                return _Ptr;
            }
        }

    public:
        static void* operator new(const size_t _Size) { // default: new/delete
            void* const _Ptr           = ::operator new[](_Size + sizeof(_Dealloc_fn));
            const _Dealloc_fn _Dealloc = [](void* const _Ptr, const size_t _Size) {
                ::operator delete[](_Ptr, _Size + sizeof(_Dealloc_fn));
            };
            ::memcpy(static_cast<char*>(_Ptr) + _Size, &_Dealloc, sizeof(_Dealloc_fn));
            return _Ptr;
        }

        template <class _Alloc, class... _Args>
        static void* operator new(
            const size_t _Size, allocator_arg_t, const _Alloc& _Al, const _Args&...) {
            static_assert(
                _Has_real_pointers<_Alloc>, "coroutine allocators must use true pointers");
            return _Allocate(_Al, _Size);
        }

        template <class _This, class _Alloc, class... _Args>
        static void* operator new(
            const size_t _Size, const _This&, allocator_arg_t, const _Alloc& _Al, const _Args&...) {
            static_assert(
                _Has_real_pointers<_Alloc>, "coroutine allocators must use true pointers");
            return _Allocate(_Al, _Size);
        }

        static void operator delete(void* const _Ptr, const size_t _Size) noexcept {
            _Dealloc_fn _Dealloc;
            ::memcpy(&_Dealloc, static_cast<const char*>(_Ptr) + _Size, sizeof(_Dealloc_fn));
            _Dealloc(_Ptr, _Size);
        }
    };

    namespace ranges {
        template <range _Rng, class _Alloc = allocator<byte>>
        struct elements_of {
            NO_UNIQUE_ADDRESS _Rng range;
            NO_UNIQUE_ADDRESS _Alloc allocator{};
        };

        template <class _Rng, class _Alloc = allocator<byte>>
        elements_of(_Rng&&, _Alloc = {}) -> elements_of<_Rng&&, _Alloc>;
    } // namespace ranges

    template <class _Rty, class _Vty = void, class _Alloc = void>
    class generator;

    template <class _Rty, class _Vty>
    using _Gen_value_t = conditional_t<is_void_v<_Vty>, remove_cvref_t<_Rty>, _Vty>;
    template <class _Rty, class _Vty>
    using _Gen_reference_t = conditional_t<is_void_v<_Vty>, _Rty&&, _Rty>;
    template <class _Ref>
    using _Gen_yield_t = conditional_t<is_reference_v<_Ref>, _Ref, const _Ref&>;

    template <class _Yielded>
    class _Gen_promise_base {
    public:
        static_assert(is_reference_v<_Yielded>);

        /* [[nodiscard]] */ suspend_always initial_suspend() noexcept {
            return {};
        }

        [[nodiscard]] auto final_suspend() noexcept {
            return _Final_awaiter{};
        }

        [[nodiscard]] suspend_always yield_value(_Yielded _Val) noexcept {
            _Ptr = ::std::addressof(_Val);
            return {};
        }

        // clang-format off
        [[nodiscard]] auto yield_value(const remove_reference_t<_Yielded>& _Val)
            noexcept(is_nothrow_constructible_v<remove_cvref_t<_Yielded>, const remove_reference_t<_Yielded>&>)
            requires (is_rvalue_reference_v<_Yielded> &&
                constructible_from<remove_cvref_t<_Yielded>, const remove_reference_t<_Yielded>&>) {
            // clang-format on
            return _Element_awaiter{_Val};
        }

        // clang-format off
        template <class _Rty, class _Vty, class _Alloc, class _Unused>
            requires same_as<_Gen_yield_t<_Gen_reference_t<_Rty, _Vty>>, _Yielded>
        [[nodiscard]] auto yield_value(
            ::std::ranges::elements_of<generator<_Rty, _Vty, _Alloc>&&, _Unused> _Elem) noexcept {
            // clang-format on
            return _Nested_awaitable<_Rty, _Vty, _Alloc>{std::move(_Elem.range)};
        }

        // clang-format off
        template <::std::ranges::input_range _Rng, class _Alloc>
            requires convertible_to<::std::ranges::range_reference_t<_Rng>, _Yielded>
        [[nodiscard]] auto yield_value(::std::ranges::elements_of<_Rng, _Alloc> _Elem) noexcept {
            // clang-format on
            using _Vty = ::std::ranges::range_value_t<_Rng>;
            return _Nested_awaitable<_Yielded, _Vty, _Alloc>{
                [](allocator_arg_t, _Alloc, ::std::ranges::iterator_t<_Rng> _It,
                    const ::std::ranges::sentinel_t<_Rng> _Se)
                    -> generator<_Yielded, _Vty, _Alloc> {
                    for (; _It != _Se; ++_It) {
                        co_yield static_cast<_Yielded>(*_It);
                    }
                }(allocator_arg, _Elem.allocator, ::std::ranges::begin(_Elem.range),
                        ::std::ranges::end(_Elem.range))};
        }

        void await_transform() = delete;

        void return_void() noexcept {}

        void unhandled_exception() {
            if (_Info) {
                _Info->_Except = ::std::current_exception();
            } else {
                throw;
            }
        }

    private:
        struct _Element_awaiter {
            remove_cvref_t<_Yielded> _Val;

            [[nodiscard]] constexpr bool await_ready() const noexcept {
                return false;
            }

            template <class _Promise>
            constexpr void await_suspend(coroutine_handle<_Promise> _Handle) noexcept {
#ifdef __cpp_lib_is_pointer_interconvertible
                static_assert(is_pointer_interconvertible_base_of_v<_Gen_promise_base, _Promise>);
#endif // __cpp_lib_is_pointer_interconvertible

                _Gen_promise_base& _Current = _Handle.promise();
                _Current._Ptr               = ::std::addressof(_Val);
            }

            constexpr void await_resume() const noexcept {}
        };

        struct _Nest_info {
            exception_ptr _Except;
            coroutine_handle<_Gen_promise_base> _Parent;
            coroutine_handle<_Gen_promise_base> _Root;
        };

        struct _Final_awaiter {
            [[nodiscard]] bool await_ready() noexcept {
                return false;
            }

            template <class _Promise>
            [[nodiscard]] coroutine_handle<> await_suspend(
                coroutine_handle<_Promise> _Handle) noexcept {
#ifdef __cpp_lib_is_pointer_interconvertible
                static_assert(is_pointer_interconvertible_base_of_v<_Gen_promise_base, _Promise>);
#endif // __cpp_lib_is_pointer_interconvertible

                _Gen_promise_base& _Current = _Handle.promise();
                if (!_Current._Info) {
                    return ::std::noop_coroutine();
                }

                coroutine_handle<_Gen_promise_base> _Cont = _Current._Info->_Parent;
                _Current._Info->_Root.promise()._Top      = _Cont;
                _Current._Info                            = nullptr;
                return _Cont;
            }

            void await_resume() noexcept {}
        };

        template <class _Rty, class _Vty, class _Alloc>
        struct _Nested_awaitable {
            static_assert(same_as<_Gen_yield_t<_Gen_reference_t<_Rty, _Vty>>, _Yielded>);

            _Nest_info _Nested;
            generator<_Rty, _Vty, _Alloc> _Gen;

            explicit _Nested_awaitable(generator<_Rty, _Vty, _Alloc>&& _Gen_) noexcept
                : _Gen(::std::move(_Gen_)) {}

            [[nodiscard]] bool await_ready() noexcept {
                return !_Gen._Coro;
            }

            template <class _Promise>
            [[nodiscard]] coroutine_handle<_Gen_promise_base> await_suspend(
                coroutine_handle<_Promise> _Current) noexcept {
#ifdef __cpp_lib_is_pointer_interconvertible
                static_assert(is_pointer_interconvertible_base_of_v<_Gen_promise_base, _Promise>);
#endif // __cpp_lib_is_pointer_interconvertible
                auto _Target =
                    coroutine_handle<_Gen_promise_base>::from_address(_Gen._Coro.address());
                _Nested._Parent =
                    coroutine_handle<_Gen_promise_base>::from_address(_Current.address());
                _Gen_promise_base& _Parent_promise = _Nested._Parent.promise();
                if (_Parent_promise._Info) {
                    _Nested._Root = _Parent_promise._Info->_Root;
                } else {
                    _Nested._Root = _Nested._Parent;
                }
                _Nested._Root.promise()._Top = _Target;
                _Target.promise()._Info      = ::std::addressof(_Nested);
                return _Target;
            }

            void await_resume() {
                if (_Nested._Except) {
                    ::std::rethrow_exception(::std::move(_Nested._Except));
                }
            }
        };

        template <class, class>
        friend class _Gen_iter;

        // _Top and _Info are mutually exclusive, and could potentially be merged.
        coroutine_handle<_Gen_promise_base> _Top =
            coroutine_handle<_Gen_promise_base>::from_promise(*this);
        add_pointer_t<_Yielded> _Ptr = nullptr;
        _Nest_info* _Info            = nullptr;
    };

    struct _Gen_secret_tag {};

    template <class _Value, class _Ref>
    class _Gen_iter {
    public:
        using value_type      = _Value;
        using difference_type = ptrdiff_t;

        _Gen_iter(_Gen_iter&& _That) noexcept : _Coro{::std::exchange(_That._Coro, {})} {}

        _Gen_iter& operator=(_Gen_iter&& _That) noexcept {
            _Coro = ::std::exchange(_That._Coro, {});
            return *this;
        }

        [[nodiscard]] _Ref operator*() const noexcept {
            assert(!_Coro.done() && "Can't dereference generator end iterator");
            return static_cast<_Ref>(*_Coro.promise()._Top.promise()._Ptr);
        }

        _Gen_iter& operator++() {
            assert(!_Coro.done() && "Can't increment generator end iterator");
            _Coro.promise()._Top.resume();
            return *this;
        }

        void operator++(int) {
            ++*this;
        }

        [[nodiscard]] bool operator==(default_sentinel_t) const noexcept {
            return _Coro.done();
        }

    private:
        template <class, class, class>
        friend class generator;

        explicit _Gen_iter(_Gen_secret_tag,
            coroutine_handle<_Gen_promise_base<_Gen_yield_t<_Ref>>> _Coro_) noexcept
            : _Coro{_Coro_} {}

        coroutine_handle<_Gen_promise_base<_Gen_yield_t<_Ref>>> _Coro;
    };

    template <class _Rty, class _Vty, class _Alloc>
    class generator : public ranges::view_interface<generator<_Rty, _Vty, _Alloc>> {
    private:
        using _Value = _Gen_value_t<_Rty, _Vty>;
        static_assert(same_as<remove_cvref_t<_Value>, _Value> && is_object_v<_Value>,
            "generator's value type must be a cv-unqualified object type");

        // clang-format off
    using _Ref = _Gen_reference_t<_Rty, _Vty>;
    static_assert(is_reference_v<_Ref>
        || (is_object_v<_Ref> && same_as<remove_cv_t<_Ref>, _Ref> && copy_constructible<_Ref>),
        "generator's second argument must be a reference type or a cv-unqualified "
        "copy-constructible object type");

    using _RRef = conditional_t<is_lvalue_reference_v<_Ref>, remove_reference_t<_Ref>&&, _Ref>;

    static_assert(common_reference_with<_Ref&&, _Value&> && common_reference_with<_Ref&&, _RRef&&>
        && common_reference_with<_RRef&&, const _Value&>,
        "an iterator with the selected value and reference types cannot model indirectly_readable");
        // clang-format on

        static_assert(_Has_real_pointers<_Alloc>, "generator allocators must use true pointers");

        friend _Gen_promise_base<_Gen_yield_t<_Ref>>;

    public:
        struct EMPTY_BASES promise_type : _Promise_allocator<_Alloc>,
                                          _Gen_promise_base<_Gen_yield_t<_Ref>> {
            [[nodiscard]] generator get_return_object() noexcept {
                return generator{
                    _Gen_secret_tag{}, coroutine_handle<promise_type>::from_promise(*this)};
            }
        };
        static_assert(is_standard_layout_v<promise_type>);
#ifdef __cpp_lib_is_pointer_interconvertible
        static_assert(is_pointer_interconvertible_base_of_v<_Gen_promise_base<_Gen_yield_t<_Ref>>,
            promise_type>);
#endif // __cpp_lib_is_pointer_interconvertible

        generator(generator&& _That) noexcept : _Coro(::std::exchange(_That._Coro, {})) {}

        ~generator() {
            if (_Coro) {
                _Coro.destroy();
            }
        }

        generator& operator=(generator _That) noexcept {
            ::std::swap(_Coro, _That._Coro);
            return *this;
        }

        [[nodiscard]] _Gen_iter<_Value, _Ref> begin() {
            // Pre: _Coro is suspended at its initial suspend point
            assert(_Coro && "Can't call begin on moved-from generator");
            _Coro.resume();
            return _Gen_iter<_Value, _Ref>{_Gen_secret_tag{},
                coroutine_handle<_Gen_promise_base<_Gen_yield_t<_Ref>>>::from_address(
                    _Coro.address())};
        }

        [[nodiscard]] default_sentinel_t end() const noexcept {
            return default_sentinel;
        }

    private:
        coroutine_handle<promise_type> _Coro = nullptr;

        explicit generator(_Gen_secret_tag, coroutine_handle<promise_type> _Coro_) noexcept
            : _Coro(_Coro_) {}
    };
} // namespace std

/////////////////////////////////////////////////////////////////////////////
// Examples start here

#include <array>
#include <cinttypes>
#include <cstdio>
#include <string>
#include <tuple>
#include <vector>

namespace {
    ///////////////////////
    // Simple non-nested serial generator

    std::generator<uint64_t> fib(int max) {
        auto a = 0, b = 1;
        for (auto n = 0; n < max; n++) {
            co_yield std::exchange(a, std::exchange(b, a + b));
        }
    } // namespace std::generator<uint64_t>fib(intmax)

    std::generator<int> other_generator(int i, int j) {
        while (i != j) {
            co_yield i++;
        }
    }

    /////////////////////
    // Demonstrate ability to yield nested sequences
    //
    // Need to use std::ranges::elements_of() to trigger yielding elements of
    // nested sequence.
    //
    // Supports yielding same generator type (with efficient resumption for
    // recursive cases)
    //
    // Also supports yielding any other range whose elements are convertible to
    // the current generator's elements.

    std::generator<uint64_t, uint64_t> nested_sequences_example() {
        std::printf("yielding elements_of std::array\n");
#if defined(__GNUC__) && !defined(__clang__)
        co_yield std::ranges::elements_of(std::array<const uint64_t, 5>{2, 4, 6, 8, 10}, {});
#else
        co_yield std::ranges::elements_of{std::array<const uint64_t, 5>{2, 4, 6, 8, 10}};
#endif
        std::printf("yielding elements_of nested std::generator\n");
        co_yield std::ranges::elements_of{fib(10)};

        std::printf("yielding elements_of other kind of generator\n");
        co_yield std::ranges::elements_of{other_generator(5, 8)};
    }

    //////////////////////////////////////
    // Following examples show difference between:
    //
    //                                    If I co_yield a...
    //                              X / X&&  | X&         | const X&
    //                           ------------+------------+-----------
    // - generator<X, X>               (same as generator<X, X&&>)
    // - generator<X, const X&>   ref        | ref        | ref
    // - generator<X, X&&>        ref        | ill-formed | ill-formed
    // - generator<X, X&>         ill-formed | ref        | ill-formed

    struct X {
        int id;
        X(int id) : id(id) {
            std::printf("X::X(%i)\n", id);
        }
        X(const X& x) : id(x.id) {
            std::printf("X::X(copy %i)\n", id);
        }
        X(X&& x) : id(std::exchange(x.id, -1)) {
            std::printf("X::X(move %i)\n", id);
        }
        ~X() {
            std::printf("X::~X(%i)\n", id);
        }
    };

    std::generator<X> always_ref_example() {
        co_yield X{1};
        {
            X x{2};
            co_yield x;
            assert(x.id == 2);
        }
        {
            const X x{3};
            co_yield x;
            assert(x.id == 3);
        }
        {
            X x{4};
            co_yield std::move(x);
        }
    }

    std::generator<X&&> xvalue_example() {
        co_yield X{1};
        X x{2};
        co_yield x; // well-formed: generated element is copy of lvalue
        assert(x.id == 2);
        co_yield std::move(x);
    }

    std::generator<const X&> const_lvalue_example() {
        co_yield X{1}; // OK
        const X x{2};
        co_yield x; // OK
        co_yield std::move(x); // OK: same as above
    }

    std::generator<X&> lvalue_example() {
        // co_yield X{1}; // ill-formed: prvalue -> non-const lvalue
        X x{2};
        co_yield x; // OK
        // co_yield std::move(x); // ill-formed: xvalue -> non-const lvalue
    }

    ///////////////////////////////////
    // These examples show different usages of reference/value_type
    // template parameters

    // value_type = std::unique_ptr<int>
    // reference = std::unique_ptr<int>&&
    std::generator<std::unique_ptr<int>&&> unique_ints(const int high) {
        for (auto i = 0; i < high; ++i) {
            co_yield std::make_unique<int>(i);
        }
    }

    // value_type = std::string_view
    // reference = std::string_view&&
    std::generator<std::string_view> string_views() {
        co_yield "foo";
        co_yield "bar";
    }

    // value_type = std::string
    // reference = std::string_view
    template <typename Allocator>
    std::generator<std::string_view, std::string> strings(std::allocator_arg_t, Allocator) {
        co_yield {};
        co_yield "start";
        for (auto sv : string_views()) {
            co_yield std::string{sv} + '!';
        }
        co_yield "end";
    }

    // Resulting vector is deduced using ::value_type.
    template <std::ranges::input_range R>
    std::vector<std::ranges::range_value_t<R>> to_vector(R&& r) {
        std::vector<std::ranges::range_value_t<R>> v;
        for (auto&& x : r) {
            v.emplace_back(static_cast<decltype(x)&&>(x));
        }
        return v;
    }

    // zip() algorithm produces a generator of tuples where
    // the reference type is a tuple of references and
    // the value type is a tuple of values.
    template <std::ranges::range... Rs, std::size_t... Indices>
    std::generator<std::tuple<std::ranges::range_reference_t<Rs>...>,
        std::tuple<std::ranges::range_value_t<Rs>...>>
        zip_impl(std::index_sequence<Indices...>, Rs... rs) {
        std::tuple<std::ranges::iterator_t<Rs>...> its{std::ranges::begin(rs)...};
        std::tuple<std::ranges::sentinel_t<Rs>...> itEnds{std::ranges::end(rs)...};
        while (((std::get<Indices>(its) != std::get<Indices>(itEnds)) && ...)) {
            co_yield {*std::get<Indices>(its)...};
            (++std::get<Indices>(its), ...);
        }
    }

    template <std::ranges::range... Rs>
    std::generator<std::tuple<std::ranges::range_reference_t<Rs>...>,
        std::tuple<std::ranges::range_value_t<Rs>...>>
        zip(Rs&&... rs) {
        return zip_impl(std::index_sequence_for<Rs...>{}, std::views::all(std::forward<Rs>(rs))...);
    }

    void value_type_example() {
        std::vector<std::string_view> s1 = to_vector(string_views());
        for (auto& s : s1)
            std::printf("\"%*s\"\n", (int) s.size(), s.data());

        std::printf("\n");

        std::vector<std::string> s2 =
            to_vector(strings(std::allocator_arg, std::allocator<std::byte>{}));
        for (auto& s : s2) {
            std::printf("\"%s\"\n", s.c_str());
        }

        std::printf("\n");

        std::vector<std::tuple<std::string, std::string>> s3 =
            to_vector(zip(strings(std::allocator_arg, std::allocator<std::byte>{}),
                strings(std::allocator_arg, std::allocator<std::byte>{})));
        for (auto& [a, b] : s3) {
            std::printf("\"%s\", \"%s\"\n", a.c_str(), b.c_str());
        }
    }

    template <typename T>
    struct stateful_allocator {
        using value_type = T;

        int id;

        explicit stateful_allocator(int id) noexcept : id(id) {}

        template <typename U>
        stateful_allocator(const stateful_allocator<U>& x) : id(x.id) {}

        T* allocate(std::size_t count) {
            std::printf("stateful_allocator(%i).allocate(%zu)\n", id, count);
            return std::allocator<T>().allocate(count);
        }

        void deallocate(T* ptr, std::size_t count) noexcept {
            std::printf("stateful_allocator(%i).deallocate(%zu)\n", id, count);
            std::allocator<T>().deallocate(ptr, count);
        }

        template <typename U>
        bool operator==(const stateful_allocator<U>& x) const {
            return this->id == x.id;
        }
    };

    std::generator<int, void, std::allocator<std::byte>> stateless_example() {
        co_yield 42;
    }

    std::generator<int, void, std::allocator<std::byte>> stateless_example(
        std::allocator_arg_t, std::allocator<std::byte>) {
        co_yield 42;
    }

    template <typename Allocator>
    std::generator<int, void, Allocator> stateful_alloc_example(std::allocator_arg_t, Allocator) {
        co_yield 42;
    }

    struct member_coro {
        std::generator<int> f() const {
            co_yield 42;
        }
    };
} // namespace

