#include <filesystem>
#include <exception>

#include <boost/predef/os.h>

#include <fmt/core.h>
#include <SFML/Graphics.hpp>
#include <imgui/imgui.h>
#include <imgui/backends/imgui-SFML.h>

namespace fs = std::filesystem;

const fs::path data_dir{ "data/" };
const auto font_path = data_dir / "arial.ttf";


int main(int argc, char** args)
{
    if constexpr(BOOST_OS_WINDOWS or BOOST_OS_UNIX or BOOST_OS_MACOS)
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
    sf::RenderWindow window(sf::VideoMode(800, 600), "SFML window");
    ImGui::SFML::Init(window);
    
    sf::Font font;
    if (!font.loadFromFile(font_path.string()))
        throw std::runtime_error("no font file found");

    sf::Text text("Hello SFML", font, 50);

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

        window.draw(text);

        ImGui::SFML::Render(window);
        window.display();
    }
    return EXIT_SUCCESS;
}