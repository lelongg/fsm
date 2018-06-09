#include <catch/catch.hpp>
#include <fsm/fsm.h>
#include <iostream>

struct string_view
{
    char const* data;
    std::size_t size;
};

inline std::ostream& operator<<(std::ostream& o, string_view const& s)
{
    return o.write(s.data, s.size);
}

template <class T>
constexpr string_view get_name()
{
    char const* p = __PRETTY_FUNCTION__;
    while (*p++ != '=')
        ;
    for (; *p == ' '; ++p)
        ;
    char const* p2 = p;
    int count = 1;
    for (;; ++p2)
    {
        switch (*p2)
        {
            case '[':
                ++count;
                break;
            case ']':
                --count;
                if (!count) return {p, std::size_t(p2 - p)};
        }
    }
    return {};
}

struct Event {};
struct Event0 {};
struct Event1 {};

enum State
{
    NotInitialized,
    Initialized,
    Started,
    Stopped
};

TEST_CASE("Transition creation")
{
    using FSM::Transition;

    Transition<State, Started, Event0, Stopped> transition;
}

TEST_CASE("Transition table creation")
{
//     using FSM::TransitionTable;
//     using FSM::Transition;
//
//     using CustomTransitionTable = TransitionTable<
//         State, Event1, Transition<State, NotInitialized, Event0, Initialized>,
//         Transition<State, Initialized, Event0, Started>,
//         Transition<State, Started, Event1, Stopped>>;

//     auto a =
//         CustomTransitionTable::transitions[int(Started)];
//     REQUIRE(int(a) == 3);
}

template <typename TransitionList>
void printTransitionList(const std::string& prefix)
{
//     for (size_t i = 0; i < TransitionList::transitions.size(); ++i)
//     {
//         std::cout << prefix << ": " << i << " -> "
//                   << int(TransitionList::transitions[i]) << std::endl;
//     }
}

template <State FromState, typename EventType, State ToState>
using Transition = FSM::Transition<State, FromState, EventType, ToState>;

TEST_CASE("FSM creation")
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
