#pragma once

#include "one_counter_automaton.h"
#include "utils.h"

#include <string>

namespace active_learning {

    class R1CA : public one_counter_automaton {
    public:
        struct transition_y {
            size_t state;
            int effect;
        };

        using couples_t = std::vector<std::pair<std::string, std::string>>;
        using transition_func_t = std::map<transition_x, transition_y>;
        using transition_t = std::pair<size_t, size_t>;

    private:
    public:
        R1CA(alphabet &alphabet, displayable_type disp_type, size_t init_state, size_t states_n, size_t maxLvl,
             std::set<size_t> final_states, transition_func_t transitions, basic_alphabet_t &alphabet1);

    public:
        bool evaluate(const std::string &word) const;

        int count(const std::string &word) const;

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
        // Specific to R1CA
        R1CA::transition_func_t transitions_;
        basic_alphabet_t &alphabet_;
    };
}
