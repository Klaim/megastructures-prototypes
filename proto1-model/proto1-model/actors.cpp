#include <proto1-model/actors.hpp>

#include <stdx/utility_ranges.hpp>

#include <proto1-model/movement.hpp>

#include <random>

namespace proto1::model::actors
{
    AnyAction random_action(ActionContext)
    {
        static std::random_device r;
        static const std::seed_seq seed_seq {r(), r(), r(), r(), r(), r(), r(), r()};
        static std::mt19937 rng_gen(seed_seq);
        static const std::vector<AnyAction> possible_actions {
            actions::Move{ Vector2::LEFT },
            actions::Move{ Vector2::RIGHT },
            actions::Move{ Vector2::DOWN },
            actions::Move{ Vector2::UP },
        };

        AnyAction chosen_action = *stdx::pick_random_element(possible_actions, rng_gen);
        return chosen_action;
    }

}