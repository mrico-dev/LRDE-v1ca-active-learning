#pragma once

#include <boost/graph/adjacency_list.hpp>
#include "alphabet.h"
#include "displayable.h"

namespace active_learning {

    class one_counter_automaton : public displayable {

    public:
        struct transition_x {
            size_t state;
            size_t counter;
            char symbol;

            // Making it comparable so it can fit into a map
            bool operator==(const transition_x &other) const;

            bool operator<(const transition_x &other) const;
        };

        using state_t = size_t;
        using init_trans_t = std::pair<state_t, state_t>;
        using new_edges_t = std::pair<std::vector<init_trans_t>, std::vector<init_trans_t>>;

    protected:
    public:
        one_counter_automaton(alphabet &alphabet, displayable_type disp_type);

    protected:
        virtual void do_nothing() {}; // So the class is polymorphic

    public:
        const std::set<char> &get_alphabet_symbols();

        const alphabet &get_alphabet() const;

    protected:
        class alphabet &alphabet_;
        state_t init_state_ = 0;
        size_t states_n_ = 0;
        size_t max_level_ = 0;
        std::set<state_t> final_states_;
    };
}