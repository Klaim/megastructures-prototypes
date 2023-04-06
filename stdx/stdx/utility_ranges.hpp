#pragma once

#include <ranges>
#include <random>
namespace stdx
{
    // Workaround gcc not implementing yet `std::<container>::append_range`
    decltype(auto) append_ranges(std::ranges::forward_range auto left, std::ranges::forward_range auto const& right)
    {
        if constexpr(requires(){ left.append_range(right); })
        {
            left.append_range(right);
        }
        else
        {
            left.insert(end(left), begin(right), end(right));
        }
        return left;
    }

    template<std::ranges::forward_range Range>
    auto pick_random_element(Range&& range, std::uniform_random_bit_generator auto& random_generator)
    {
        if (std::ranges::empty(range))
            return end(range);

        auto result_it = begin(range);
        if (range.size() == 1)
            return result_it;

        std::uniform_int_distribution<long long> uniform_dist{0, static_cast<long long>(range.size() - 1)};
        const auto random_idx = uniform_dist(random_generator);
        std::ranges::advance(result_it, random_idx);
        return result_it;
    }

}