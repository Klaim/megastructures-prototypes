#include <proto2-view.hpp>

#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/engine.hpp>

#include <proto2-model/movement.hpp>

#include "godot-conversions.hpp"

namespace proto2
{
    void World::_bind_methods()
    {
        using namespace godot;
        ClassDB::bind_method(D_METHOD("get_walls_positions"), &World::get_walls_positions);
        ClassDB::bind_method(D_METHOD("get_characters_positions"), &World::get_characters_positions);
        ClassDB::bind_method(D_METHOD("get_player_positions"), &World::get_player_positions);
        ClassDB::bind_method(D_METHOD("player_action_wait"), &World::player_action_wait);
        ClassDB::bind_method(D_METHOD("player_action_move"), &World::player_action_move);

    }

    World::World() = default;
    World::~World() = default;

    void World::_ready()
    {
        Node::_ready();

        if( godot::Engine::get_singleton()->is_editor_hint() )
            return;

        m_turn_solver.start_until_player_turn();
    }

    auto World::get_walls_positions() const -> godot::TypedArray<godot::Vector2i>
    {
        godot::UtilityFunctions::print("Gathering walls positions...");
        godot::TypedArray<godot::Vector2i> result;
        for(const auto& wall_pos : m_world.area.walls)
            result.append(godot::Vector2i{ wall_pos.x, wall_pos.y });
        godot::UtilityFunctions::print("Gathering walls positions - DONE");
        return result;
    }

    auto World::get_characters_positions() const -> godot::Dictionary
    {
        godot::UtilityFunctions::print("Gathering characters positions...");
        godot::Dictionary result;
        auto bodies = m_world.entities.view<model::Body>();
        for(auto&& [id, body] : bodies.each())
            result[ static_cast<int64_t>(id) ] = godot::Vector2i{ body.position.x, body.position.y };
        godot::UtilityFunctions::print("Gathering characters positions - DONE");
        return result;
    }

    auto World::get_player_positions() const -> godot::TypedArray<godot::Vector2i>
    {
        godot::UtilityFunctions::print("Gathering player positions...");
        godot::TypedArray<godot::Vector2i> result;
        auto bodies = m_world.entities.view<model::Body>();
        for(auto&& [id, body] : bodies.each())
            if(body.actor_id && m_world.actors.at(body.actor_id.value()).is_player())
                result.append(godot::Vector2i{ body.position.x, body.position.y });
        godot::UtilityFunctions::print("Gathering player positions - DONE");
        return result;
    }

    auto World::play_action(model::AnyAction action) -> godot::Array
    {
        godot::Array events_sequence;

        godot::UtilityFunctions::print("Player's action: {}, processing turns...", action.type_id().name());
        const auto turn_info = m_turn_solver.play_action_until_next_turn(std::move(action));
        godot::UtilityFunctions::print("Processing turns - DONE");
        godot::UtilityFunctions::print("Current Turn = ", turn_info.current_turn);

        for(const auto& event : turn_info.events)
        {
            const std::string event_description = std::format("[{}]: {}", event.type_name(), event.text_description());
            auto event_godot = to_godot(event);
            godot::UtilityFunctions::print("  -> ", event_godot, " : ", event_description.c_str());
            events_sequence.push_back(event_godot);
        }

        return events_sequence;
    }

    auto World::player_action_wait() -> godot::Array
    {
        return play_action(model::actions::Wait{});
    }

    auto World::player_action_move(const godot::Vector2i& direction) -> godot::Array
    {
        return play_action(model::actions::Move{ model::Vector2{ direction.x, direction.y } });
    }

}

