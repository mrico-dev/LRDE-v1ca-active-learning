#pragma once

#include "one_counter_automaton.h"
#include "displayable.h"
#include "alphabet.h"
#include "utils.h"

#include <string>
#include <optional>
#include <iostream>
#include <boost/graph/adjacency_list.hpp>

namespace active_learning {

    class V1CA : public one_counter_automaton {
    public:
        enum class transition_color {
            init,
            loop_out,           // Getting out of the loop
            loop_in_top,        // Getting from top to bottom of the loop
            loop_in_bottom      // Getting from bottom to top of the loop
        };

        struct transition_y {
            size_t state;
            transition_color color;

            bool operator==(const transition_y &other) const;
        };

        struct state_prop {
            size_t level;
            std::string name;
        };

        using transition_func_t = std::map<one_counter_automaton::transition_x, V1CA::transition_y>;
        using couples_t = std::vector<std::pair<state_t, state_t>>;

    private:
        std::optional<std::string> empty_(std::set<state_t> &visited, state_t curr, std::string curr_word) const;

        static void inter_with_(const V1CA &automaton1, const V1CA &automaton2,
                                std::set<V1CA::state_t> &visited1,
                                std::set<V1CA::state_t> &visited2,
                                V1CA::state_t curr1, V1CA::state_t curr2,
                                V1CA &res, V1CA::state_t res_curr) ;

        std::vector<std::pair<transition_x, transition_y>> get_out_trans(state_t) const;

        bool add_transition(const transition_x &x, const transition_y &y);

    public:
        explicit V1CA(const visibly_alphabet_t &alphabet);

        V1CA(const V1CA &copy) = default;

        V1CA(std::vector<state_prop> &state_names, state_t initial_state,
             std::vector<state_t> &final_states, visibly_alphabet_t &alphabet,
             std::vector<std::tuple<state_t, state_t, char>> &edges);

        V1CA inter_with(const V1CA &other) const;

        V1CA complement() const;

        std::optional<std::string> empty() const;

        std::optional<std::string> is_equivalent_to(V1CA &other) const;

        std::optional<std::string> is_subset_of(const V1CA &other) const;

        void display(const std::string &path) override;

        void link_and_color_edges(couples_t &couples);

        state_t add_state(const state_prop &prop);

        bool accepts(const std::string &word) const;

        friend class writer;

    private:
        // Specific to V1CA
        V1CA::transition_func_t transitions_;
        visibly_alphabet_t alphabet_;
        // Additional info
        std::map<state_t, state_prop> state_props_;
    };

}

// V1C2AL_V1CA_H
