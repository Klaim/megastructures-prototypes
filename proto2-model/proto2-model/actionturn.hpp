#pragma once

#include <optional>
#include <vector>
#include <type_traits>
#include <memory>
#include <typeindex>
#include <string>
#include <format>

#include <tl/generator.hpp>

#include <proto2-model/export.hpp>
#include <proto2-model/core.hpp>


namespace proto2::model
{

    template<class T>
    concept Event = std::semiregular<std::remove_cvref_t<T>>
        and requires(const std::remove_cvref_t<T>& event)
    {
        { event.text_description() } -> std::convertible_to<std::string>;
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

        std::type_index type_id() const
        {
            return storage->type_id();
        }

        std::string text_description() const
        {
            return storage->text_description();
        }

    private:
        struct Interface
        {
            virtual std::unique_ptr<Interface> clone() const = 0;
            virtual std::type_index type_id() const = 0;
            virtual std::string text_description() const = 0;

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

            std::type_index type_id() const override
            {
                return typeid(impl);
            }

            std::string text_description() const override
            {
                return impl.text_description();
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

        std::type_index type_id() const
        {
            return storage->type_id();
        }

    private:
        struct Interface
        {
            virtual ActionResults execute(ActionContext action_context) const = 0;
            virtual std::unique_ptr<Interface> clone() const = 0;
            virtual std::type_index type_id() const = 0;

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

            std::type_index type_id() const override
            {
                return typeid(impl);
            }
        };

        std::unique_ptr<Interface> storage;

    };


    namespace events
    {
        struct Waited
        {
            BodyID body_id;

            std::string text_description() const { return std::format("Body{{{}}} waited.", body_id); }
        };

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
