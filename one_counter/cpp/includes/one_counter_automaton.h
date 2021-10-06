#pragma once

#include <boost/graph/adjacency_list.hpp>

namespace active_learning {

    // Used for pair inside set
    // Comparable pair
    template<class T1, class T2>
    struct pair_comp {

        pair_comp(T1 e1, T2 e2) : first(e1), second(e2) {}

        bool operator==(const pair_comp<T1, T2> &p1) const {
            return p1.first == first and p1.second == second;
        }

        bool operator<(const pair_comp<T1, T2> &p1) const {
            return (first + second * 65536) > (p1.first + p1.second * 65536);
        }

        T1 first;
        T2 second;
    };

    struct automaton_vertex {
    };

    struct automaton_edge {
    };

    template<class T1, class T2>
    pair_comp<T1, T2> make_pair_comp(T1 e1, T2 e2) {
        return pair_comp(e1, e2);
    }

    enum class automaton_type {
        V1CA,
        R1CA
    };

    // TODO later when everything works
    class one_counter_automaton {

    public:
        automaton_type get_automaton_type() const;

    protected:
        one_counter_automaton() = default;

    protected:
        automaton_type automaton_type_;
    };
}