#pragma once

#include <proto2-model/actionturn.hpp>


namespace proto2::model
{
    namespace events
    {
        struct Moved
        {
            BodyID body_id;
            Vector2 initial_position;
            Vector2 new_position;

            std::string text_description() const
            {
                return std::format("Body{{{}}} moved from {{ {}, {} }} to {{ {}, {} }}",
                    body_id,
                    initial_position.x, initial_position.y,
                    new_position.x, new_position.y
                );
            }
        };
        static_assert(Event<Moved>);

        struct FailedToMove
        {
            BodyID body_id;
            Vector2 target_position;

            std::string text_description() const
            {
                return std::format("Body{{{}}} attempted to move to {{ {}, {} }} but failed",
                    body_id,
                    target_position.x, target_position.y
                );
            }
        };
    }

    namespace actions
    {
        struct Move
        {
            Vector2 direction;

            ActionResults execute(ActionContext action_context) const
            {
                const auto initial_position = action_context.body_acting.position;
                const auto new_position = action_context.body_acting.position + direction;
                if (not action_context.world.is_free_position(new_position))
                    return {
                        .action_is_success = false,
                        .events = { events::FailedToMove{
                            .body_id = action_context.body_acting.id,
                            .target_position = new_position,
                        } }
                    };

                action_context.body_acting.position = new_position;
                return {
                    .events = { events::Moved{
                        .body_id  = action_context.body_acting.id,
                        .initial_position = initial_position,
                        .new_position = new_position,
                    }}
                };
            }
        };
        static_assert(Action<Move>);

        const inline Move MOVE_LEFT{Vector2::LEFT};
        const inline Move MOVE_RIGHT{Vector2::RIGHT};
        const inline Move MOVE_UP{Vector2::UP};
        const inline Move MOVE_DOWN{Vector2::DOWN};

        const inline std::vector<Move> ADJACENT_MOVES{
            MOVE_LEFT, MOVE_RIGHT, MOVE_UP, MOVE_DOWN};

    }
}