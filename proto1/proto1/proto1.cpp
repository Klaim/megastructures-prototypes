#include <filesystem>
#include <exception>
#include <vector>
#include <array>

#include <boost/predef/os.h>
#include <boost/unordered/unordered_flat_map.hpp>
#include <fmt/core.h>

#include <imgui/imgui.h>
#include <imgui/backends/imgui-SFML.h>

#include <proto1-model/core.hpp>

#include <proto1/view.hpp>
#include <proto1/ui.hpp>



namespace proto1
{

    namespace fs = std::filesystem;
    static const fs::path data_dir{ "data/" };
    static const auto font_path = data_dir / "arial.ttf";

    consteval bool must_force_environment_to_local() {
        return BOOST_OS_WINDOWS or BOOST_OS_UNIX or BOOST_OS_MACOS;
    }

    enum class KeyState
    {
        up,
        down,
        just_up,
        just_down
    };

    inline std::string to_string(KeyState key_state)
    {
        switch (key_state)
        {
        case KeyState::up:
            return "up";
        case KeyState::just_up:
            return "just_up";
        case KeyState::down:
            return "down";
        case KeyState::just_down:
            return "just_down";
        default:
            return "unknown";
        }
    }

    class KeyStateTracker
    {
    public:
        KeyStateTracker() = default;

        explicit KeyStateTracker(sf::Keyboard::Key key_to_track)
            : key(key_to_track)
        {
        }

        KeyState get_state()
        {
            if (is_key_down_last_update && !has_key_just_changed)
            {
                return KeyState::down;
            }

            if (is_key_down_last_update && has_key_just_changed)
            {
                return KeyState::just_down;
            }

            if (!is_key_down_last_update && !has_key_just_changed)
            {
                return KeyState::up;
            }

            if (!is_key_down_last_update && has_key_just_changed)
            {
                return KeyState::just_up;
            }

            std::unreachable();
        }

        void update()
        {
            if(key == sf::Keyboard::Unknown)
                return;

            const bool is_key_down_now = sf::Keyboard::isKeyPressed(key);

            if (is_key_down_now)
            {
                if (is_key_down_last_update)
                {
                    has_key_just_changed = false;
                }
                else
                {
                    has_key_just_changed = true;
                }
            }
            else
            {
                if (is_key_down_last_update)
                {
                    has_key_just_changed = true;
                }
                else
                {
                    has_key_just_changed = false;
                }
            }

            is_key_down_last_update = is_key_down_now;
        }

    private:
        sf::Keyboard::Key key = sf::Keyboard::Unknown;
        bool is_key_down_last_update = false;
        bool has_key_just_changed = false;
    };
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
    model::TurnSolver turn_solver{ world };
    turn_solver.start_until_player_turn();

    view::View world_view{ world, std::move(view_config) };

    boost::unordered::unordered_flat_map<sf::Keyboard::Key, KeyStateTracker> keys_state{ 
        { sf::Keyboard::Space, KeyStateTracker{ sf::Keyboard::Space }}
    };

    sf::Clock deltaClock;
    // Start the game loop
    while (window.isOpen())
    {
        // Process events / acquire inputs
        sf::Event event;
        while (window.pollEvent(event))
        {
            ImGui::SFML::ProcessEvent(window, event);
            // Close window: exit
            if (event.type == sf::Event::Closed)
                window.close();
        }
        
        // update input
        for (auto&& [key, tracker] : keys_state)
        {
            tracker.update();
        }

        if (keys_state[sf::Keyboard::Space].get_state() == KeyState::just_down)
        {
            auto turns_info = turn_solver.play_action_until_next_turn(model::actions::Wait{});
            world_view.update(turns_info);
        }

        // update ui
        ImGui::SFML::Update(window, deltaClock.restart());
        ui::update_ui();

        // update the game's state

        model::TurnInfo turn_info;

        // update the view's state
        world_view.update(turn_info);

        // display
        window.clear();
        world_view.draw(window);
        ImGui::SFML::Render(window);
        window.display();
    }
    return EXIT_SUCCESS;
}