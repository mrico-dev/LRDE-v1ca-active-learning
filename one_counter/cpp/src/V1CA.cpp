#include <queue>
#include <fstream>
#include <utility>
#include <boost/graph/graphviz.hpp>
#include <boost/graph/adjacency_list.hpp>

#include "V1CA.h"
#include "dot_writers.h"

namespace active_learning {

    /**
     * Export the V1CA as a png and .dot file.
     * This function requires to be on linux and have dot installed to get the .png file
     * @param path The path to the V1CA png and dot file, without the extension
     */
    void V1CA::display(const std::string &path) {

        // Writing dot file
        std::string full_path = path + ".dot";
        std::ofstream file;
        file.open(full_path);
        writer::write_v1ca(file, *this);
        file.close();

        // Creating png file
        // Hoping that you are on linux and have dot installed
        system(("dot -Tpng " + full_path + " > " + path + ".png").c_str());
    }

    /**
     * Tell whether is two V1CA are equivalent, i.e if their language are the same.
     * @param other The other V1CA.
     * @return true if they are equivalent, false otherwise.
     */
    std::optional<std::string> V1CA::is_equivalent_to(V1CA &other) const {
        auto res_left = is_subset_of(other);
        if (res_left)
            return res_left;
        auto res_right = other.is_subset_of(*this);
        if (res_right)
            return res_right;

        return std::nullopt;
    }

    /**
     * Tell whether the language of this V1CA is a subset of the language of another V1CA.
     * @param other The other V1CA.
     * @return std::nullopt if it is a subset,
     *  a counter-example string of a word contained in this V1CA but not in the other otherwise.
     */
    std::optional<std::string> V1CA::is_subset_of(const V1CA &other) const {
        auto other_complement = other.complement();
        return inter_with(other_complement).empty();
    }

    /**
     * Recursive function for empty()
     */
    std::optional<std::string> V1CA::empty_(std::set<state_t> &visited, state_t curr, std::string curr_word) const {

        if (visited.contains(curr))
            return std::nullopt;

        visited.insert(curr);

        if (final_states_.contains(curr))
            return curr_word;

        for (const auto &out_trans : get_out_trans(curr)) {
            auto out_state = out_trans.second.state;
            auto empty = empty_(visited, out_state, curr_word + out_trans.first.symbol);
            if (empty)
                return empty;
        }

        return std::nullopt;
    }

    /**
     * Tell whether the language of a V1CA is empty, i.e if the V1CA cannot accept any word.
     * @return std::nullopt if the language is empty,
     *         a string containing an supposedly accepted word if the language is not empty.
     */
    std::optional<std::string> V1CA::empty() const {
        // Doing a recursive traversal and checking whether there is an accessible final state
        std::set<state_t> visited;
        // There need to be only one starting state though
        return empty_(visited, init_state_, "");
    }

    /**
     * Recursive function for inter_with()
     */
    void V1CA::inter_with_(const V1CA &automaton1, const V1CA &automaton2,
                     std::set<V1CA::state_t> &visited1,
                     std::set<V1CA::state_t> &visited2,
                     std::map<V1CA::state_t, V1CA::state_t> &auto1_state_to_res_state,
                     V1CA::state_t curr1, V1CA::state_t curr2,
                     V1CA &res, V1CA::state_t res_curr) {

        if (visited1.contains(curr1) or visited2.contains(curr2))
            return;

        visited1.insert(curr1);
        visited2.insert(curr2);

        auto handled_dest = std::set<state_t>();
        for (auto out_trans1 : automaton1.get_out_trans(curr1)) {

            // Picking a state to add or not
            auto dest_to_handle = out_trans1.second.state;
            if (handled_dest.contains(dest_to_handle))
                continue;
            handled_dest.insert(dest_to_handle);

            // Getting all transitions from current state to dest state
            auto transitions_to_dest = std::set<transition_x>();
            for (auto out_trans_1_other : automaton1.get_out_trans(curr1)) {
                if (out_trans1.second.state == dest_to_handle)
                    transitions_to_dest.insert(out_trans_1_other.first);
            }

            // Checking if transitions exist in automaton2
            auto transitions2_to_dest = std::set<transition_x>();
            for (auto &trans1 : transitions_to_dest) {
                for (auto out_trans_2 : automaton2.get_out_trans(curr2)) {
                    if (trans1.symbol == out_trans_2.first.symbol and trans1.counter == out_trans_2.first.counter)
                        transitions2_to_dest.insert(out_trans_2.first);
                }
            }

            // We check that there is the same number of equal transitions
            // We also check that all transition from trans2 go to same destination
            // (we could optimize by saying that if 2 states are not the same, both can't be added to res)
            auto common_dest = transitions2_to_dest.size() == transitions_to_dest.size();
            auto dest2 = automaton2.transitions_.at(*transitions2_to_dest.begin()).state;
            for (auto any_dest_2 : transitions2_to_dest)
                common_dest &= automaton2.transitions_.at(any_dest_2).state == dest2;

            // Checking that both were never visited
            common_dest &= (visited1.contains(dest_to_handle) == visited2.contains(dest2));

            // If the destination is not verified by automaton2, we stop trying
            if (not common_dest)
                break;

            // Adding new state to result if state was never visited before
            auto res_dest = 424242u;
            if (not visited1.contains(dest_to_handle)) {
                res_dest = res.add_state({automaton1.state_props_.at(curr1).level,
                                            automaton1.state_props_.at(curr1).name + "~" +
                                            automaton2.state_props_.at(curr2).name});
                auto1_state_to_res_state.insert({dest_to_handle, res_dest});
            } else {
                res_dest = auto1_state_to_res_state.at(dest_to_handle);
            }

            // Adding all transitions from current state to this state
            for (auto &transition : transitions_to_dest) {
                auto edge_ok = res.add_transition({res_curr, transition.counter, transition.symbol}, {res_dest, automaton1.transitions_.at(transition).color});
                if (not edge_ok)
                    throw std::runtime_error("Could not add transition to result automaton while processing intersection.");
            }

            auto dest1 = out_trans1.second.state;
            inter_with_(automaton1, automaton2, visited1, visited2, auto1_state_to_res_state, dest1, dest2, res, res_dest);
        }
    }

    /**
     * Get the V1CA whose language is the intersection of the languages of two given V1CA
     * @param other The other V1CA
     * @return The intersection V1CA
     */
    V1CA V1CA::inter_with(const V1CA &other) const {
        if (not (alphabet_ == other.alphabet_))
            throw std::invalid_argument("Intersection of two V1CA must be performed on V1CA with the same dictionaries.");

        // Making copies because we might need to modify these
        V1CA automaton1 = V1CA(*this);
        V1CA automaton2 = V1CA(other);

        // Making both automata the same level
        if (automaton1.max_level_ < automaton2.max_level_) {
             automaton1.increase_max_level(automaton2.max_level_ - automaton1.max_level_);
        } else if (automaton2.max_level_ < automaton1.max_level_) {
            automaton2.increase_max_level(automaton1.max_level_ - automaton2.max_level_);
        }

        std::set<state_t> visited1;
        std::set<state_t> visited2;

        auto init_state1 = automaton1.init_state_;
        auto init_state2 = automaton2.init_state_;

        auto res = V1CA(alphabet_);
        auto init = res.add_state({0, automaton1.state_props_.at(init_state1).name});

        auto auto1_st_to_res_st = std::map<state_t, state_t>();
        auto1_st_to_res_st.insert({init_state1, 0});

        inter_with_(automaton1, automaton2, visited1, visited2, auto1_st_to_res_st, init_state1, init_state2, res, init);

        return res;
    }


    /**
     * Get a V1CA whose language is the complementary language of this V1CA
     * The given V1CA needs to be complete (have a transition for all possible character)
     * @return The complement V1CA
     */
    V1CA V1CA::complement() const {
        V1CA res = V1CA(*this);

        for (auto st=0u; st < res.states_n_; ++st) {
            if (res.state_props_[st].level == 0) {
                if (res.final_states_.contains(st)) {
                    res.final_states_.erase(st);
                } else {
                    res.final_states_.insert(st);
                }
            }
        }

        return res;
    }


    V1CA::V1CA(const visibly_alphabet_t &alphabet) : one_counter_automaton(static_cast<class alphabet&>((active_learning::alphabet &) alphabet), displayable_type::V1CA) {}

    V1CA::V1CA(std::vector<state_prop> &state_props, state_t initial_state, std::vector<state_t> &final_states,
               visibly_alphabet_t &alphabet, std::vector<std::tuple<state_t, state_t, char>> &edges) :
            one_counter_automaton(alphabet, displayable_type::V1CA), alphabet_(alphabet) {

        init_state_ = initial_state;
        states_n_ = state_props.size();
        max_level_ = 0;

        for (auto i = 0u; i < state_props.size(); ++i) {
            state_props_.insert({i, state_props[i]});
            // FIXME Could be optimized by passing cv as arguments
            auto state_cv = static_cast<size_t>(alphabet.get_cv(state_props[i].name));
            if (state_cv > max_level_)
                max_level_ = state_cv;
        }

        for (const auto &final : final_states)
            final_states_.insert(final);

        std::cout << "\nTransitioning:\n";
        for (auto &transition : edges) {
            for (auto cv = 0u; cv <= max_level_; ++cv) {
                auto trans_x = transition_x{std::get<0>(transition), cv, std::get<2>(transition)};
                auto trans_y = transition_y{std::get<1>(transition), transition_color::init};

                transitions_.insert({trans_x, trans_y});
            }
        }
    }

    bool V1CA::transition_y::operator==(const transition_y &other) const {
        return state == other.state and color == other.color;
    }

    V1CA::state_t V1CA::add_state(const state_prop &prop) {
        auto new_state = states_n_++;
        state_props_[new_state] = prop;

        return new_state;
    }

    bool V1CA::add_transition(const transition_x &x, const transition_y &y) {
        if (x.state >= states_n_ or y.state >= states_n_)
            return false;

        transitions_.insert({x, y});
        return true;
    }

    std::vector<std::pair<V1CA::transition_x, V1CA::transition_y>> V1CA::get_out_trans(state_t from) const {
        std::vector<std::pair<transition_x, transition_y>> res;
        for (auto trans : transitions_) {
            if (trans.first.state == from)
                res.emplace_back(trans);
        }

        return res;
    }

    void V1CA::link_and_color_edges(couples_t &couples) {

        // couple.first and couple.second necessarily have different levels
        for (const auto &couple: couples) {
            for (auto &trans : transitions_) {

                // Only considering max_value transitions
                if (trans.first.counter < max_level_)
                    continue;

                // Linking loop_in_bottom and loop_out
                if (trans.first.state == couple.second
                    and alphabet_.get_cv(trans.first.symbol) == -1) {

                    transition_x new_edge_x = {couple.first, trans.first.counter, trans.first.symbol};
                    transition_y new_edge_y = {trans.second.state, transition_color::loop_in_bottom};
                    // deleting the possible former transition with same transition_x before adding the new one,
                    // as the former one does not make anymore sense with this specific cv
                    if (transitions_.contains(new_edge_x))
                        transitions_.erase(new_edge_x);
                    transitions_.insert({new_edge_x, new_edge_y});

                    // Coloring of loop_out (is not mandatory)
                    for (auto &other_trans : transitions_) {
                        if (other_trans.first.state == new_edge_x.state
                            and other_trans.first.symbol == new_edge_x.symbol
                            and other_trans.first.counter < new_edge_x.counter) {
                            transitions_[other_trans.first].color = transition_color::loop_out;
                        }
                    }
                // Linking loop in top
                } else if (trans.first.state == couple.first
                    and alphabet_.get_cv(trans.first.symbol) == 1) {

                    transition_x new_edge_x = {couple.second, trans.first.counter, trans.first.symbol};
                    transition_y new_edge_y = {trans.second.state, transition_color::loop_in_top};

                    transitions_.insert({new_edge_x, new_edge_y});
                }
            }
        }
    }

    bool V1CA::accepts(const std::string &word) const {
        auto curr_state = init_state_;
        auto cv = 0u;

        for (auto &c : word) {
            auto trans_cv = (cv > max_level_) ? max_level_ : cv;
            auto x = transition_x{curr_state, trans_cv, c};
            if (not transitions_.contains(x))
                return false;

            auto y = transitions_.at(x);
            curr_state = y.state;
            cv += alphabet_.get_cv(c);
        }

        return final_states_.contains(curr_state) and not cv;
    }

    void V1CA::increase_max_level(size_t n) {
        auto new_max_level = max_level_ + n;
        auto visited = std::set<state_t>();
        auto stack = std::stack<state_t>();
        stack.push(init_state_);

        while (not stack.empty()) {
            state_t curr_st = stack.top();
            stack.pop();

            if (visited.contains(curr_st))
                continue;
            visited.insert(curr_st);

            auto out_trans = get_out_trans(curr_st);

            for (auto trans: out_trans) {
                (void) trans;
            }
        }

        max_level_ = new_max_level;
    }

    void V1CA::increase_max_level() {

        auto dup_states = std::map<state_t, state_t>();

        // Copying max_level states
        for (state_t st = 0u; st < states_n_; ++st) {
            auto prop = state_props_.at(st);
            if (prop.level == max_level_) {
                auto new_state = add_state({max_level_ + 1, prop.name + "_bis"});
                dup_states.insert({st, new_state});
            }
        }

        // Handling transition
        for (auto transition : transitions_) {
            auto x = transition.first;
            auto y = transition.second;

            // Copying edges at max_level to max_level + 1
            if (state_props_.at(x.state).level == max_level_ and state_props_.at(y.state).level == max_level_) {
                auto new_x = transition_x{dup_states.at(x.state), x.counter + 1, x.symbol};
                auto new_y = transition_y{dup_states.at(y.state), y.color};
                transitions_.insert({new_x, new_y});
            }

            // Handling edges between max_level and max_level + 1
            if (state_props_.at(x.state).level == max_level_) {
                if (alphabet_.get_cv(x.symbol) == 1) {
                    auto new_x = transition_x{x.state, x.counter, x.symbol};
                    auto new_y = transition_y{dup_states.at(y.state), y.color};
                    transitions_.insert({new_x, new_y});
                } else if (alphabet_.get_cv(x.symbol) == -1) {
                    auto new_x = transition_x{x.state, x.counter, x.symbol};
                    auto new_y = transition_y{dup_states.at(y.state), y.color};
                    transitions_.insert({new_x, new_y});
                }
            }
        }

        ++max_level_;
    }
}
