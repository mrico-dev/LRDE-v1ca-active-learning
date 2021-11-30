#pragma once

#include "one_counter_automaton.h"
#include "utils.h"

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
        using transition_t = std::pair<size_t, size_t>;

    private:
    public:
        R1CA(alphabet &alphabet, displayable_type dispType, size_t initState, size_t statesN, size_t maxLvl,
             std::set<size_t> finalStates, transition_func_t transitions, basic_alphabet_t &alphabet1);

    public:
        bool evaluate(const std::string &word);

        int count(const std::string &word);

        [[nodiscard]]
        const basic_alphabet_t &get_alphabet() const;

        explicit R1CA(basic_alphabet_t &alphabet);

        R1CA(size_t states,
             size_t max_level,
             std::vector<size_t> final_states,
             const std::vector<std::tuple<size_t, size_t, char, int>> &transitions,
             std::map<utils::triple_comp<size_t, size_t, char>, utils::pair_comp<bool, size_t>> &colors,
             basic_alphabet &alphabet,
             size_t init_state = 0);

        void display(const std::string &path) override;

        [[nodiscard]]
        bool is_final(size_t state) const;

        static R1CA
        from_scratch(size_t initState, size_t statesN, size_t maxLvl,
                     const std::set<size_t> &finalStates, const transition_func_t &transitions,
                     basic_alphabet_t &alphabet1);

    private:
        size_t init_state_ = 0;
        size_t states_n_;
        size_t max_lvl_;
        std::set<size_t> final_states_;
        transition_func_t transitions_;
        basic_alphabet_t &alphabet_;
    };
}
