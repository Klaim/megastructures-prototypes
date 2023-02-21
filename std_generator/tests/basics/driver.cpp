// Sources: 
// - https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2022/p2502r2.pdf
// - https://godbolt.org/z/5hcaPcfvP

#include <std_generator/version.hpp>
#include <std_generator/generator.hpp>

int main() {
    std::setvbuf(stdout, nullptr, _IOLBF, BUFSIZ);

    std::printf("nested_sequences_example\n");
    for (uint64_t a : nested_sequences_example()) {
        std::printf("-> %" PRIu64 "\n", a);
    }

    std::printf("\nby_value_example\n");

    for (auto&& x : always_ref_example()) {
        std::printf("-> %i\n", x.id);
    }

    std::printf("\nby_rvalue_ref_example\n");

    for (auto&& x : xvalue_example()) {
        std::printf("-> %i\n", x.id);
    }

    std::printf("\nby_const_ref_example\n");

    for (auto&& x : const_lvalue_example()) {
        std::printf("-> %i\n", x.id);
    }

    std::printf("\nby_lvalue_ref_example\n");

    for (auto&& x : lvalue_example()) {
        std::printf("-> %i\n", x.id);
    }

    std::printf("\nvalue_type example\n");

    value_type_example();

    std::printf("\nmove_only example\n");

    for (std::unique_ptr<int> ptr : unique_ints(5)) {
        std::printf("-> %i\n", *ptr);
    }

    std::printf("\nstateless_alloc examples\n");

    stateless_example();
    stateless_example(std::allocator_arg, std::allocator<float>{});

    std::printf("\nstateful_alloc example\n");

    stateful_alloc_example(std::allocator_arg, stateful_allocator<double>{42});

    member_coro m;
    assert(*m.f().begin() == 42);
}
