#include "gdextension-helloworld.hpp"
#include <godot_cpp/core/class_db.hpp>

namespace godot {

    void GDExample::_bind_methods() {
    }

    GDExample::GDExample() {
        // Initialize any variables here.
        time_passed = 0.0;
    }

    GDExample::~GDExample() {
        // Add your cleanup here.
    }

    void GDExample::_process(double delta) {
        time_passed += delta;

        Vector2 new_position = Vector2(10.0 + (10.0 * sin(time_passed * 2.0)), get_position().y);

        set_position(new_position);
    }

}