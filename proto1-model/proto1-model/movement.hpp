#pragma once

#include <proto1-model/actionturn.hpp>

namespace proto1::model::actions
{
    struct Move
    {
        Vector2 direction;

        ActionResults execute(ActionContext action_context) const
        {
            action_context.body_acting.position = action_context.body_acting.position + direction;
            return { {Event{}} };
        }
    };

}