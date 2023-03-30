#pragma once

#include <optional>
#include <vector>

#include <tl/generator.hpp>
#include <boost-te.hpp>

#include <proto1-model/export.hpp>
#include <proto1-model/core.hpp>


namespace proto1::model
{
    
    struct Event{}; // TODO: make this type-erasing wrapper?
    
    struct ActionResults
    {
        std::vector<Event> events;
    };

    struct ActionContext
    {
        World& world;
        Body& body_acting;
        Actor& actor_deciding;
    };

    struct Action
    {
        ActionResults execute(ActionContext action_context) const
        {
            return boost::te::call<ActionResults>(
                [](auto const &self, ActionContext action_context) { 
                    return self.execute(action_context); 
                }
                , *this, action_context);
        }
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


    using AnyAction = boost::te::poly<Action>;

    inline
    auto execute(const AnyAction& action, ActionContext action_context)
        -> ActionResults
    {
        return action.execute(action_context);
    }
    
    inline
    auto decide_next_action(ActionContext action_context)
        -> AnyAction
    { 
        // FIXME:
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
