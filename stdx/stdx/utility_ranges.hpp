#pragma once

#include <ranges>

namespace stdx
{
    // Workaround gcc not implementing yet `std::<container>::append_range`
    auto append_ranges(std::ranges::forward_range auto&& left, std::ranges::forward_range auto&& right)
    {
        if constexpr(requires(){ left.append_range(right); })
        {
            left.append_range(right);
        }
        else
        {
            left.insert(begin(right), end(right), end(left));
        }
        return left;
    }

}