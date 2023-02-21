#include <filesystem>
#include <exception>
#include <vector>
#include <array>

#include <boost/predef/os.h>
#include <magic_enum.hpp>
#include <fmt/core.h>
#include <SFML/Graphics.hpp>
#include <imgui/imgui.h>
#include <imgui/backends/imgui-SFML.h>

#include <proto1-model/proto1-model.hpp>


namespace proto1
{

    namespace fs = std::filesystem;

    namespace view
    {
        struct Description
        {
            char letter = '\0';
            sf::Color letter_color = sf::Color::Magenta;
            sf::Color background_color = sf::Color::White;
        };

        
        enum class EntityKind
        {
            player, npc, walls
        };
        

        const Description& get_description(const EntityKind& kind)
        {
            static std::array<Description, magic_enum::enum_count<EntityKind>()> descriptions{
                Description{ .letter = '@'  , .letter_color = sf::Color::White  , .background_color = sf::Color::Black  },
                Description{ .letter = 'Q'  , .letter_color = sf::Color::Yellow , .background_color = sf::Color::Black  },
                Description{ .letter = '#'  , .letter_color = sf::Color::Black  , .background_color = sf::Color::White  },
            };

            return descriptions[static_cast<std::size_t>(kind)];
        }
        
        constexpr float GRID_SQUARE_SIZE = 30.0f;

        sf::Vector2f to_view_position(const model::Position& position)
        {
            return { position.x * GRID_SQUARE_SIZE, position.y * GRID_SQUARE_SIZE };
        }


        class Entity
        {
            sf::Text m_text;
            sf::RectangleShape m_background;
        public:

            Entity(const sf::Font& font, const Description& desc, sf::Vector2f initial_position)
                : m_text{ std::string{desc.letter}, font, 20 }
                , m_background{ sf::Vector2f{ GRID_SQUARE_SIZE, GRID_SQUARE_SIZE } }
            {
                set_position(initial_position);
                m_text.setFillColor(desc.letter_color);
                m_background.setFillColor(desc.background_color);
            }

            Entity(const Entity&) = default;
            Entity(Entity&&) noexcept = default;
            Entity& operator=(const Entity&) = default;
            Entity& operator=(Entity&&) noexcept = default;

            void draw(sf::RenderWindow& window) const
            {
                window.draw(m_background);
                window.draw(m_text);
            }

            void set_position(sf::Vector2f new_position)
            {
                m_background.setPosition(new_position);
                m_text.setPosition(new_position);
            }

        };

        struct Config
        {
            sf::Font font;
        };

        std::vector<Entity> create_view(const model::World& world, const Config& config)
        {
            std::vector<Entity> all_views;

            const auto& wall_desc = get_description(EntityKind::walls);
            for(const auto& wall_pos : world.area.walls)
                all_views.emplace_back(config.font, wall_desc, to_view_position(wall_pos));

            const auto& player_desc = get_description(EntityKind::player);
            const auto& npc_desc = get_description(EntityKind::npc);
            auto bodies_view = world.entities_compoments.view<const model::Body>();
            for(const auto& [entity_id, body] : bodies_view.each())
            {
                const auto& body_ref = body;
                const bool is_player = [&]{
                    if(not body_ref.controlling_actor_id)
                        return false;
                    auto actor_it = world.actors.find(body_ref.controlling_actor_id.value());
                    if(world.actors.end() == actor_it)
                        return false;

                    return actor_it->second.kind == model::Actor::Kind::player;
                }();
                const auto& desc = get_description( is_player ? EntityKind::player : EntityKind::npc );
                all_views.emplace_back(config.font, desc, to_view_position(body.position));

            }

            return all_views;
        }

    }

    static const fs::path data_dir{ "data/" };
    static const auto font_path = data_dir / "arial.ttf";

    consteval bool must_force_environment_to_local() {
        return BOOST_OS_WINDOWS or BOOST_OS_UNIX or BOOST_OS_MACOS;
    }

}

int main(int argc, char** args)
{
    using namespace proto1;

    if constexpr( must_force_environment_to_local() )
    {
        // Force the directory where this executable is be the root of our environment
        // from this point on.
        const fs::path this_exe_path = args[0];
        const auto this_exe_install_dir = this_exe_path.parent_path();
        fs::current_path(this_exe_install_dir);
    }

    if( not fs::is_directory(data_dir) )
        throw std::runtime_error("no data directory found");

    // Create the main window
    sf::RenderWindow window(sf::VideoMode(800, 600), "SFML window", sf::Style::Default | sf::Style::Resize);
    ImGui::SFML::Init(window);

    view::Config view_config;
    if (!view_config.font.loadFromFile(font_path.string()))
        throw std::runtime_error("no font file found");

    auto world = model::create_test_world();
    std::vector<view::Entity> entities = view::create_view(world, view_config);

    sf::Clock deltaClock;
    // Start the game loop
    while (window.isOpen())
    {
        // Process events
        sf::Event event;
        while (window.pollEvent(event))
        {
            ImGui::SFML::ProcessEvent(window, event);
            // Close window: exit
            if (event.type == sf::Event::Closed)
                window.close();
        }
        ImGui::SFML::Update(window, deltaClock.restart());

        ImGui::Begin("Hello, world!");
        ImGui::Button("Look at this pretty button");
        ImGui::End();

        window.clear();

        for(const auto& entity : entities)
        {
            entity.draw(window);
        }

        ImGui::SFML::Render(window);
        window.display();
    }
    return EXIT_SUCCESS;
}