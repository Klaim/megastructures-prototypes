#pragma once

#include <proto2-model/actionturn.hpp>

namespace proto2::model
{
    namespace events
    {
        struct Attacked
        {
            EntityID attacker = {};
            Vector2 target = {};

            std::string text_description() const
            {
                return std::format("Body{{{}}} attacked targetting {{{}, {}}}", attacker, target.x, target.y);
            }
        };
        BOOST_DESCRIBE_STRUCT(Attacked, (), (attacker, target));
        static_assert(Event<Attacked>);

        struct TookDamages
        {
            EntityID target = {};
            EntityID attacker = {};
            int damages = 0;

            std::string text_description() const
            {
                return std::format("Body{{{}}} took {} damages from Body{{{}}}'s attack", target, damages, attacker);
            }

        };
        BOOST_DESCRIBE_STRUCT(TookDamages, (), (target, damages));
        static_assert(Event<TookDamages>);

        struct Evaded
        {
            EntityID evader = {};
            EntityID attacker = {};

            std::string text_description() const
            {
                return std::format("Body{{{}}} evaded the attack from Body{{{}}}", evader, attacker);
            }
        };
        BOOST_DESCRIBE_STRUCT(Evaded, (), (evader, attacker));
        static_assert(Event<Evaded>);

        struct Destroyed
        {
            EntityID destroyed_id = {};

            std::string text_description() const
            {
                return std::format("Body{{{}}} is destroyed", destroyed_id);
            }
        };
        BOOST_DESCRIBE_STRUCT(Destroyed, (), (destroyed_id));
        static_assert(Event<Destroyed>);


    }

    namespace actions
    {
        struct Attack
        {
            static const int default_damages = 1;

            Vector2 relative_target = {};
            int damages = default_damages;

            ActionResults execute(ActionContext action_context) const;
        };
        BOOST_DESCRIBE_STRUCT(Attack, (), (relative_target, damages));
        static_assert(Action<Attack>);
    }
}


