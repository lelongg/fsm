#ifndef FSM_H
#define FSM_H

#include <initializer_list>
#include <iostream>
#include <map>
#include <memory>
#include <type_traits>
#include <vector>

namespace FSM {

template <typename EventType>
struct VirtualTransition
{
    void operator()(const EventType&) {}
};

template <
    typename StateType,
    StateType FromState,
    typename EventType,
    StateType ToState>
struct Transition : VirtualTransition<EventType>
{
    using Event = EventType;
    static const StateType from_state = FromState;
    static const StateType to_state = ToState;
};

template <class Function, std::size_t... Indices>
constexpr auto make_array_helper(Function f, std::index_sequence<Indices...>)
    -> std::array<
        typename std::result_of<Function(std::size_t)>::type,
        sizeof...(Indices)>
{
    return {{f(Indices)...}};
}

template <int N, class Function>
constexpr auto make_array(Function f)
    -> std::array<typename std::result_of<Function(std::size_t)>::type, N>
{
    return make_array_helper(f, std::make_index_sequence<N>{});
}

template <typename StateType, typename EventType, typename... Transitions>
constexpr std::underlying_type_t<StateType> MaxState()
{
    return std::max({Transitions::from_state...});
}

template <typename StateType, typename EventType, typename... Transitions>
struct TransitionHelper;

namespace detail {
template <
    typename StateType,
    typename EventType,
    typename Transition,
    typename... Transitions>
std::array<
    StateType,
    std::size_t(MaxState<StateType, EventType, Transitions...>() + 1)>
transitions()
{
    using TransitionHelper_ =
        TransitionHelper<StateType, EventType, Transition, Transitions...>;
    static std::array<
        StateType,
        std::size_t(MaxState<StateType, EventType, Transitions...>() + 1)>
        transition_array = make_array<std::size_t(
            MaxState<StateType, EventType, Transitions...>() + 1)>(
            TransitionHelper_::getToState);
    return transition_array;
}
} // namespace detail

template <
    typename StateType,
    typename EventType,
    typename Transition,
    typename... Transitions>
static const auto& transitions =
    detail::transitions<StateType, EventType, Transition, Transitions...>();

template <
    typename StateType,
    typename EventType,
    typename Transition,
    typename... Transitions>
struct TransitionHelper<StateType, EventType, Transition, Transitions...>
{
    static constexpr StateType getToState(const std::size_t from_state)
    {
        if (std::is_same<typename Transition::Event, EventType>()
            && StateType(from_state) == Transition::from_state)
        {
            return Transition::to_state;
        }
        else
        {
            return TransitionHelper<
                StateType, EventType, Transitions...>::getToState(from_state);
        }
    }
};

template <typename StateType, typename EventType>
struct TransitionHelper<StateType, EventType>
{
    static constexpr StateType getToState(const std::size_t from_state)
    {
        return StateType(from_state);
    }
};

template <typename StateType, typename... Transitions>
struct FSM
{
    template <StateType FromState, typename EventType, StateType ToState>
    using Transition = Transition<StateType, FromState, EventType, ToState>;

    FSM(StateType initial_state) : state_(initial_state) {}

    template <typename Event>
    StateType process(const Event&)
    {
        const auto state_as_underlying_type =
            static_cast<std::underlying_type_t<StateType>>(state_);

        if (MaxState<StateType, Event, Transitions...>()
            >= state_as_underlying_type)
        {
            state_ = transitions<
                StateType, Event, Transitions...>[state_as_underlying_type];
        }

        return state_;
    }

    StateType state_;
};
}; // namespace FSM

#endif
