#include <proto1-model/actors.hpp>

#include <stdx/utility_ranges.hpp>

#include <proto1-model/movement.hpp>

#include <random>

namespace proto1::model::actors
{
    namespace
    {
        AnyAction random_action(ActionContext, bool allow_wait)
        {
            static std::random_device r;
            static std::mt19937 rng_gen(r()); // FIXME: better seed
            static const std::vector<AnyAction> possible_actions {
                actions::Move{ Vector2::LEFT },
                actions::Move{ Vector2::RIGHT },
                actions::Move{ Vector2::DOWN },
                actions::Move{ Vector2::UP },
            };

            if(allow_wait)
            {
                std::uniform_int percent_chance { 1, 100 };
                if (percent_chance(rng_gen) >= 50)
                    return actions::Wait{};
            }

            AnyAction chosen_action = *stdx::pick_random_element(possible_actions, rng_gen);
            return chosen_action;
        }

    }

    AnyAction random_action(ActionContext context)
    {
        return random_action(context, false);
    }

    AnyAction random_action_or_wait(ActionContext context)
    {
        return random_action(context, true);
    }

}