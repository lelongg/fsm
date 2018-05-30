#ifndef FSM_H
#define FSM_H

#include <atomic>
#include <type_traits>
#include <initializer_list>

template <class...>
struct conjunction : std::true_type
{
};
template <class B1>
struct conjunction<B1> : B1
{
};
template <class B1, class... Bn>
struct conjunction<B1, Bn...>
    : std::conditional_t<bool(B1::value), conjunction<Bn...>, B1>
{
};

using StateType = int;

struct Event;

struct VirtualState
{
};

template <StateType state_value>
struct State : VirtualState
{
    static const StateType value = state_value;
};

template <StateType state, StateType other_state>
struct is_same_state : std::false_type
{
};

template <StateType state>
struct is_same_state<state, state> : std::true_type
{
};

template <typename FromStateType, typename EventType, typename ToState>
class Transition
{
public:
    using FromState = FromStateType;
    using Event = EventType;
    static const StateType from_state = FromState::value;
    static const StateType to_state = ToState::value;
};

template <typename Transition, typename... Transitions>
constexpr std::initializer_list<StateType> buildTransitionMap()
{
    return {Transition::to_state, buildTransitionMap<Transitions...>()};
}

template <typename Transition>
constexpr std::initializer_list<StateType> buildTransitionMap()
{
    return {Transition::to_state};
}

template <typename... Transitions>
struct TransitionMap
{
    static constexpr StateType transition_map[] = buildTransitionMap<Transitions...>();
};

template <typename Transition, typename... Transitions>
class TransitionTable : public TransitionTable<Transitions...>
{
public:
    template <
        StateType from_state_value,
        typename Event,
        typename = std::enable_if_t<conjunction<
            is_same_state<from_state_value, Transition::FromState::value>,
            std::is_same<Event, typename Transition::Event>>::value>>

        inline constexpr static StateType nextState()
    {
        return Transition::to_state;
    }
};

template <typename Transition>
class TransitionTable<Transition>
{
public:
    template <
        StateType from_state_value,
        typename Event,
        typename = std::enable_if_t<conjunction<
            is_same_state<from_state_value, Transition::FromState::value>,
            std::is_same<Event, typename Transition::Event>>::value>>

    inline constexpr static StateType nextState()
    {
        return Transition::to_state;
    }
};

template <typename TransitionTable>
class FSM
{
public:

    void process(const Event&)
    {
        state_ = new State<0>;
//         current_state_ = TransitionTable::template nextState<Event>(current_state_);
    }

    StateType state() const
    {
        return current_state_;
    }

private:
    std::atomic<StateType> current_state_;
    VirtualState * state_;
};

#endif
