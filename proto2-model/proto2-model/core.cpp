#include <proto2-model/core.hpp>

#include <random>
#include <functional>
#include <algorithm>

#include <proto2-model/actors.hpp>

namespace proto2::model
{


    namespace
    {

        int random_int(int min_value, int max_value)
        {
            static std::random_device random_device;
            static std::default_random_engine random_engine(random_device());
            std::uniform_int_distribution<int> uniform_dist(min_value, max_value);
            return uniform_dist(random_engine);
        }

        Position random_position(const Rectangle& area_section)
        {
            return {
                /*.x =*/ random_int(area_section.lowest_x(), area_section.highest_x() - 1),
                /*.y =*/ random_int(area_section.lowest_y(), area_section.highest_y() - 1),
            };
        }

        Position random_position(const Area& area)
        {
            return random_position(Rectangle{
                .position = {},
                .size = area.size
                });
        }

        Position random_free_position(const Area& area, int max_attempts = 1000)
        {
            while(max_attempts--)
            {
                auto new_pos = random_position(area);
                if(std::ranges::none_of(area.walls, [&](const auto& wall_pos){ return wall_pos == new_pos; }))
                {
                    return new_pos;
                }
            }
            return {};
        }

    }


    ActorID Actor::new_id()
    {
        static ActorID next_id = 0;
        return next_id++;
    }

    bool Area::is_wall(const Position& at_position) const
    {
        return not is_position_inside(at_position, Rectangle{ Vector2_ZERO, size })
            || std::ranges::find(walls, at_position) != walls.end();
    }

    bool World::is_free_position(const Position& position) const
    {
        return not area.is_wall(position)
            && std::ranges::none_of(entities.view<const Body>().each(), [&](auto&& data){
                const auto& [entity_id, body] = data;
               return body.position == position;
            });
    }

    bool World::is_controlled_by_player(const Body& body) const
    {
        if(not body.actor_id.has_value())
            return false;

        auto actor_it = actors.find(body.actor_id.value());
        if(actor_it == actors.end())
            return false;

        return actor_it->second.is_player();
    }

    bool World::has_player_bodies() const
    {
        auto bodies = entities.view<Body>();

        return not bodies.empty()
            && std::ranges::any_of(bodies.each(), [&](auto&& data){
                const auto& [entity, body] = data;
                return is_controlled_by_player(body);
            });
    }

    Area create_test_area(Size size, int wall_count)
    {
        Area area{
            .size = { size.width + 2, size.height + 2 },
        };

        const auto fill_line_with_walls = [&](int y){
            for(int x = 0; x < area.size.width; ++x)
                    area.walls.push_back({ x, y });
        };

        for(int y = 0; y < area.size.height; ++y)
        {
            if(y == 0 || y == area.size.height - 1)
            {
                fill_line_with_walls(y);
            }
            else
            {
                area.walls.push_back({ 0, y });
                area.walls.push_back({ area.size.width - 1, y });
            }
        }

        while(wall_count > 0)
        {
            auto new_wall_pos = random_free_position(area);
            area.walls.push_back(new_wall_pos);
            --wall_count;
        }

        return area;
    }

    void create_new_character(World& world, Actor actor)
    {
        const auto body_id = world.entities.create();
        const auto actor_id = Actor::new_id();
        world.actors.insert({ actor_id, std::move(actor) });
        world.entities.emplace<Body>(body_id, Body{
            .id = body_id,
            .position = random_free_position(world.area),
            .actor_id = actor_id,
        });
    }

    struct SpawnCharacter
    {
        ActionResults execute(ActionContext context) const
        {
            create_new_character(context.world, context.actor_deciding);
            return {};
        }
    };
    BOOST_DESCRIBE_STRUCT(SpawnCharacter, (), ());
    static_assert(Action<SpawnCharacter>);

    World create_test_world()
    {
        World world
        {
            .area = create_test_area({ 40, 40 }, 100),
        };

        create_new_character(world, Actor{ .kind = Actor::Kind::player });
        create_new_character(world, Actor{ .decide_next_action = actors::DoRandomAction{} });
        create_new_character(world, Actor{ .decide_next_action = actors::WalkUntilYouReachAWall{} });

        // auto actions = actors::default_random_actions();
        // actions.push_back(SpawnCharacter{});
        // create_new_character(world, Actor{ .decide_next_action = actors::DoRandomAction{
        //     .possible_actions = actions
        // } });

        return world;
    }
}

