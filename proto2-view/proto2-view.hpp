#pragma once

#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/typed_array.hpp>
#include <godot_cpp/classes/node.hpp>

#include <proto2-model/core.hpp>
#include <proto2-model/actionturn.hpp>

namespace proto2
{
    class World : public godot::Node
    {
        GDCLASS(World, godot::Node)

    public:

        World();
        ~World();

        auto get_walls_positions() const -> godot::TypedArray<godot::Vector2i>;
        auto get_characters_positions() const -> godot::Dictionary;
        auto get_player_positions() const -> godot::TypedArray<godot::Vector2i>;

        godot::Array player_action_wait();
        godot::Array player_action_move_up();
        godot::Array player_action_move_down();
        godot::Array player_action_move_left();
        godot::Array player_action_move_right();

        // Applies an action from the player and processes turns until it's the player's
        // turn again.
        // Returns a sequence of events as JSON objects that happened when processing turns.
        auto play_action(model::AnyAction action) -> godot::Array;


        void _ready() override;

    private:
        proto2::model::World m_world = model::create_test_world();
        proto2::model::TurnSolver m_turn_solver{ m_world };


    protected:
        static void _bind_methods();
    };
}
