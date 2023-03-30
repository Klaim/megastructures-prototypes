#pragma once

#include <optional>
#include <vector>

#include <tl/generator.hpp>

#include <proto1-model/export.hpp>
#include <proto1-model/core.hpp>


namespace proto1::model
{

    struct ActionResults
    {
        std::vector<Event> events;
    };


    namespace actions
    {
        using Wait = Action;
    }


    inline
    auto execute(Action& action, World& world, Body& body_acting, Actor actor_deciding)
        -> ActionResults
    {
        // FIXME: 
        return {};
    }
    
    inline
    auto decide_next_action(Actor& actor, Body& body)
        -> Action
    { 
        // FIXME:
        return actions::Wait{};
    }

    struct TurnInfo
    {
        int current_turn = 0;
        std::vector<Event> events;
    };

    class PROTO1_MODEL_SYMEXPORT TurnSolver
    {
    public:
        
        explicit TurnSolver(World& world);
        ~TurnSolver();

        TurnSolver(TurnSolver&& other);
        TurnSolver& operator=(TurnSolver&& other);
        TurnSolver(const TurnSolver&) = delete;
        TurnSolver& operator=(const TurnSolver&) = delete;

        auto start_until_player_turn() -> TurnInfo;

        auto play_action_until_next_turn(Action action) -> TurnInfo;

    private:
        using TurnSequence = tl::generator<TurnInfo>;

        World& world;
        std::optional<TurnSequence> turn_sequence;
        decltype(turn_sequence->begin()) turn_iterator;
        Action next_player_action;

        auto start_sequence() -> TurnSequence;
    };


}
