#pragma once

#include "one_counter_automaton.h"

#include <string>

namespace active_learning {

    struct transition_x {
        size_t state;
        size_t counter;
        char symbol;

        // Making it comparable so it can fit into a map
        bool operator==(const transition_x &other) const {
            return other.state == state
               and other.counter == counter
               and other.symbol == symbol;
        }

        bool operator<(const transition_x &other) const {

            if (state != other.state)
                return state > other.state;
            if (counter != other.counter)
                return counter > other.counter;

            return symbol > other.symbol;
        }
    };

    struct transition_y {
        size_t state;
        int effect;
    };

    class R1CA : public one_counter_automaton {
    public:
        using couples_t = std::vector<std::pair<std::string, std::string>>;
        using transition_func_t = std::map<transition_x, transition_y>;
        using state_t = size_t;
        using transition_t = std::pair<state_t, state_t>;

    public:
        bool evaluate(const std::string &word);
        int count(const std::string &word);

        [[nodiscard]] const basic_alphabet_t &get_alphabet() const;

        explicit R1CA(basic_alphabet_t &alphabet);

        R1CA(size_t states,
            size_t max_level,
            std::vector<size_t> final_states,
            const std::vector<std::tuple<size_t, size_t, char, int>> &transitions,
            std::map<std::tuple<size_t, size_t, char>, pair_comp<bool, size_t>> &colors,
            basic_alphabet &alphabet,
            size_t init_state=0);

        void display(const std::string& path) const override;

        bool is_final(size_t state);

    private:
        size_t init_state_ = 0;
        size_t states_n_;
        size_t max_lvl_;
        std::set<size_t> final_states_;
        transition_func_t transitions_;
        basic_alphabet_t &alphabet_;
    };
}
