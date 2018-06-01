#ifndef FSM_H
#define FSM_H

#include <initializer_list>
#include <iostream>
#include <map>
#include <memory>
#include <type_traits>
#include <vector>

/// A type that represents a parameter pack of zero or more integers.
template <typename T, T... I>
struct integer_sequence
{
    static_assert(std::is_integral<T>::value, "Integral type");

    using type = T;

    static constexpr T size = sizeof...(I);

    /// Generate an integer_sequence with an additional element.
    template <T N>
    using append = integer_sequence<T, I..., N>;

    using next = append<size>;
};

template <typename T, T... I>
constexpr T integer_sequence<T, I...>::size;

template <std::size_t... I>
using index_sequence = integer_sequence<std::size_t, I...>;

namespace detail {
// Metafunction that generates an integer_sequence of T containing [0, N)
template <typename T, T Nt, std::size_t N>
struct iota
{
    static_assert(Nt >= 0, "N cannot be negative");

    using type = typename iota<T, Nt - 1, N - 1>::type::next;
};

// Terminal case of the recursive metafunction.
template <typename T, T Nt>
struct iota<T, Nt, 0ul>
{
    using type = integer_sequence<T>;
};
} // namespace detail

// make_integer_sequence<T, N> is an alias for integer_sequence<T, 0,...N-1>
template <typename T, T N>
using make_integer_sequence = typename detail::iota<T, N, N>::type;

template <int N>
using make_index_sequence = make_integer_sequence<std::size_t, N>;

// index_sequence_for<A, B, C> is an alias for index_sequence<0, 1, 2>
template <typename... Args>
using index_sequence_for = make_index_sequence<sizeof...(Args)>;

namespace FSM {

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
    integer_sequence<std::underlying_type_t<Index>, Indices...>)
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
    using underlaying = std::underlying_type_t<Index>;
    return make_array_helper(
        f, Index(), make_integer_sequence<underlaying, N>{});
}

template <typename StateType, typename EventType, typename... Transitions>
struct TransitionHelper;

template <typename StateType, typename... Transitions>
struct TransitionTable;

template <typename StateType, typename EventType, typename Transition, typename... Transitions>
struct TransitionTable<StateType, EventType, Transition, Transitions...>
{
    using TransitionHelper_ =
        TransitionHelper<StateType, EventType, Transition, Transitions...>;

    static constexpr auto max_state = TransitionHelper_::maxState();

    static constexpr auto transitions =
        make_array<StateType, max_state + 1>(TransitionHelper_::getToState);
};

template <typename StateType, typename EventType>
struct TransitionTable<StateType, EventType>
{
    using TransitionHelper_ = TransitionHelper<StateType, EventType>;

    static constexpr auto max_state = TransitionHelper_::maxState();

    static constexpr auto transitions =
        make_array<StateType, max_state + 1>(TransitionHelper_::getToState);
};

template <typename StateType, typename EventType, typename Transition, typename... Transitions>
struct TransitionHelper<StateType, EventType, Transition, Transitions...>
{
    static constexpr std::underlying_type_t<StateType> maxState()
    {
        constexpr auto from_state =
            static_cast<std::underlying_type_t<StateType>>(Transition::from_state);
        constexpr auto to_state =
            static_cast<std::underlying_type_t<StateType>>(Transition::to_state);
        return std::max(
            std::max(from_state, to_state),
            TransitionHelper<StateType, EventType, Transitions...>::maxState());
    }

    static constexpr StateType getToState(const std::underlying_type_t<StateType> from_state)
    {
        if (std::is_same<typename Transition::Event, EventType>()
            && StateType(from_state) == Transition::from_state)
        {
            return Transition::to_state;
        }
        else
        {
            return TransitionHelper<StateType, EventType, Transitions...>::getToState(
                from_state);
        }
    }
};

template <typename StateType, typename EventType>
struct TransitionHelper<StateType, EventType>
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
    using TransitionList = TransitionTable<StateType, Event, Transitions...>;

    template <StateType FromState, typename EventType, StateType ToState>
    using Transition = Transition<StateType, FromState, EventType, ToState>;

    FSM(StateType initial_state) : state_(initial_state) {}

    template <typename Event>
    StateType process(const Event&)
    {
        const auto state_as_underlying_type =
            static_cast<std::underlying_type_t<StateType>>(state_);

        if (TransitionList<Event>::max_state >= state_as_underlying_type)
        {
            state_ = TransitionList<Event>::transitions[state_as_underlying_type];
        }

        return state_;
    }

    StateType state_;
};
}; // namespace FSM

#endif
