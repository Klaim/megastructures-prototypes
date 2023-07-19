#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <proto2-model/version.hpp>
#include <proto2-model/core.hpp>
#include <proto2-model/actionturn.hpp>
#include <proto2-model/movement.hpp>

TEST_CASE( "dummy test" )
{
    CHECK( 1 == 1 );
}

TEST_CASE( "AnyAction works" )
{
    using namespace proto2;

    {
        model::AnyAction action = model::actions::Move{ model::Vector2_UP };
    }
    {
        model::AnyAction action(model::actions::Move{ model::Vector2_UP });
    }

    {
        model::AnyAction action(model::actions::Move_UP);
    }
    {
        model::AnyAction action{ model::actions::Move_UP };
    }
    {
        model::AnyAction action = model::actions::Move_UP;
    }
}

TEST_CASE( "Vector2 is constexpr" )
{
    using namespace proto2;
    static constexpr model::Vector2 x;
    static constexpr auto y = x;
    static constexpr auto z = x + y;
}

// #include <boost-te.hpp>

// struct Drawable {
//   void draw(std::ostream &out) const {
//     boost::te::call([](auto const &self, auto &out) { self.draw(out); }, *this, out);
//   }
// };

// struct Square {
//   void draw(std::ostream &out) const { out << "Square"; }
// };

// struct Circle {
//   void draw(std::ostream &out) const { out << "Circle"; }
// };

// struct Triangle {
//   void draw(std::ostream &out) const { out << "Triangle"; }
// };

// TEST_CASE( "should_erase_the_call" )
// {
//   boost::te::poly<Drawable> drawable{Square{}};

//   {
//     std::stringstream str{};
//     drawable.draw(str);
//     CHECK("Square" == str.str());
//   }

//   {
//     std::stringstream str{};
//     drawable = Circle{};
//     drawable.draw(str);
//     CHECK("Circle" == str.str());
//   }
// }
