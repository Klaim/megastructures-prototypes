#include <filesystem>
#include <exception>
#include <vector>
#include <array>

#include <boost/predef/os.h>
#include <fmt/core.h>

#include <imgui/imgui.h>
#include <imgui/backends/imgui-SFML.h>

#include <proto1-model/proto1-model.hpp>

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
    view::View world_view{ world, std::move(view_config) };

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