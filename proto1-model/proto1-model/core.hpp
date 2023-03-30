#pragma once

#include <string>
#include <optional>
#include <vector>

#include <boost/container/flat_map.hpp>
#include <entt/entt.hpp>

#include <proto1-model/export.hpp>

namespace proto1::model 
{
    template<typename... Args>
    using flat_map = boost::container::flat_map<Args...>;

    struct Vector2
    {
        int x = 0;
        int y = 0;

        friend Vector2 operator+(const Vector2& left, const Vector2& right)
        {
            return { left.x + right.x, left.y + right.y };
        }

        bool operator==(const Vector2&) const = default;
    };

    using Position = Vector2;

    struct Size
    {
        int width;
        int height;
    };

    inline Position operator+(const Position& left, const Size& right)
    {
        return { left.x + right.width, left.y + right.height };
    }

    struct Rectangle
    {
        Position position;
        Size size;


        Position top_left() const { return position; }
        Position top_right() const { return { lowest_x(), lowest_y() }; }
        Position bottom_left() const { return { lowest_x(), highest_y() }; }
        Position bottom_right() const { return position + size; }

        int lowest_x() const { return position.x; }
        int lowest_y() const { return position.y; }
        int highest_x() const { return position.x + size.width; }
        int highest_y() const { return position.y + size.height; }
    };

    
    struct Event{}; // TODO: make this type-erasing wrapper?
    
    struct Action // TODO: make this a type-erasing wrapper (ideally using only values)
    {
    };

    using ActorID = int;

    struct Actor
    {
        static ActorID new_id();

        enum class Kind
        {
            not_player,
            player,
        };

        Actor::Kind kind = Kind::not_player;

        bool is_player() const { return kind == Kind::player; }

    };

    struct Body
    {
        Position position;
        std::optional<ActorID> actor_id;

        bool can_act() const { return actor_id.has_value(); }
    };


    struct PROTO1_MODEL_SYMEXPORT Area
    {
        Size size;
        std::vector<Position> walls;

        bool is_wall(const Position& at_position) const;

    };

    struct PROTO1_MODEL_SYMEXPORT World
    {
        Area area;
        flat_map<ActorID, Actor> actors;
        entt::registry entities;

        bool has_player_bodies() const;
        bool is_controlled_by_player(const Body& body) const;
    };


    PROTO1_MODEL_SYMEXPORT 
    Area create_test_area(Size size, int wall_count);

    PROTO1_MODEL_SYMEXPORT 
    World create_test_world();


}
