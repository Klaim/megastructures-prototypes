#pragma once

#include <SFML/Graphics.hpp>

#include <proto1-model/core.hpp>
#include <proto1-model/actionturn.hpp>

namespace proto1::view
{
    struct Description
    {
        char letter = '\0';
        sf::Color letter_color = sf::Color::Magenta;
        sf::Color background_color = sf::Color::White;
    };

    enum class EntityKind
    {
        player,
        npc,
        walls
    };

    const Description &get_description(const EntityKind &kind);

    inline constexpr float GRID_SQUARE_SIZE = 30.0f;

    sf::Vector2f to_view_position(const model::Position &position);

    class Entity
    {
        sf::Text m_text;
        sf::RectangleShape m_background;

    public:

        Entity(const Entity &) = default;
        Entity(Entity &&) noexcept = default;
        Entity &operator=(const Entity &) = default;
        Entity &operator=(Entity &&) noexcept = default;


        Entity(const sf::Font &font, const Description &desc, sf::Vector2f initial_position);

        
        void draw(sf::RenderWindow &window) const;

        void set_position(sf::Vector2f new_position);
    };

    struct Config
    {
        sf::Font font;
    };
    
    class Grid
    {
        sf::VertexArray horizontal_lines;
        sf::VertexArray vertical_lines;
    public:
        Grid(const model::Size& size, const sf::Color& color, float square_size);

        void draw(sf::RenderWindow& window) const;
    };


    class View
    {
    public:
        explicit View(model::World& world, Config config);

        void update(const model::TurnInfo& turn_info);

        void draw(sf::RenderWindow& window) const;
    private:
        Config config;
        model::World& world;
        std::vector<Entity> entities_views;
        Grid grid;
    };

}
