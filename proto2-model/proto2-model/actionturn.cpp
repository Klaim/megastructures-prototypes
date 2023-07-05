#include <proto2-model/actionturn.hpp>

#include <stdx/utility_ranges.hpp>

namespace proto2::model
{
    auto execute(const AnyAction& action, ActionContext action_context) -> ActionResults
    {
        return action.execute(action_context);
    }

    auto decide_next_action(ActionContext action_context)-> AnyAction
    {
        if (action_context.actor_deciding.decide_next_action)
            return action_context.actor_deciding.decide_next_action(action_context);

        return actions::Wait{};
    }

    TurnSolver::TurnSolver(World& world)
        : world(world)
    {}

    TurnSolver::~TurnSolver() = default;

    auto TurnSolver::start_until_player_turn() -> TurnInfo
    {
        turn_sequence = start_sequence();
        turn_iterator = turn_sequence->begin();
        return *turn_iterator;
    }

    auto TurnSolver::play_action_until_next_turn(AnyAction action) -> TurnInfo
    {
        if(not turn_sequence.has_value())
            throw std::runtime_error("TurnSolver::play_action_until_next_turn called before TurnSolver::start_until_player_turn");

        next_player_action = action;
        ++turn_iterator;
        return *turn_iterator;
    }


    auto TurnSolver::start_sequence() -> TurnSequence
    {
        TurnInfo turn_info;
        int current_turn = 0;
        while(world.has_player_bodies())
        {
            ++current_turn;
            turn_info.current_turn = current_turn;

            auto bodies = world.entities.view<Body>();

            for(auto [entity, body] : bodies.each())
            {
                if(not body.can_act())
                    continue;

                auto& actor = world.actors[ body.actor_id.value() ];

                AnyAction next_action{ actions::Wait{} };
                if(actor.is_player())
                {
                    co_yield turn_info;
                    turn_info = { .current_turn = current_turn };
                    assert(next_player_action.has_value());
                    next_action = *next_player_action;
                }
                else
                {
                    next_action = decide_next_action({ world, body, actor });
                }

                ActionResults action_results = execute(next_action, { world, body, actor });
                body.last_action_failed = not action_results.action_is_success;
                turn_info.events = stdx::append_ranges(turn_info.events, action_results.events);

            }
        }
    }
}

