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

}