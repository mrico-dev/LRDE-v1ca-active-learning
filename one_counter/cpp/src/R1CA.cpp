#include "R1CA.h"

#include <fstream>
#include <utility>
#include <V1CA.h>

namespace active_learning {

    bool R1CA::evaluate(const std::string &word) {
        // Starting at initial state with counter = 0
        auto curr_state = init_state_;
        int counter = 0;

        // Getting from states to states using symbol of the word
        for (char c : word) {
            // FIXME max_lvl_ or max_level_ + 1 (and underneath as well)
            auto counter_clip = (static_cast<size_t>(counter) > max_lvl_) ? max_lvl_ : counter;

            if (not transitions_.contains({curr_state, counter_clip, c}))
                return false;

            transition_y trans_y = transitions_[{curr_state, counter_clip, c}];
            curr_state = trans_y.state;
            counter += trans_y.effect;

            if (counter < 0)
                return false;
        }

        return is_final(curr_state) and not counter;
    }

    int R1CA::count(const std::string &word) {
        // Starting at initial state with counter = 0
        auto curr_state = init_state_;
        int counter = 0;

        // Getting from states to states using symbol of the word
        for (char c : word) {
            auto counter_clip = (static_cast<size_t>(counter) > max_lvl_) ? max_lvl_ : counter;

            if (not transitions_.contains({curr_state, counter_clip, c}))
                return -1;

            transition_y trans_y = transitions_[{curr_state, counter_clip, c}];
            curr_state = trans_y.state;
            counter += trans_y.effect;

            if (counter < 0)
                return -1;
        }

        return counter;
    }

    const basic_alphabet &R1CA::get_alphabet() const {
        return alphabet_;
    }

    R1CA::R1CA(basic_alphabet &alphabet) : one_counter_automaton(alphabet, displayable_type::R1CA),
                                           alphabet_(alphabet) {
        states_n_ = 0;
        max_lvl_ = UINT64_MAX;
    }

    bool R1CA::is_final(size_t v) const {
        return final_states_.contains(v);
    }

    R1CA::R1CA(size_t states,
               size_t max_level,
               std::vector<size_t> final_states,
               const std::vector<std::tuple<size_t, size_t, char, int>> &transitions,
               std::map<utils::triple_comp<size_t, size_t, char>, utils::pair_comp<bool, size_t>> &colors,
               basic_alphabet &alphabet,
               size_t init_state) : one_counter_automaton(alphabet, displayable_type::R1CA),
                                    init_state_(init_state),
                                    states_n_(states),
                                    max_lvl_(max_level),
                                    alphabet_(alphabet) {
        for (auto fs: final_states)
            final_states_.insert(fs);

        for (auto &trans : transitions) {
            size_t src;
            size_t dest;
            char symbol;
            int effect;
            std::tie(src, dest, symbol, effect) = trans;

            if (colors.contains({src, dest, symbol})) {
                // Conditional transition
                auto edge = utils::make_triple_comp(src, dest, symbol);
                const auto color = colors[edge];
                const auto inf = color.first;
                const auto level = color.second;

                if (inf) {
                    for (auto i = 0u; i <= level; ++i)
                        transitions_[{src, i, symbol}] = {dest, effect};
                } else {
                    for (auto i = level - 1; i > level; --i)
                        transitions_[{src, i, symbol}] = {dest, effect};
                }
            } else {
                // Non-conditional transition
                for (auto i = 0u; i < max_level; ++i)
                    transitions_[{src, i, symbol}] = {dest, effect};
            }
        }
    }

    void R1CA::display(const std::string &path) {
        // Writing dot file
        std::string full_path = path + ".dot";
        std::ofstream file;
        file.open(full_path);

        // Writing states
        file << "digraph G {\n";
        for (auto i = 0u; i < states_n_; ++i) {
            file << i << "[ shape=\""
                 << ((is_final(i)) ? "doublecircle" : "circle")
                 << "\"];\n";
        }
        for (auto &trans : transitions_) {
            file << trans.first.state
                 << "->"
                 << trans.second.state
                 << " [label=\""
                 << trans.first.symbol
                 << "\"];\n";
        }
        file.close();

        // Creating png file
        // Hoping that you are on linux and have dot installed
        system(("dot -Tpng " + full_path + " > " + path + ".png").c_str());
    }

    R1CA R1CA::from_scratch(size_t init_state, size_t states_n, size_t max_lvl,
                           const std::set<size_t> &final_states, const transition_func_t &transitions,
                           basic_alphabet_t &alphabet) {
        return R1CA(alphabet, displayable_type::R1CA, init_state, states_n, max_lvl, final_states, transitions,
                    alphabet);
    }

    R1CA::R1CA(alphabet &alphabet, displayable_type dispType, size_t initState, size_t statesN, size_t maxLvl,
               std::set<size_t> finalStates, R1CA::transition_func_t transitions,
               basic_alphabet_t &alphabet1) : one_counter_automaton(alphabet, dispType), init_state_(initState),
                                              states_n_(statesN), max_lvl_(maxLvl), final_states_(std::move(finalStates)),
                                              transitions_(std::move(transitions)), alphabet_(alphabet1) {}
}