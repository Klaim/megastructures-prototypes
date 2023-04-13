#pragma once

#include <optional>
#include <vector>
#include <type_traits>
#include <memory>
#include <typeindex>

#include <tl/generator.hpp>
#include <fmt/core.h>

#include <proto1-model/export.hpp>
#include <proto1-model/core.hpp>


namespace proto1::model
{

    struct Event{}; // TODO: make this type-erasing wrapper?

    struct ActionResults
    {
        bool action_failed = false;
        std::vector<Event> events;
    };

    struct ActionContext
    {
        World& world;
        Body& body_acting;
        Actor& actor_deciding;
    };

    template<class T>
    concept Action = requires(T&& thing, ActionContext context) // TODO: add value semantics requirements
    {
        { thing.execute(context) } -> std::convertible_to<ActionResults>;
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

        template<Action T>
        AnyAction(T&& impl)
            requires(not std::is_same_v<AnyAction, std::remove_cvref_t<T>>)
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

        // TODO:
        // consider this https://godbolt.org/z/5ajqT584v
        // OR BETTER: https://www.fluentcpp.com/2021/01/29/inheritance-without-pointers/

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


    namespace actions
    {
        struct Wait
        {
            ActionResults execute(ActionContext) const
            {
                return {};
            }
        };
    }

    inline
    auto execute(const AnyAction& action, ActionContext action_context)
        -> ActionResults
    {
        fmt::print("\n{} -> {}", action_context.actor_deciding.is_player() ? "Player" : "NPC", action.type_id().name());
        return action.execute(action_context);
    }

    inline
    auto decide_next_action(ActionContext action_context)
        -> AnyAction
    {
        if (action_context.actor_deciding.decide_next_action)
            return action_context.actor_deciding.decide_next_action(action_context);

        return actions::Wait{};
    }

    struct TurnInfo
    {
        int current_turn = 0;
        std::vector<Event> events;
    };

    class PROTO1_MODEL_SYMEXPORT TurnSolver
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
