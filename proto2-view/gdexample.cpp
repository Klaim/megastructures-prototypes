#include "gdexample.hpp"

#include <chrono>

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

namespace proto2
{

    void GDExample::_bind_methods()
    {
    }

    GDExample::GDExample() = default;

    GDExample::~GDExample() = default;

    void GDExample::_process(double delta)
    {
        Node::_process(delta);

        static auto next_print = std::chrono::high_resolution_clock::now();
        const auto start_new_cycle = [&]{
            next_print = std::chrono::high_resolution_clock::now() + std::chrono::seconds{1};
        };

        const auto now = std::chrono::high_resolution_clock::now();
        if(next_print <= now)
        {
            godot::UtilityFunctions::print("GDExample - XXXXXXXXXXXX");
            start_new_cycle();
        }

        if( godot::Engine::get_singleton()->is_editor_hint() )
            return;

        rotate(sin(delta));
    }

    void GDExample::_ready()
    {
    }

}