#include <catch/catch.hpp>
#include <fsm/fsm.h>

struct Event0;
struct Event1;

struct InitialState : State<0>
{
    static StateType nextState(const Event0&)
    {
        return 1;
    };
};

struct FinalState : State<1>
{
    static StateType nextState(const Event1&)
    {
        return 0;
    }
};

TEST_CASE("Transition creation")
{
    Transition<InitialState, Event, FinalState> transition;
}

TEST_CASE("Transition table creation")
{
    TransitionTable<
        Transition<InitialState, Event, FinalState>,
        Transition<InitialState, Event, FinalState>,
        Transition<InitialState, Event, FinalState>>
        transition_table;
}

TEST_CASE("FSM creation")
{
    FSM<TransitionTable<
        Transition<InitialState, Event, FinalState>, Transition<InitialState, Event, FinalState>,
        Transition<InitialState, Event, FinalState>>>
        fsm;
}

TEST_CASE("Final look")
{

}
