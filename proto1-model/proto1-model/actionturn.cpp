#include <proto1-model/actionturn.hpp>

#include <stdx/utility_ranges.hpp>

namespace proto1::model
{
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

                auto actor = world.actors[body.actor_id.value()];

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
                turn_info.events = stdx::append_ranges(turn_info.events, action_results.events);

            }
        }
    }  
}

