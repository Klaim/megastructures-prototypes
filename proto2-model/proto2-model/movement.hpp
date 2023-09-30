#pragma once

#include <array>
#include <boost/describe.hpp>


#include <proto2-model/actionturn.hpp>


namespace proto2::model
{
    namespace events
    {
        struct Moved
        {
            EntityID body_id;
            Vector2 initial_position;
            Vector2 new_position;

            std::string text_description() const
            {
                return fmt::format("Body{{{}}} moved from {{ {}, {} }} to {{ {}, {} }}",
                    body_id,
                    initial_position.x, initial_position.y,
                    new_position.x, new_position.y
                );
            }
        };
        BOOST_DESCRIBE_STRUCT(Moved, (), (body_id, initial_position, new_position));
        static_assert(Event<Moved>);

        struct FailedToMove
        {
            EntityID body_id;
            Vector2 target_position;

            std::string text_description() const
            {
                return fmt::format("Body{{{}}} attempted to move to {{ {}, {} }} but failed",
                    body_id,
                    target_position.x, target_position.y
                );
            }
        };
        BOOST_DESCRIBE_STRUCT(FailedToMove, (), (body_id, target_position));
    }

    namespace actions
    {
        struct Move
        {
            Vector2 direction;

            constexpr Move() noexcept = default;
            constexpr explicit Move(Vector2 direction) noexcept : direction(std::move(direction)) {}

            constexpr Move(const Move& other) noexcept = default;
            constexpr Move(Move&& other) noexcept = default;
            constexpr Move& operator=(const Move& other) noexcept = default;
            constexpr Move& operator=(Move&& other) noexcept = default;

            ActionResults execute(ActionContext action_context) const
            {
                const auto initial_position = action_context.body_acting.position;
                const auto new_position = action_context.body_acting.position + direction;
                if (not action_context.world.is_free_position(new_position))
                    return {
                        .action_is_success = false,
                        .events = { events::FailedToMove{
                            .body_id = action_context.body_acting.id,
                            .target_position = new_position,
                        } }
                    };

                action_context.body_acting.position = new_position;
                return {
                    .events = { events::Moved{
                        .body_id  = action_context.body_acting.id,
                        .initial_position = initial_position,
                        .new_position = new_position,
                    }}
                };
            }

        };
        BOOST_DESCRIBE_STRUCT(Move, (), (direction));
        static_assert(Action<Move>);

        constexpr Move Move_UP { Vector2_UP };
        constexpr Move Move_DOWN { Vector2_DOWN };
        constexpr Move Move_LEFT { Vector2_LEFT };
        constexpr Move Move_RIGHT { Vector2_RIGHT };

        static constexpr std::array ADJACENT_MOVES{ Move_UP, Move_RIGHT, Move_DOWN, Move_LEFT };

    }
}