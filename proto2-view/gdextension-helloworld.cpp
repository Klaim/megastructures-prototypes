#include "gdextension-helloworld.hpp"

#include <fmt/printf.h>


#include <godot_cpp/core/class_db.hpp>

namespace godot {

    void GDExample::_bind_methods() {
    }

    GDExample::GDExample() = default;

    GDExample::~GDExample() = default;

    void GDExample::_process(double delta) {
        rotate(sin(delta));
    }

}