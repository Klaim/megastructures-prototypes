#ifndef GDEXAMPLE_H
#define GDEXAMPLE_H

#include <godot_cpp/classes/sprite2d.hpp>

namespace proto2
{

    class GDExample : public godot::Sprite2D
    {
        GDCLASS(GDExample, godot::Sprite2D)

    private:
    protected:
        static void _bind_methods();

    public:
        GDExample();
        ~GDExample();

        void _process(double delta);

        void _ready();
    };

}

#endif
