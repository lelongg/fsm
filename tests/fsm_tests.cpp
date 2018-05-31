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

template <typename T>
constexpr auto type_name_length = get_name<T>().size;

struct Event
{};
struct Event0
{};
struct Event1
{};

enum class State
{
    NotInitialized,
    Initialized,
    Started,
    Stopped
};

TEST_CASE("Transition creation")
{
    Transition<State, State::Started, Event0, State::Stopped> transition;
}

TEST_CASE("Transition table creation")
{
    using CustomTransitionTable = TransitionTable<
        State, Transition<State, State::NotInitialized, Event0, State::Initialized>,
        Transition<State, State::Initialized, Event0, State::Started>,
        Transition<State, State::Started, Event1, State::Stopped>>;

    auto a =
        CustomTransitionTable::TransitionList<Event1>::transitions[int(State::Started)];
    REQUIRE(int(a) == 3);
}

template <typename TransitionList>
void printTransitionList(const std::string& prefix)
{
    for (int i = 0; i < TransitionList::transitions.size(); ++i)
    {
        std::cout << prefix << ": " << i << " -> " << int(TransitionList::transitions[i]) << std::endl;
    }
}

TEST_CASE("FSM creation")
{
    FSM<State, Transition<State, State::NotInitialized, Event0, State::Initialized>,
        Transition<State, State::Initialized, Event0, State::Started>,
        Transition<State, State::Started, Event1, State::Stopped>>
        fsm(State::NotInitialized);

    printTransitionList<decltype(fsm)::TransitionList<Event0>>("Event0");
    printTransitionList<decltype(fsm)::TransitionList<Event1>>("Event1");

    REQUIRE(fsm.process(Event0()) == State::Initialized);
    REQUIRE(fsm.process(Event1()) != State::Started);
}
