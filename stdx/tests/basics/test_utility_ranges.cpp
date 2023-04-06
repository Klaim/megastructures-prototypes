#include <doctest/doctest.h>

#include <vector>

#include <stdx/utility_ranges.hpp>

TEST_CASE("append_ranges - basics")
{
    const std::vector first_values { 1, 2, 3 };
    const std::vector second_values { 4, 5, 6 };
    const std::vector<int> no_values {  };

    SUBCASE("empty ranges")
    {
        auto x = stdx::append_ranges(no_values, no_values);
        CHECK(x.empty());

        auto a = stdx::append_ranges(x, first_values);
        CHECK_EQ(a, first_values);
    }

    SUBCASE("non-empty ranges")
    {
        auto x = stdx::append_ranges(first_values, second_values);
        CHECK_EQ(x, std::vector{ 1, 2, 3, 4, 5, 6 });
    }
}
