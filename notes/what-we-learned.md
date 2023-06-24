
Proto 1:
--------


- Most entity-component-system library do not allow copy of the whole component data.
    -> Maybe make my own ecs system/library (probalby using "plf-colony/hive")
    -> Maybe work without value semantics?
    -> Maybe not use an ECS?

- Coroutine generator implementations are not all equal - chose them well.
    -> Use `tl::generator` until we have `std::generator`


Proto 2:
--------
(using Godot 4.1 Beta2)

- GDExtension dont need to be built with the same flags as Godot, just need godot-cpp to be.

- Godot doesnt do hot-reloading of GDExtensions yet, see
    - https://github.com/godotengine/godot/issues/66231
    - https://github.com/godotengine/godot/issues/54999
    - https://github.com/godotengine/godot-cpp/issues/955

- GDExtension classes inheriting Node like any GDSCript will behave like if they were GDScript but marked `@tool` and run in the editor. This is not intuitive and there is no way to make it "unmarked". There is some work ongoing to fix that:
    - https://github.com/godotengine/godot-cpp/issues/709

- Using fmt in a GDExtension seems to crash the game when running without a console. To investigate.
    - see https://github.com/godotengine/godot-cpp/issues/1144


