#pragma once

#include <any>
#include <string>
#include <ranges>
#include <unordered_map>

#include <godot_cpp/variant/variant.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/classes/json.hpp>

#include <proto2-model/core.hpp>
#include <proto2-model/actionturn.hpp>
#include <proto2-model/movement.hpp>

namespace proto2
{

    template<proto2::model::ReflectionReady T>
    godot::Variant to_godot(const T& value)
    {

        // 1. convert to json
        // 2. convert to godot::Variant using Godot's json parser

        const auto json_string = model::reflection(value).dump();
        godot::Variant godot_value = godot::JSON::parse_string(json_string.c_str());

        return godot_value;
    }


}
