#include <proto2-model/combat.hpp>

namespace proto2::model::actions
{


    ActionResults Attack::execute(ActionContext action_context) const
    {
        const auto target_position = action_context.body_acting.position + relative_target;

        ActionResults results{
            .action_is_success = false,
            .events = {
                events::Attacked{
                    .attacker = action_context.body_acting.id,
                    .target = target_position,
                }
            }
        };

        static constexpr int attack_range = 1;
        const auto distance_to_target = action_context.body_acting.position.distance_to(target_position);
        if (distance_to_target > attack_range) // not in range?
            return results;

        auto maybe_target_id = action_context.world.entity_at(target_position);
        if(not maybe_target_id) // hit nothing?
            return results;


        static constexpr int chances_of_evasion = 50;
        const auto dice_roll = random_int(1, 100);

        if (dice_roll > 50)
        {
            const int attack_damages = damages;

            // apply the damages to the target
            auto& target_body = action_context.world.entities.get<Body>(*maybe_target_id);
            auto& target_damage_state = action_context.world.entities.get<DamageState>(*maybe_target_id);
            target_damage_state.take_damages(attack_damages);

            // notify that there was damages
            results.action_is_success = true;
            results.events.push_back(events::TookDamages{
                .target = *maybe_target_id,
                .attacker = action_context.body_acting.id,
                .damages = attack_damages,
            });

            // handle death case where the target have been destroyed
            if(target_damage_state.is_destroyed())
            {
                action_context.world.entities.destroy(*maybe_target_id);
                results.events.push_back(events::Destroyed{
                    .destroyed_id = *maybe_target_id,
                });
            }

        }
        else
        {
            results.events.push_back(events::Evaded{
                .evader = *maybe_target_id,
                .attacker = action_context.body_acting.id,
            });
        }

        return results;
    }


}