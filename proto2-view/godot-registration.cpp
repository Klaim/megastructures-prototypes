
#include "gdexample.hpp"
#include "proto2-view.hpp"

#include "export.hpp"

#include <gdextension_interface.h>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/godot.hpp>

namespace proto2
{
    void initialize_example_module(godot::ModuleInitializationLevel p_level)
    {
        if (p_level != godot::MODULE_INITIALIZATION_LEVEL_SCENE)
            return;

        godot::ClassDB::register_class<GDExample>();
        godot::ClassDB::register_class<World>();
    }

    void uninitialize_example_module(godot::ModuleInitializationLevel p_level)
    {
        if (p_level != godot::MODULE_INITIALIZATION_LEVEL_SCENE)
            return;
    }

}

extern "C"
{
    PROTO2_VIEW_SYMEXPORT
    auto init_megastructures_proto2_gdextension(GDExtensionInterfaceGetProcAddress p_get_proc_address, const GDExtensionClassLibraryPtr p_library, GDExtensionInitialization *r_initialization)
        -> GDExtensionBool
    {
        godot::GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);

        init_obj.register_initializer(proto2::initialize_example_module);
        init_obj.register_terminator(proto2::uninitialize_example_module);
        init_obj.set_minimum_library_initialization_level(godot::MODULE_INITIALIZATION_LEVEL_SCENE);

        return init_obj.init();
    }
}