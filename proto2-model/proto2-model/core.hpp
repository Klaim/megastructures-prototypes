#pragma once

#include <string>
#include <optional>
#include <vector>
#include <functional>
#include <format>

#include <boost/container/flat_map.hpp>
#include <boost/describe.hpp>
#include <entt/entt.hpp>
#include <stdx/function2.hpp>

#include <proto2-model/export.hpp>

namespace proto2::model
{
    template<typename... Args>
    using flat_map = boost::container::flat_map<Args...>;

    struct Vector2
    {
        int x = 0;
        int y = 0;

        constexpr Vector2(int x, int y) noexcept
            : x(x), y(y)
        {}

        constexpr Vector2() noexcept = default;
        constexpr ~Vector2() = default;
        constexpr Vector2(const Vector2& other)  noexcept = default;
        constexpr Vector2(Vector2&& other)  noexcept = default;
        constexpr Vector2& operator=(const Vector2& other) noexcept = default;
        constexpr Vector2& operator=(Vector2&& other)  noexcept = default;

        friend constexpr
        Vector2 operator+(const Vector2& left, const Vector2& right) noexcept
        {
            return { left.x + right.x, left.y + right.y };
        }

        constexpr bool operator==(const Vector2&) const noexcept = default;

        constexpr Vector2 left() const noexcept;
        constexpr Vector2 right() const noexcept;
        constexpr Vector2 up() const noexcept;
        constexpr Vector2 down() const noexcept;

        int distance_to(const Vector2& other) const noexcept;

    };
    static_assert(std::regular<Vector2>);
    BOOST_DESCRIBE_STRUCT(Vector2, (), (x, y));

    static constexpr Vector2 Vector2_ZERO   = Vector2{};
    static constexpr Vector2 Vector2_UP     = Vector2{0, -1};
    static constexpr Vector2 Vector2_DOWN   = Vector2{0, 1};
    static constexpr Vector2 Vector2_LEFT   = Vector2{-1, 0};
    static constexpr Vector2 Vector2_RIGHT  = Vector2{1, 0};

    constexpr Vector2 Vector2::left() const noexcept { return *this + Vector2_LEFT; }
    constexpr Vector2 Vector2::right() const noexcept { return *this + Vector2_RIGHT; }
    constexpr Vector2 Vector2::up() const noexcept { return *this + Vector2_UP; }
    constexpr Vector2 Vector2::down() const noexcept { return *this + Vector2_DOWN; }
    inline int Vector2::distance_to(const Vector2& other) const noexcept
    {
        return std::abs(x - other.x) + std::abs(y - other.y);
    }

    using Position = Vector2;

    struct Size
    {
        int width = 0;
        int height = 0;
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


    inline bool is_position_inside(const Position& position, const Rectangle& rectangle)
    {
        return position.x >= rectangle.lowest_x()
            && position.x < rectangle.highest_x()
            && position.y >= rectangle.lowest_y()
            && position.y < rectangle.highest_y();
    }

    using ActorID = int;
    using EntityID = entt::registry::entity_type;


    class AnyAction;
    class ActionContext;

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

        // fu2::unique_function<AnyAction (ActionContext)> decide_next_action;
        std::function<AnyAction (ActionContext)> decide_next_action;
    };

    struct Body
    {
        EntityID id = {};
        Position position;
        std::optional<ActorID> actor_id;
        bool last_action_failed = false;

        bool can_act() const { return actor_id.has_value(); }
    };

    struct DamageState
    {
        int dammages = 0;
        int destruction_threashold = 1;

        bool is_destroyed() const { return dammages >= destruction_threashold; }
        bool is_alive() const { return not is_destroyed(); }

        void take_damages(int additional_damages)
        {
            dammages += additional_damages;
        }
    };


    struct PROTO2_MODEL_SYMEXPORT Area
    {
        Size size;
        std::vector<Position> walls;

        bool is_wall(const Position& at_position) const;

    };

    struct PROTO2_MODEL_SYMEXPORT World
    {
        Area area;
        flat_map<ActorID, Actor> actors;
        entt::registry entities;

        bool has_player_bodies() const;
        bool is_controlled_by_player(const Body& body) const;

        bool is_free_position(const Position& position) const;
        std::optional<EntityID> entity_at(const Position& position) const;
    };


    PROTO2_MODEL_SYMEXPORT
    Area create_test_area(Size size, int wall_count);

    PROTO2_MODEL_SYMEXPORT
    World create_test_world();

    PROTO2_MODEL_SYMEXPORT
    void create_new_character(World& world, Actor actor);

    PROTO2_MODEL_SYMEXPORT
    int random_int(int min_value, int max_value);

    PROTO2_MODEL_SYMEXPORT
    Position random_position(const Rectangle &area_section);

    PROTO2_MODEL_SYMEXPORT
    Position random_position(const Area &area);

    PROTO2_MODEL_SYMEXPORT
    Position random_free_position(const Area &area, int max_attempts = 1000);

}

template <> struct std::formatter<proto2::model::EntityID> : std::formatter<unsigned long>
{
    auto format(proto2::model::EntityID id, std::format_context& ctx) const
    {
        return formatter<unsigned long>::format(static_cast<unsigned long>(id), ctx);
    }
};
