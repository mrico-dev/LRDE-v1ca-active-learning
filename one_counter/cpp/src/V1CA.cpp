#include <queue>
#include <fstream>
#include <utility>
#include <boost/graph/graphviz.hpp>
#include <boost/graph/adjacency_list.hpp>

#include "V1CA.h"

namespace active_learning {

    void V1CA::link_by_name(std::string src_name, std::string dest_name, char symbol) {
        for (auto vi = boost::vertices(*graph); vi.first != vi.second; ++vi.first) {
            if ((*graph)[*vi.first].name == src_name) {
                for (auto vj = boost::vertices(*graph); vj.first != vj.second; ++vj.first) {
                    if ((*graph)[*vj.first].name == dest_name) {
                        auto new_edge_index = boost::add_edge(*vi.first, *vj.first, *graph);
                        if (!new_edge_index.second)
                            throw std::runtime_error("link_by_name(): Could not create new edge. Boost won't allow it.");
                        (*graph)[new_edge_index.first].symbol = symbol;
                        return;
                    }
                }
            }
        }

        throw std::invalid_argument(
                "link_by_name(): Could not find states with names '" + src_name + "' and '" + dest_name + "'.");
    }

    V1CA::graph_t &V1CA::get_mutable_graph() {
        return *graph;
    }

    bool V1CA::is_final(const V1CA_vertex &state) {
        return final_states_.contains(state.name);
    }

    bool V1CA::is_init(const V1CA_vertex &state) {
        return init_states_.contains(state.name);
    }

    std::optional<unsigned long> V1CA::get_next_index(vertex_descriptor_t state_index, char c) {

        for (auto edge_it = boost::edges(*graph); edge_it.first != edge_it.second; ++edge_it.first) {
            auto edge_desc = *edge_it.first;
            if ((*graph)[edge_desc].symbol == c and boost::source(edge_desc, *graph) == state_index)
                return boost::target(edge_desc, *graph);
        }

        return std::nullopt;
    }

    std::optional<unsigned long> V1CA::get_prev_index(vertex_descriptor_t state_index, char c) {

        for (auto edge_it = boost::edges(*graph); edge_it.first != edge_it.second; ++edge_it.first) {
            auto edge_desc = *edge_it.first;
            if ((*graph)[edge_desc].symbol == c and boost::target(edge_desc, *graph) == state_index)
                return boost::source(edge_desc, *graph);
        }

        return std::nullopt;
    }

    bool V1CA::is_state_isomorphic(V1CA &other, vertex_descriptor_t state1, vertex_descriptor_t state2,
            label_map_t &label_map) {

        auto queue1 = std::queue<unsigned long>();  // unsigned long is the vertex descriptor
        auto queue2 = std::queue<unsigned long>();
        auto visited = std::set<unsigned long>();

        // Checking if both states are final, or not, or initial, or not
        // Using xor to check if values are different
        if ((is_final((*graph)[state1]) ^ is_final((*graph)[state2])) or (is_init((*graph)[state1]) ^ is_init((*graph)[state2]))) {
            return false;
        }

        while (!queue1.empty()) {
            auto q1 = queue1.front();
            auto q2 = queue2.front();
            queue1.pop();
            queue2.pop();

            if (visited.contains(q1))
                continue;
            visited.insert(q1);

            // FIXME we could possibly factorize this code
            for (auto symbol : *alphabet_) {
                // Checking if same next
                auto next1 = get_next_index(q1, symbol.first);
                auto next2 = other.get_next_index(q2, symbol.first);

                if (next1.has_value() ^ next2.has_value())
                    return false;
                if (is_init((*graph)[*next1]) ^ other.is_init((*other.graph)[*next2]))
                    return false;
                if (is_final((*graph)[*next1]) ^ other.is_final((*other.graph)[*next2]))
                    return false;

                // Checking label
                if (!label_map.contains(*next1)) {
                    if (label_map.contains(*next2))
                        return false;

                    // Arbitrarily choosing label_map size as label (since we know it was not used before)
                    auto new_label = label_map.size();
                    label_map.insert({*next1, new_label});
                    label_map.insert({*next2, new_label});
                } else {
                    if (label_map[*next1] != label_map[*next2])
                        return false;
                }

                // Planning to visit next state
                if (!visited.contains(*next1)) {
                    queue1.push(*next1);
                    queue2.push(*next2);
                }

                // Now checking prev
                auto prev1 = get_prev_index(q1, symbol.first);
                auto prev2 = other.get_prev_index(q2, symbol.first);

                if (prev1.has_value() ^ prev1.has_value())
                    return false;
                if (is_init((*graph)[*prev1]) ^ other.is_init((*other.graph)[*prev2]))
                    return false;
                if (is_final((*graph)[*prev1]) ^ other.is_final((*other.graph)[*prev2]))
                    return false;

                // Checking label
                if (!label_map.contains(*prev1)) {
                    if (label_map.contains(*prev2))
                        return false;

                    auto new_label = label_map.size();
                    label_map.insert({*prev1, new_label});
                    label_map.insert({*prev2, new_label});
                } else {
                    if (label_map[*prev1] != label_map[*prev2])
                        return false;
                }

                // Planning to visit prev
                if (!visited.contains(*prev1)) {
                    queue1.push(*prev1);
                    queue2.push(*prev2);
                }
            }
        }

        return true;
    }

    bool V1CA::is_isomorphic_to_(V1CA &other, states_t &states1, states_t &states2, couples_t &res,
             label_map_t &label_map) {
        if (states1.empty() and states2.empty()) {
            return true;
        }

        for (auto state_index1 : states1) {
            for (auto state_index2 : states2) {

                auto label_map_cp = label_map_t(label_map);
                if (is_state_isomorphic(other, state_index1, state_index2, label_map_cp)) {
                    // Let's remove the two states from the list (by copy because recursion) and look for other couples
                    auto new_states1 = states_t(states1);
                    auto new_states2 = states_t(states2);
                    new_states1.erase(std::remove(new_states1.begin(), new_states1.end(), state_index1), new_states1.end());
                    new_states2.erase(std::remove(new_states2.begin(), new_states2.end(), state_index1), new_states2.end());

                    auto isomorphism_found = is_isomorphic_to_(other, new_states1, new_states2, res, label_map_cp);
                    if (isomorphism_found) {
                        res->emplace_back(state_index1, state_index2);
                    }

                    // We return either way, because this is the only way to fin isomorphism (no need to keep looking)
                    return isomorphism_found;
                }
            }
        }

        return false;
    }

    V1CA::couples_t V1CA::is_isomorphic_to(V1CA &other, unsigned int from_level1, unsigned from_level2) {

        states_t starting_states_1 = get_all_states_of_level(from_level1);
        states_t starting_states_2 = other.get_all_states_of_level(from_level2);

        if (starting_states_1.size() != starting_states_2.size()) {
            return std::nullopt;
        }

        couples_t res;
        label_map_t labels;
        if (is_isomorphic_to_(other, starting_states_1, starting_states_2, res, labels))
            return res;

        return std::nullopt;
    }

    V1CA::states_t V1CA::get_all_states_of_level(unsigned int level) {
        states_t res;
        for (auto vp = boost::vertices(*graph); vp.first != vp.second; ++vp.first) {
            auto state_index = *vp.first;
            if ((*graph)[state_index].cv == level) {
                res.push_back(state_index);
            }
        }

        return res;
    }

    void V1CA::display(const std::string &path) {
        std::string full_path = path + ".dot";
        std::ofstream file;
        file.open(full_path);
        boost::write_graphviz(file, *graph);
        // Hoping that you are on linux and have dot installed
        system(("dot -Tpng " + full_path + " > " + path + ".png").c_str());
    }

    V1CA::V1CA(std::vector<V1CA_vertex> &states, std::vector<V1CA_vertex> &initial_states,
               std::vector<V1CA_vertex> &final_states, const alphabet_t &al,
               std::vector<std::tuple<V1CA_vertex, V1CA_vertex, char>> &edges) {
        graph = std::make_shared<graph_t>();
        alphabet_ = std::make_shared<alphabet_t>(al);
        // Adding initial states
        for (auto &e : initial_states) {
            this->init_states_.insert(e.name);
        }
        // Adding final states
        for (auto &e : final_states) {
            this->final_states_.insert(e.name);
        }
        // Adding states to graph
        for (auto &v : states) {
            auto new_v = boost::add_vertex(*graph);
            (*graph)[new_v] = v;
        }
        // Adding edges to graph
        for (auto &e : edges) {
            link_by_name(std::get<0>(e).name, std::get<1>(e).name, std::get<2>(e));
        }
    }

    V1CA::V1CA() {
        graph = std::make_shared<graph_t>();
        alphabet_ = std::make_shared<alphabet_t>();
    }

    std::optional<int> V1CA::get_period_cv() {
        if (period_cv_ < 0)
            return std::nullopt;

        return period_cv_;
    }

    void V1CA::set_period_cv(int cv) {
        period_cv_ = cv;
    }

    void V1CA::remove_state(unsigned long vertex_descriptor) {
        boost::remove_vertex(vertex_descriptor, *graph);
    }

    void V1CA::set_periodic(bool periodic) {
        periodic_ = periodic;
    }

    void V1CA::set_colored(bool colored) {
        colored_ = colored;
    }

    V1CA::graph_color_t &V1CA::get_mutable_loop_in_no_cond_color() {
        return std::get<1>(colors_);
    }

    V1CA::graph_color_t &V1CA::get_mutable_init_edge_color() {
        return std::get<0>(colors_);
    }

    V1CA::graph_color_t &V1CA::get_mutable_loop_in_with_cond_color() {
        return std::get<2>(colors_);
    }

    V1CA::graph_color_t &V1CA::get_mutable_loop_out_color() {
        return std::get<3>(colors_);
    }

    V1CA_vertex::V1CA_vertex(std::string name, unsigned int cv) : name(std::move(name)), cv(cv) {}

    V1CA_edge::V1CA_edge(char symbol) : symbol(symbol) {}
}
