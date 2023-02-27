#pragma once

#include <proto1-model/core.hpp>

namespace proto1::model
{

    struct Event{}; // TODO: make this type-erasing wrapper?

    struct ActionResults
    {
        World world_after_events;
        std::vector<Event> events;
    };

    struct Action // TODO: make this a type-erasing wrapper (ideally using only values)
    {
        auto execute(World& world, Body& body_acting, Actor actor_deciding) -> ActionResults
        {
            // FIXME: 
            return {};
        }
    };

    struct TurnInfo
    {
        int current_turn = 0;
        World world;
        std::vector<Event> events;
    };


    class TurnSolver
    {
    public:

    };


}
