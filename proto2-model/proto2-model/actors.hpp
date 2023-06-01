#pragma once

#include <tl/generator.hpp>

#include <proto2-model/export.hpp>
#include <proto2-model/actionturn.hpp>


namespace proto2::model::actors
{
    PROTO2_MODEL_SYMEXPORT
    std::vector<AnyAction> default_random_actions();

    struct PROTO2_MODEL_SYMEXPORT DoRandomAction
    {
        std::vector<AnyAction> possible_actions = default_random_actions();
        bool allow_waiting = false;

        AnyAction operator()(ActionContext context) const;

    };


    class PROTO2_MODEL_SYMEXPORT WalkUntilYouReachAWall
    {
    public:

        WalkUntilYouReachAWall() = default;
        WalkUntilYouReachAWall(WalkUntilYouReachAWall&&) noexcept = default;
        WalkUntilYouReachAWall& operator=(WalkUntilYouReachAWall&&) noexcept = default;

        WalkUntilYouReachAWall(const WalkUntilYouReachAWall& other)
        {
            last_action = other.last_action;
        }

        WalkUntilYouReachAWall& operator=(const WalkUntilYouReachAWall& other)
        {
            *this = WalkUntilYouReachAWall(other);
            return *this;
        }

        AnyAction operator()(ActionContext action_context);

    private:
        using Generator = tl::generator<AnyAction>;

        struct State
        {
            Generator generator;
            decltype(generator.begin()) iterator;
        };

        std::optional<State> state;

        DoRandomAction do_random_action;
        std::optional<AnyAction> last_action;
        ActionContext* current_action_context = nullptr;

        Generator start_decision_logic();

    };

}


