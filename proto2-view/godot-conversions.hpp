#pragma once

#include <boost/pfr.hpp>
#include <boost/type_index.hpp>

#include <godot_cpp/variant/variant.hpp>
#include <godot_cpp/variant/dictionary.hpp>

namespace proto2
{
    std::string type_name(const auto& event_or_action) // TODO: constrain the type
    {
        std::string name = event_or_action.type_id().pretty_name();

        static const std::string struct_prefix = "struct ";
        name.erase(name.find(struct_prefix), struct_prefix.size());

        return name;
    }


}
