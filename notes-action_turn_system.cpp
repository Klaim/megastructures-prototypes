// model

using ActorID = int;
struct Actor 
{
    bool is_player() const;
};


struct Body 
{
    std::optional<ActorID> actor_id;
    
    bool can_act() bool { return actor_id ? true : false; }
};

struct World {
    flat_map<ActorID, Actor> actors;
    std::vector<Body> bodies;

    bool has_player_bodies() const 
    {
        return not bodies.empty()
            && std::ranges::any_if(bodies, [](const auto& body){ return body.actor_id && actors[body.actor_id].is_player(); })
            ;
    }

};

struct Event{};

struct ActionResults
{
    World world;
    std::vector<Event> events;
};

struct Action
{
    auto execute(World world, Body body, Actor actor) -> ActionResults;
};


struct TurnUpdate
{
    World world;
    std::vector<Event> events;
};

class TurnSolver
{
    World world;
    std::generator<TurnUpdate> turns_generator;
    Action next_player_action;
public:

    TurnUpdate start_until_player_turn()
    {
        turns_generator = start_actions_turns();
        return *turns_generator;
    }

    auto play_action_until_next_turn(Action action) -> TurnUpdate
    {
        next_player_action = action;
        ++turns_generator;
        TurnUpdate turn = *turns_generator;
        return turn;
    }

private:
   
    auto start_actions_turns() -> std::generator<TurnUpdate>
    {
        TurnUpdate turn_info;
        while(world.has_player_bodies())
        {
            for(auto body : world.bodies)
            {
                if(body.can_act())
                    continue;

                auto actor = world.actors[body.actor_id.value()];

                const Action next_action = [&]{
                    if(actor.is_player())
                    {
                        co_yield turn_info;
                        turn_info = {};
                        return next_player_action;   
                    }
                    else
                    {
                        return actor.decide_next_action();
                    }
                }();

                ActionResults action_results = next_action.execute(world, body, actor);
                world = action_results.world;
                turn_info.events.push_back(action_results.events);

            }
        }
    }    


};


// view

World world
TurnSolver turn_solver{world};
auto last_world_update = turn_solver.start_until_player_turn();

while(playing)
{
    // interpret events
    for(auto event: last_world_update.events)
    {
        view.play_event(event);
    }
    world = last_world_update.world;

    // request player's action
    auto player_input = handle_input();
    Action player_action = deduce_action(player_input);
    last_world_update = turn_solver.play_action_until_next_turn(world, action);
}





