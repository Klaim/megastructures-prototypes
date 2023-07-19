#include <proto2-model/actors.hpp>

#include <stdx/utility_ranges.hpp>

#include <proto2-model/movement.hpp>

#include <random>

namespace proto2::model::actors
{
    std::vector<AnyAction> default_random_actions()
    {
        return std::vector<AnyAction>(actions::ADJACENT_MOVES.cbegin(), actions::ADJACENT_MOVES.cend());
    }

    AnyAction DoRandomAction::operator()(ActionContext context) const
    {
        static std::random_device r;
        static std::mt19937 rng_gen(r()); // FIXME: better seed

        if(allow_waiting)
        {
            std::uniform_int_distribution<int> percent_chance { 1, 100 };
            if (percent_chance(rng_gen) >= 50)
                return actions::Wait{};
        }

        AnyAction chosen_action = *stdx::pick_random_element(possible_actions, rng_gen);
        return chosen_action;
    }

    AnyAction WalkUntilYouReachAWall::operator()(ActionContext action_context)
    {
        current_action_context = &action_context;

        if(state)
        {
            ++state->iterator;
        }
        else
        {
            state = State{ start_decision_logic() };
            state->iterator = state->generator.begin();
        }

        current_action_context = nullptr;

        if(state->iterator != state->generator.end())
        {
            return *state->iterator;
        }
        else
        {
            state.reset();
            return actions::Wait{};
        }
    }

    WalkUntilYouReachAWall::WalkUntilYouReachAWall() = default;
    WalkUntilYouReachAWall::WalkUntilYouReachAWall(WalkUntilYouReachAWall&&) noexcept = default;
    WalkUntilYouReachAWall& WalkUntilYouReachAWall::operator=(WalkUntilYouReachAWall&&) noexcept = default;

    WalkUntilYouReachAWall::Generator WalkUntilYouReachAWall::start_decision_logic()
    {
        assert(current_action_context);
        while(true)
        {
            if(current_action_context->body_acting.last_action_failed)
            {
                last_action.reset();
            }

            if(not last_action.has_value())
                last_action = do_random_action(*current_action_context);

            co_yield *last_action;
        }
    }

}