#pragma once

#include <proto1-model/actionturn.hpp>

namespace proto1::model::actions
{
    struct Move
    {
        Vector2 direction;

        ActionResults execute(ActionContext action_context) const
        {
            const auto new_position = action_context.body_acting.position + direction;
            if(not action_context.world.is_free_position(new_position))
                return { .action_failed = true, .events = {Event{}} };

            action_context.body_acting.position = new_position;
            return { .events = {Event{}} };
        }
    };

    const inline Move MOVE_LEFT{ Vector2::LEFT };
    const inline Move MOVE_RIGHT{ Vector2::RIGHT };
    const inline Move MOVE_UP{ Vector2::UP };
    const inline Move MOVE_DOWN{ Vector2::DOWN };

    const inline std::vector<Move> ADJACENT_MOVES {
        MOVE_LEFT, MOVE_RIGHT, MOVE_UP, MOVE_DOWN
    };


}