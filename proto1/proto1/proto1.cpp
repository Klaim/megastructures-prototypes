#include <SFML/Graphics.hpp>
#include <imgui/imgui.h>
#include <imgui/backends/imgui-SFML.h>

int main()
{
    // Create the main window
    sf::RenderWindow window(sf::VideoMode(800, 600), "SFML window");
    ImGui::SFML::Init(window);
    
    sf::Clock deltaClock;
    // Start the game loop
    while (window.isOpen())
    {
        // Process events
        sf::Event event;
        while (window.pollEvent(event))
        {
            // Close window: exit
            if (event.type == sf::Event::Closed)
                window.close();
        }

        ImGui::SFML::Update(window, deltaClock.restart());

        ImGui::Begin("Hello, world!");
        ImGui::Button("Look at this pretty button");
        ImGui::End();

        window.clear();
        ImGui::SFML::Render(window);
        window.display();
    }
    return EXIT_SUCCESS;
}