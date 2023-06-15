#include <proto2-view.hpp>

#include <godot_cpp/variant/utility_functions.hpp>

namespace proto2
{
    void World::_bind_methods()
    {
        using namespace godot;
        ClassDB::bind_method(D_METHOD("get_walls_positions"), &World::get_walls_positions);
        ClassDB::bind_method(D_METHOD("get_characters_positions"), &World::get_characters_positions);
        ClassDB::bind_method(D_METHOD("get_player_positions"), &World::get_player_positions);
        ClassDB::bind_method(D_METHOD("player_action_wait"), &World::player_action_wait);

    }

    World::World() = default;
    World::~World() = default;

    void World::_ready()
    {
        m_turn_solver.start_until_player_turn();
    }

    auto World::get_walls_positions() const -> godot::TypedArray<godot::Vector2i>
    {
        godot::TypedArray<godot::Vector2i> result;
        for(const auto& wall_pos : m_world.area.walls)
            result.append(godot::Vector2i{ wall_pos.x, wall_pos.y });
        return result;
    }

    auto World::get_characters_positions() const -> godot::TypedArray<godot::Vector2i>
    {
        auto bodies = m_world.entities.view<model::Body>();
        godot::TypedArray<godot::Vector2i> result;
        for(auto&& [id, body] : bodies.each())
            result.append(godot::Vector2i{ body.position.x, body.position.y });
        return result;
    }

    auto World::get_player_positions() const -> godot::TypedArray<godot::Vector2i>
    {
        auto bodies = m_world.entities.view<model::Body>();
        godot::TypedArray<godot::Vector2i> result;
        for(auto&& [id, body] : bodies.each())
            if(body.actor_id && m_world.actors.at(body.actor_id.value()).is_player())
                result.append(godot::Vector2i{ body.position.x, body.position.y });
        return result;
    }

    void World::player_action_wait()
    {
        godot::UtilityFunctions::print("Player waits");
        m_turn_solver.play_action_until_next_turn(model::actions::Wait{});
    }

}

