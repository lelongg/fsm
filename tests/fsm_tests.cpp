#include <catch/catch.hpp>
#include <fsm/fsm.h>
#include <iostream>

struct Event0 {};
struct Event1 {};

enum State
{
    NotInitialized,
    Initialized,
    Started,
    Stopped
};

template <State FromState, typename EventType, State ToState>
using Transition = FSM::Transition<State, FromState, EventType, ToState>;

TEST_CASE("FSM")
{
    using FSM::FSM;

    FSM<State,
        Transition<NotInitialized, Event0, Initialized>,
        Transition<Initialized, Event0, Started>,
        Transition<Started, Event1, Stopped>>
        fsm(NotInitialized);

    REQUIRE(fsm.process(Event1()) == NotInitialized);
    REQUIRE(fsm.process(Event0()) == Initialized);
    REQUIRE(fsm.process(Event1()) == Initialized);
    REQUIRE(fsm.process(Event0()) == Started);
    REQUIRE(fsm.process(Event0()) == Started);
    REQUIRE(fsm.process(Event1()) == Stopped);
    REQUIRE(fsm.process(Event0()) == Stopped);
    REQUIRE(fsm.process(Event1()) == Stopped);
}
