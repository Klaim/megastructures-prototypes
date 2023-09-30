#pragma once

#include <optional>
#include <vector>
#include <type_traits>
#include <memory>
#include <string>
#include <any>

#include <boost/type_index.hpp>
#include <boost/algorithm/string/erase.hpp>

#include <fmt/format.h>
#include <tl/generator.hpp>
#include <nlohmann/json.hpp>

#include <proto2-model/export.hpp>
#include <proto2-model/core.hpp>


namespace proto2::model
{
    using json = nlohmann::json;

    template<class TypeInfo>
    std::string universal_type_name(const TypeInfo& info)
    {
        auto name = info.pretty_name();
        static constexpr std::string_view struct_prefix = "struct ";
        static constexpr std::string_view class_prefix = "class ";
        boost::erase_all(name, struct_prefix);
        boost::erase_all(name, class_prefix);
        return name;
    }

    template<class T>
    std::string type_name()
    {
        return universal_type_name(boost::typeindex::type_id<T>());
    }

    template<class T>
    constexpr
    std::string type_name(const T&)
    {
        return type_name<T>();
    }


    template<class T>
    concept Reflectable = requires
    {
        { boost::mp11::mp_for_each<boost::describe::describe_members<T, boost::describe::mod_public>>([](auto&&){}) };
    };

    template<class T>
    concept ReflectionSource = requires(const std::remove_cvref_t<T>& x)
    {
        { x.reflection() } -> std::same_as<json>;
    };

    template<class T>
    concept ReflectionReady = Reflectable<T> or ReflectionSource<T>;

    json reflection(ReflectionSource auto const & message)
    {
        return message.reflection();
    }

    template<ReflectionReady T>
    void insert_to_json(json& result, std::string_view name, const T& value)
    {
        result[name] = reflection(value);
    }

    template<typename T>
        requires (not ReflectionReady<T>)
    void insert_to_json(json& result, std::string_view name, const T& value)
    {
        result[name] = value;
    }

    template<Reflectable T>
    json reflection(const T& message)
    {
        using json = json;
        json result;
        const auto name = type_name(message);
        result["type_name"] = name;
        boost::mp11::mp_for_each<boost::describe::describe_members<T, boost::describe::mod_public>>(
            [&](auto member_info) {
                insert_to_json(result, member_info.name, message.*member_info.pointer);
            });
        return result;
    }


    template<class T>
    concept Event = std::semiregular<std::remove_cvref_t<T>>
        and requires(const std::remove_cvref_t<T>& event)
    {
        { event.text_description() } -> std::convertible_to<std::string>;
        { ::proto2::model::reflection(event) } -> std::same_as<json>;
    };

    struct AnyEvent
    {
        AnyEvent() = default;

        AnyEvent(const AnyEvent& other)
            : storage(other.storage->clone())
        {
        }

        AnyEvent& operator=(const AnyEvent& other)
        {
            storage = other.storage->clone();
            return *this;
        }

        AnyEvent(AnyEvent&&) noexcept = default;
        AnyEvent& operator=(AnyEvent&&) noexcept = default;

        template<class T>
        AnyEvent(T&& impl)
            requires(not std::is_same_v<AnyEvent, std::remove_cvref_t<T>>
            and Event<T>)
            : storage(std::make_unique<Impl<T>>(std::forward<T>(impl)))
        {

        }

        boost::typeindex::type_index type_id() const
        {
            return storage->type_id();
        }

        std::string type_name() const
        {
            return universal_type_name(type_id());
        }

        std::string text_description() const
        {
            return storage->text_description();
        }

        json reflection() const
        {
            return storage->reflection();
        }

        template<Event T>
        T& value()
        {
            const auto requested_type_id = boost::typeindex::type_id<T>();
            const auto stored_type_id = type_id();
            if( requested_type_id != stored_type_id )
                throw std::invalid_argument(fmt::format("wrong type stored, requested '{}' but stored is '{}'", requested_type_id.pretty_name(), stored_type_id.pretty_name()));
            return static_cast<Impl<T>*>(storage.get())->impl;
        }

        template<Event T>
        const T& value() const
        {
            return const_cast<AnyEvent&>(*this).value();
        }

    private:
        struct Interface
        {
            virtual std::unique_ptr<Interface> clone() const = 0;
            virtual boost::typeindex::type_index type_id() const = 0;
            virtual std::string text_description() const = 0;
            virtual json reflection() const = 0;

            virtual ~Interface() = default;
        };

        template<Event T>
        struct Impl : Interface
        {
            T impl;

            explicit Impl(T value) : impl(std::move(value)) {}

            std::unique_ptr<Interface> clone() const override
            {
                return std::make_unique<Impl<T>>(impl);
            }

            boost::typeindex::type_index type_id() const override
            {
                return boost::typeindex::type_id<T>();
            }

            std::string text_description() const override
            {
                return impl.text_description();
            }

            json reflection() const override
            {
                return ::proto2::model::reflection(impl);
            }
        };

        std::unique_ptr<Interface> storage;

    };

    struct ActionResults
    {
        bool action_is_success = true;
        std::vector<AnyEvent> events;
    };

    struct ActionContext
    {
        World& world;
        Body& body_acting;
        Actor& actor_deciding;
    };

    template<class T>
    concept Action = std::semiregular<std::remove_cvref_t<T>>
        and requires(const std::remove_cvref_t<T>& action, ActionContext context)
    {
        { action.execute(context) } -> std::convertible_to<ActionResults>;
        { ::proto2::model::reflection(action) } -> std::same_as<json>;
    };

    struct AnyAction
    {
        AnyAction() = default;

        AnyAction(const AnyAction& other)
            : storage(other.storage->clone())
        {
        }

        AnyAction& operator=(const AnyAction& other)
        {
            storage = other.storage->clone();
            return *this;
        }

        AnyAction(AnyAction&&) noexcept = default;
        AnyAction& operator=(AnyAction&&) noexcept = default;

        template<class T>
        AnyAction(T&& impl)
            requires(not std::is_same_v<AnyAction, std::remove_cvref_t<T>>
            and Action<T>)
            : storage(std::make_unique<Impl<T>>(std::forward<T>(impl)))
        {

        }


        ActionResults execute(ActionContext action_context) const
        {
            return storage->execute(action_context);
        }

        boost::typeindex::type_index type_id() const
        {
            return storage->type_id();
        }

        std::string type_name() const
        {
            return universal_type_name(type_id());
        }

        json reflection() const
        {
            return storage->reflection();
        }

        template<Action T>
        T& value()
        {
            const auto requested_type_id = boost::typeindex::type_id<T>();
            const auto stored_type_id = type_id();
            if( requested_type_id != stored_type_id )
                throw std::invalid_argument(fmt::format("wrong type stored, requested '{}' but stored is '{}'", requested_type_id.pretty_name(), stored_type_id.pretty_name()));
            return static_cast<Impl<T>*>(storage.get())->impl;
        }

        template<Action T>
        const T& value() const
        {
            return const_cast<AnyAction*>(this)->value();
        }

    private:
        struct Interface
        {
            virtual ActionResults execute(ActionContext action_context) const = 0;
            virtual std::unique_ptr<Interface> clone() const = 0;
            virtual boost::typeindex::type_index type_id() const = 0;
            virtual json reflection() const = 0;

            virtual ~Interface() = default;
        };

        template<Action T>
        struct Impl : Interface
        {
            T impl;

            explicit Impl(T value) : impl(std::move(value)) {}

            ActionResults execute(ActionContext action_context) const override
            {
                return impl.execute(action_context);
            }

            std::unique_ptr<Interface> clone() const override
            {
                return std::make_unique<Impl<T>>(impl);
            }

            boost::typeindex::type_index type_id() const override
            {
                return boost::typeindex::type_id<T>();
            }

            json reflection() const override
            {
                return ::proto2::model::reflection(impl);
            }
        };

        std::unique_ptr<Interface> storage;

    };


    template<class T>
    concept AnyMessageType = std::same_as<T, ::proto2::model::AnyAction> or std::same_as<T, ::proto2::model::AnyEvent>;

    template<class T>
    concept MessageType = not AnyMessageType<T> and (Event<T> or Action<T>);

    namespace events
    {
        struct Waited
        {
            EntityID body_id;

            std::string text_description() const { return fmt::format("Body{{{}}} waited.", body_id); }
        };
        BOOST_DESCRIBE_STRUCT(Waited, (), (body_id));

        static_assert(Event<Waited>);
    }

    namespace actions
    {


        struct Wait
        {
            ActionResults execute(ActionContext context) const
            {
                return { .events = { events::Waited{ .body_id = context.body_acting.id } } };
            }
        };
        BOOST_DESCRIBE_STRUCT(Wait, (), ());

    }

    auto execute(const AnyAction& action, ActionContext action_context) -> ActionResults;

    auto decide_next_action(ActionContext action_context)-> AnyAction;

    struct TurnInfo
    {
        int current_turn = 0;
        std::vector<AnyEvent> events;
    };

    class PROTO2_MODEL_SYMEXPORT TurnSolver
    {
    public:

        explicit TurnSolver(World& world);
        ~TurnSolver();

        TurnSolver(const TurnSolver&) = delete;
        TurnSolver& operator=(const TurnSolver&) = delete;

        auto start_until_player_turn() -> TurnInfo;

        auto play_action_until_next_turn(AnyAction action) -> TurnInfo;

    private:
        using TurnSequence = tl::generator<TurnInfo>;

        World& world;
        std::optional<TurnSequence> turn_sequence;
        decltype(turn_sequence->begin()) turn_iterator;
        std::optional<AnyAction> next_player_action;

        auto start_sequence() -> TurnSequence;
    };


}
