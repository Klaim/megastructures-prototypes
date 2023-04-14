
#include <proto1/view.hpp>


#include <magic_enum.hpp>

namespace proto1::view
{
    const Description &get_description(const EntityKind &kind)
    {
        static std::array<Description, magic_enum::enum_count<EntityKind>()> descriptions{
            Description{.letter = '@', .letter_color = sf::Color::White, .background_color = sf::Color::Black},
            Description{.letter = 'Q', .letter_color = sf::Color::Yellow, .background_color = sf::Color::Black},
            Description{.letter = '#', .letter_color = sf::Color::Black, .background_color = sf::Color::White},
        };

        return descriptions[static_cast<std::size_t>(kind)];
    }

    sf::Vector2f to_view_position(const model::Position &position)
    {
        return {position.x * GRID_SQUARE_SIZE, position.y * GRID_SQUARE_SIZE};
    }

    Entity::Entity(const sf::Font &font, const Description &desc, sf::Vector2f initial_position)
        : m_text{std::string{desc.letter}, font, 20}, m_background{sf::Vector2f{GRID_SQUARE_SIZE, GRID_SQUARE_SIZE}}
    {
        set_position(initial_position);
        m_text.setFillColor(desc.letter_color);
        m_background.setFillColor(desc.background_color);
    }

    void Entity::draw(sf::RenderWindow &window) const
    {
        window.draw(m_background);
        window.draw(m_text);
    }

    void Entity::set_position(sf::Vector2f new_position)
    {
        m_background.setPosition(new_position);
        m_text.setPosition(new_position);
    }

    namespace 
    {
        std::vector<Entity> create_view(const model::World &world, const Config &config)
        {
            std::vector<Entity> all_views;

            const auto &wall_desc = get_description(EntityKind::walls);
            for (const auto &wall_pos : world.area.walls)
                all_views.emplace_back(config.font, wall_desc, to_view_position(wall_pos));

            const auto &player_desc = get_description(EntityKind::player);
            const auto &npc_desc = get_description(EntityKind::npc);
            auto bodies_view = world.entities.view<const model::Body>();
            for (const auto &[entity_id, body] : bodies_view.each())
            {
                const bool is_player = world.is_controlled_by_player(body);
                const auto &desc = get_description(is_player ? EntityKind::player : EntityKind::npc);
                all_views.emplace_back(config.font, desc, to_view_position(body.position));
            }

            return all_views;
        }

     


    }

    

    Grid::Grid(const model::Size& size, const sf::Color& color, float square_size)
    {
        assert(size.height >= 0);
        assert(size.width >= 0);

        const std::size_t row_count = size.height + 1;
        const std::size_t column_count = size.width + 1;
        
        horizontal_lines.setPrimitiveType(sf::Lines);
        horizontal_lines.resize(row_count * 2);
        for(std::size_t line_idx = 0; line_idx < row_count; ++line_idx)
        {
            const auto left_vertex_idx = line_idx * 2;
            const auto right_vertex_idx = left_vertex_idx + 1;
        
            horizontal_lines[left_vertex_idx].position.y = square_size * line_idx;
            horizontal_lines[left_vertex_idx].position.x = 0;
            horizontal_lines[right_vertex_idx].position.y = square_size * line_idx;
            horizontal_lines[right_vertex_idx].position.x = square_size * size.height;
        }

        vertical_lines.setPrimitiveType(sf::Lines);
        vertical_lines.resize(column_count * 2);
        for(std::size_t line_idx = 0; line_idx < column_count; ++line_idx)
        {
            const auto left_vertex_idx = line_idx * 2;
            const auto right_vertex_idx = left_vertex_idx + 1;
        
            vertical_lines[left_vertex_idx].position.x = square_size * line_idx;
            vertical_lines[left_vertex_idx].position.y = 0;
            vertical_lines[right_vertex_idx].position.y = square_size * size.width;
            vertical_lines[right_vertex_idx].position.x = square_size * line_idx;
        }
        
        for(std::size_t idx = 0; idx < horizontal_lines.getVertexCount(); ++idx)
            horizontal_lines[idx].color = color;

        for(std::size_t idx = 0; idx < vertical_lines.getVertexCount(); ++idx)
            vertical_lines[idx].color = color;
    }


    void Grid::draw(sf::RenderWindow& window) const
    {
        window.draw(horizontal_lines);
        window.draw(vertical_lines);
    }


    View::View(model::World& world, Config view_config)
        : config(view_config)
        , world(world)
        , entities_views(create_view(world, config))
        , grid(world.area.size, sf::Color::Red, GRID_SQUARE_SIZE)
    {
    }

    void View::update(const model::TurnInfo& turn_info)
    {
        if(turn_info.events.empty())
            return;
        
        entities_views = create_view(world, config);
    }

    void View::draw(sf::RenderWindow& window) const
    {
        grid.draw(window);

        for(auto& entity_view : entities_views)
            entity_view.draw(window);
    }

}
