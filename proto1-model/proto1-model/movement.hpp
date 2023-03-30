#pragma once

#include <proto1-model/actionturn.hpp>

namespace proto1::model::actions
{
    struct Move
    {
        Vector2 target;

        ActionResults execute(ActionContext action_context) const
        {
            action_context.body_acting.position = target;
            return {};
        }
    };

}