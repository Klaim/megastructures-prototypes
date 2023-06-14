#include "gdextension-helloworld.hpp"

#include <fmt/printf.h>

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/engine.hpp>

namespace proto2
{

    void GDExample::_bind_methods()
    {
    }

    GDExample::GDExample() = default;

    GDExample::~GDExample() = default;

    void GDExample::_process(double delta)
    {
        if( godot::Engine::get_singleton()->is_editor_hint() )
            return;

        rotate(sin(delta));
    }

    void GDExample::_ready()
    {
        fmt::println("GDExample : ready!");
    }

}