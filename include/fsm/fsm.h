#ifndef FSM_H
#define FSM_H

#include <initializer_list>
#include <iostream>
#include <map>
#include <memory>
#include <type_traits>
#include <vector>

template <typename EventType>
struct VirtualTransition
{
    void operator()(const EventType&) {}
};

template <typename StateType, StateType FromState, typename EventType, StateType ToState>
struct Transition : VirtualTransition<EventType>
{
    using Event = EventType;
    static const StateType from_state = FromState;
    static const StateType to_state = ToState;
};

template <class Function, typename Index, std::underlying_type_t<Index>... Indices>
constexpr auto make_array_helper(
    Function f,
    Index,
    std::integer_sequence<std::underlying_type_t<Index>, Indices...>)
    -> std::array<
        typename std::result_of<Function(std::underlying_type_t<Index>)>::type,
        sizeof...(Indices)>
{
    return {{f(Indices)...}};
}

template <typename Index, std::underlying_type_t<Index> N, typename Function>
constexpr auto make_array(Function f)
    -> std::array<typename std::result_of<Function(std::underlying_type_t<Index>)>::type, N>
{
    return make_array_helper(
        f, Index(), std::make_integer_sequence<std::underlying_type_t<Index>, N>{});
}

template <typename StateType, typename... Transitions>
struct TransitionHelper;

template <typename StateType, typename... Transitions>
struct TransitionTable;

template <typename StateType, typename Transition, typename... Transitions>
struct TransitionTable<StateType, Transition, Transitions...>
    : TransitionTable<StateType, Transitions...>
{
    template <typename EventType = typename Transition::Event>
    struct TransitionList
    {
        using TransitionHelper_ =
            TransitionHelper<StateType, Transition, Transitions...>;

        static constexpr auto max_state = TransitionHelper_::maxState();

        static constexpr auto transitions =
            make_array<StateType, max_state + 1>(TransitionHelper_::getToState);
    };
};

template <typename StateType>
struct TransitionTable<StateType>
{
    template <typename EventType>
    struct TransitionList
    {
        using TransitionHelper_ = TransitionHelper<StateType>;

        static constexpr auto max_state = TransitionHelper_::maxState();

        static constexpr auto transitions =
            make_array<StateType, max_state + 1>(TransitionHelper_::getToState);
    };
};

template <typename StateType, typename Transition, typename... Transitions>
struct TransitionHelper<StateType, Transition, Transitions...>
{
    static constexpr std::underlying_type_t<StateType> maxState()
    {
        constexpr auto from_state =
            static_cast<std::underlying_type_t<StateType>>(Transition::from_state);
        constexpr auto to_state =
            static_cast<std::underlying_type_t<StateType>>(Transition::to_state);
        return std::max(
            std::max(from_state, to_state),
            TransitionHelper<StateType, Transitions...>::maxState());
    }

    static constexpr StateType getToState(const std::underlying_type_t<StateType> from_state)
    {
        if (StateType(from_state) == Transition::from_state)
        {
            return Transition::to_state;
        }
        else if (
            TransitionTable<StateType, Transitions...>::template TransitionList<
                typename Transition::Event>::max_state
            > 0)
        {
            return TransitionTable<StateType, Transitions...>::template TransitionList<
                typename Transition::Event>::transitions[from_state];
        }
        else
        {
            return StateType(from_state);
        }
    }
};

template <typename StateType>
struct TransitionHelper<StateType>
{
    static constexpr std::underlying_type_t<StateType> maxState()
    {
        return static_cast<std::underlying_type_t<StateType>>(StateType());
    }

    static constexpr StateType getToState(const std::underlying_type_t<StateType> from_state)
    {
        return StateType(from_state);
    }
};

template <typename StateType, typename... Transitions>
struct FSM
{
    template <typename Event>
    using TransitionList =
        typename TransitionTable<StateType, Transitions...>::template TransitionList<Event>;

    FSM(StateType initial_state) : state_(initial_state) {}

    template <typename Event>
    StateType process(const Event&)
    {
        const auto state_as_underlying_type =
            static_cast<std::underlying_type_t<StateType>>(state_);
        std::cout << TransitionList<Event>::max_state << '\t'
                  << state_as_underlying_type << std::endl;
        if (TransitionList<Event>::max_state >= state_as_underlying_type)
        {
            state_ = TransitionList<Event>::transitions[state_as_underlying_type];
        }

        return state_;
    }

    StateType state_;
};

#endif
