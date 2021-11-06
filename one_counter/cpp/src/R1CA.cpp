#include "R1CA.h"
#include "dot_writers.h"

#include <utility>
#include <fstream>
#include <boost/graph/graphviz.hpp>

namespace active_learning {

    bool R1CA::evaluate(const std::string &word) {
        // Starting at initial state with counter = 0
        auto curr_state = init_state_;
        int counter = 0;

        // Getting from states to states using symbol of the word
        for (char c : word) {
            if (not transitions_.contains({curr_state, static_cast<size_t>(counter), c}))
                return false;

            transition_y trans_y = transitions_[{curr_state, static_cast<size_t>(counter), c}];
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
            if (not transitions_.contains({curr_state, static_cast<size_t>(counter), c}))
                return -1;

            transition_y trans_y = transitions_[{curr_state, static_cast<size_t>(counter), c}];
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

    R1CA::R1CA(basic_alphabet &alphabet) : one_counter_automaton(alphabet, displayable_type::R1CA), alphabet_(alphabet) {}

    void R1CA::display(const std::string &path) const {
        // Writing dot file
        std::string full_path = path + ".dot";
        std::ofstream file;
        file.open(full_path);
        boost::write_graphviz(file, graph_,
                              vertex_writer((displayable &) *this),
                              edge_writer((displayable &) *this, alphabet_));

        // Creating png file
        // Hoping that you are on linux and have dot installed
        system(("dot -Tpng " + full_path + " > " + path + ".png").c_str());
    }

    bool R1CA::is_final(size_t v) {
        return final_states_.contains(v);
    }

    R1CA::R1CA(size_t states,
               size_t max_level,
               std::vector<size_t> final_states,
               const std::vector<std::tuple<size_t, size_t, char, int>>& transitions,
               std::map<std::tuple<size_t, size_t, char>, pair_comp<bool, size_t>> &colors,
               basic_alphabet &alphabet,
               size_t init_state) : one_counter_automaton(alphabet, displayable_type::R1CA),
                                    init_state_(init_state),
                                    states_n_(states),
                                    max_lvl_(max_level),
                                    final_states_(std::move(final_states)),
                                    alphabet_(alphabet) {
        for (auto &trans : transitions) {
            size_t src;
            size_t dest;
            char symbol;
            int effect;
            std::tie(src, dest, symbol, effect) = trans;

            if (colors.contains({src, dest, symbol})) {
                // Conditional transition
                auto edge = std::make_tuple(src, dest, symbol);
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
}