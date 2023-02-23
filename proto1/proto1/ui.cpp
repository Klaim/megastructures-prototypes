#include <proto1/ui.hpp>

#include <imgui/imgui.h>

namespace proto1::ui
{
    void update_ui()
    {
        ImGui::Begin("Hello, world!");
        ImGui::Button("Look at this pretty button");
        ImGui::End();
    }
}


