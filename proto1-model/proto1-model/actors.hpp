#pragma once

#include <tl/generator.hpp>

#include <proto1-model/actionturn.hpp>


namespace proto1::model::actors
{
    AnyAction random_action(ActionContext, bool allow_wait = true);

    class WalkUntilYouReachAWall
    {
    public:

        WalkUntilYouReachAWall() = default;
        WalkUntilYouReachAWall(WalkUntilYouReachAWall&&) noexcept = default;
        WalkUntilYouReachAWall& operator=(WalkUntilYouReachAWall&&) noexcept = default;

        AnyAction operator()(ActionContext action_context)
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

    private:
        using Generator = tl::generator<AnyAction>;

        struct State
        {
            Generator generator;
            decltype(generator.begin()) iterator;
        };

        std::optional<State> state;

        std::optional<AnyAction> last_action;
        ActionContext* current_action_context = nullptr;

        Generator start_decision_logic()
        {
            assert(current_action_context);
            while(true)
            {
                if(current_action_context->body_acting.last_action_failed)
                {
                    last_action.reset();
                }

                if(not last_action.has_value())
                    last_action = random_action(*current_action_context, false);

                co_yield *last_action;
            }
        }
        
    };

}


