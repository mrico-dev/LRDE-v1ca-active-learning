#include <queue>
#include <fstream>
#include <utility>
#include <boost/graph/graphviz.hpp>
#include <boost/graph/adjacency_list.hpp>

#include "V1CA_builder.h"
#include "V1CA.h"

namespace active_learning {

    /**
     * Creates a new transition between two states using the states name.
     * This function is not optimized as we need to go through the graph to find the state names.
     * Try to keep a vertex_descriptor if possible instead of using this function.
     * Raise an runtime_error if the edge cound not be created and
     * a invalid_argument error if one of the state does not exist.
     * @param src_name The name of the first state
     * @param dest_name The name of the second state
     * @param symbol The symbol associated with the new transition
     */
    void V1CA::link_by_name(std::string src_name, std::string dest_name, char symbol) {
        for (auto vi = boost::vertices(graph); vi.first != vi.second; ++vi.first) {
            if (graph[*vi.first].name == src_name) {
                for (auto vj = boost::vertices(graph); vj.first != vj.second; ++vj.first) {
                    if (graph[*vj.first].name == dest_name) {
                        auto new_edge_index = boost::add_edge(*vi.first, *vj.first, graph);
                        if (!new_edge_index.second)
                            throw std::runtime_error("link_by_name(): Could not create new edge. Boost won't allow it.");
                        graph[new_edge_index.first].symbol = symbol;
                        return;
                    }
                }
            }
        }

        throw std::invalid_argument(
                "link_by_name(): Could not find states with names '" + src_name + "' and '" + dest_name + "'.");
    }

    /**
     * Class getter
     * @return The mutable adjency_list (graph_t) object
     */
    V1CA::graph_t &V1CA::get_mutable_graph() {
        return graph;
    }

    /**
     * Tell whether a state is a final state using its properties
     * This works based on the fact that all states have different names
     * @param state The name and cv of the state
     * @return true if the state is final, false otherwise
     */
    bool V1CA::is_final(const V1CA_vertex &state) {
        return final_states_.contains(state.name);
    }

    /**
     * Tell whether a state is the initial state using its properties
     * This works based on the fact that all states have different names
     * @param state The name and cv of the state
     * @return true if the state is the initial state, false otherwise
     */
    bool V1CA::is_init(const V1CA_vertex &state) {
        return init_states_.contains(state.name);
    }

    /**
     * Returns the successor of a state using a specific symbol
     * @param state_index The state whose successor needs to be found
     * @param c The symbol used as the transition between the two states
     * @return The target state if it exists, std::nullopt otherwise
     */
    std::optional<unsigned long> V1CA::get_next_index(vertex_descriptor_t state_index, char c) {

        for (auto edge_it = boost::edges(graph); edge_it.first != edge_it.second; ++edge_it.first) {
            auto edge_desc = *edge_it.first;
            if (graph[edge_desc].symbol == c and boost::source(edge_desc, graph) == state_index)
                return boost::target(edge_desc, graph);
        }

        return std::nullopt;
    }

    /**
     * Returns the predecessor of a state using a specific symbol
     * @param state_index The state whose predecessor needs to be found
     * @param c The symbol used as the transition between the two states
     * @return The source state if it exists, std::nullopt otherwise
     */
    std::optional<unsigned long> V1CA::get_prev_index(vertex_descriptor_t state_index, char c) {

        for (auto edge_it = boost::edges(graph); edge_it.first != edge_it.second; ++edge_it.first) {
            auto edge_desc = *edge_it.first;
            if (graph[edge_desc].symbol == c and boost::target(edge_desc, graph) == state_index)
                return boost::source(edge_desc, graph);
        }

        return std::nullopt;
    }

    /**
     * Tell whether two states of two V1CA are isomorphic.
     * To do so, we check if they are locally isomorphic, and then if all their neighbors
     * are recursively isomorphic as well.
     * A label_map is kept to identify states that were already visited.
     * @param other The other V1CA that contains state2
     * @param state1 The first state of the V1CA
     * @param state2 The other state of the other V1CA
     * @param label_map The map to keep track of the visited states, and give them ids to make sure the isomophism
     * check is not locally biases
     * @return true if states are isomorphic, false otherwise.
     */
    bool V1CA::is_state_isomorphic(V1CA &other, vertex_descriptor_t state1, vertex_descriptor_t state2,
            label_map_t &label_map) {

        auto queue1 = std::queue<vertex_descriptor_t>();
        auto queue2 = std::queue<vertex_descriptor_t>();
        auto visited = std::set<vertex_descriptor_t>();

        // Checking if both states are final, or not, or initial, or not
        // Using xor to check if values are different
        if ((is_final(graph[state1]) ^ other.is_final(other.get_mutable_graph()[state2]))
            or (is_init(graph[state1]) ^ other.is_init(other.get_mutable_graph()[state2]))) {
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
                if (is_init(graph[*next1]) ^ other.is_init(other.graph[*next2]))
                    return false;
                if (is_final(graph[*next1]) ^ other.is_final(other.graph[*next2]))
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
                if (is_init(graph[*prev1]) ^ other.is_init(other.graph[*prev2]))
                    return false;
                if (is_final(graph[*prev1]) ^ other.is_final(other.graph[*prev2]))
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

    /**
     * Recursive implementation of is_isomorphic_to
     */
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
                    new_states2.erase(std::remove(new_states2.begin(), new_states2.end(), state_index2), new_states2.end());

                    auto isomorphism_found = is_isomorphic_to_(other, new_states1, new_states2, res, label_map_cp);
                    if (isomorphism_found) {
                        res.emplace_back(graph[state_index1].name,
                                other.get_mutable_graph()[state_index2].name);
                    }

                    // We return either way, because this is the only way to fin isomorphism (no need to keep looking)
                    return isomorphism_found;
                }
            }
        }

        return false;
    }

    /**
     * Tell whether two V1CA are isomorphic, and find a couples of equivalent states.
     * @param other The other V1CA
     * @param from_level1 The level where reference states are taken from in the first V1CA
     * @param from_level2 The level where reference states are taken from in the other V1CA
     * @return Couples of matching equivalent states if the V1CAs are equivalent, std::nullopt if not.
     */
    std::optional<V1CA::couples_t>
    V1CA::is_isomorphic_to(V1CA &other, unsigned int from_level1, unsigned from_level2) {

        states_t starting_states_1 = get_all_states_of_level(from_level1);
        states_t starting_states_2 = other.get_all_states_of_level(from_level2);

        if (starting_states_1.size() != starting_states_2.size()) {
            return std::nullopt;
        }

        couples_t res;
        label_map_t labels;
        if (is_isomorphic_to_(other, starting_states_1, starting_states_2, res, labels)) {
            return res;
        }

        return std::nullopt;
    }

    /**
     * Extract all states of a specific level (counter value) of a V1CA.
     * @param level The level of the states
     * @return A list of the states
     */
    V1CA::states_t V1CA::get_all_states_of_level(unsigned int level) {
        states_t res;
        for (auto vp = boost::vertices(graph); vp.first != vp.second; ++vp.first) {
            auto state_index = *vp.first;
            if (graph[state_index].cv == level) {
                res.emplace_back(state_index);
            }
        }

        return res;
    }

    /**
     * Export the V1CA as a png and .dot file.
     * This function requires to be on linux and have dot installed to get the .png file
     * @param path The path to the V1CA png and dot file, without the extenstion
     */
    void V1CA::display(const std::string &path) {

        // Writing dot file
        std::string full_path = path + ".dot";
        std::ofstream file;
        file.open(full_path);
        boost::write_graphviz(file, graph,
                vertex_writer<V1CA>(*this),
                edge_writer<V1CA, alphabet_t>(*this, *alphabet_));

        // Creating png file
        // Hoping that you are on linux and have dot installed
        system(("dot -Tpng " + full_path + " > " + path + ".png").c_str());
    }

    /**
     * Create a V1CA using information about the automaton
     * @param states A set of state names
     * @param initial_states The names of the initial states (should be one)
     * @param final_states The names of the final states
     * @param al The reference target language alphabet
     * @param edges The names of source and targets of edges
     */
    V1CA::V1CA(std::vector<V1CA_vertex> &states, std::vector<V1CA_vertex> &initial_states,
               std::vector<V1CA_vertex> &final_states, const alphabet_t &al,
               std::vector<std::tuple<V1CA_vertex, V1CA_vertex, char>> &edges) {
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
            auto new_v = boost::add_vertex(graph);
            graph[new_v] = v;
        }
        // Adding edges to graph
        for (auto &e : edges) {
            link_by_name(std::get<0>(e).name, std::get<1>(e).name, std::get<2>(e));
        }
    }

    /**
     * Creates an empty V1CA
     */
    V1CA::V1CA() {
        alphabet_ = std::make_shared<alphabet_t>();
    }

    /**
     * Class setter
     * @param cv The counter value of were the periodic part of the V1CA starts
     */
    void V1CA::set_period_cv(int cv) {
        period_cv_ = cv;
    }

    /**
     * Class setter
     * @param periodic Whether the V1CA is periodic
     */
    void V1CA::set_periodic(bool periodic) {
        periodic_ = periodic;
    }

    /**
     * Class setter
     * @param colored whether the V1CA is colored (if there is conditions on edges)
     */
    void V1CA::set_colored(bool colored) {
        colored_ = colored;
    }

    /**
     * Class getter
     * @return The set of edges that loop in back in the periodic part with no condition
     */
    V1CA::graph_color_t &V1CA::get_mutable_loop_in_no_cond_color() {
        return std::get<1>(colors_);
    }

    /**
     * Class getter
     * @return The set of edges that where initially already there before coloring
     */
    V1CA::graph_color_t &V1CA::get_mutable_init_edge_color() {
        return std::get<0>(colors_);
    }

    /**
     * Class getter
     * @return The set of edges that loop in back in the periodic part with a condition on the cv
     */
    V1CA::graph_color_t &V1CA::get_mutable_loop_in_with_cond_color() {
        return std::get<2>(colors_);
    }

    /**
     * Class getter
     * @return The set of edges that loop out of the periodic part with a condition on the cv
     */
    V1CA::graph_color_t &V1CA::get_mutable_loop_out_color() {
        return std::get<3>(colors_);
    }

    /**
     * Class getter
     * @return Whether the graph is colored, i.e if there is conditions on the edges
     */
    bool V1CA::colored() const {
        return colored_;
    }

    /**
     * Class getter
     * @return The counter value of where the periodic part of the V1CA starts, if a periodic structure was found;
     * -1 otherwise.
     */
    int V1CA::period_cv() const {
        return (periodic_ ? period_cv_ : -1);
    }

    /**
     * Get the edge between two states in the automaton
     * @param src The source state
     * @param dest The target state
     * @return The V1CA_edge object
     */
    V1CA_edge V1CA::get_edge(V1CA::vertex_descriptor_t src, V1CA::vertex_descriptor_t dest) {
        for (auto e_it = boost::edges(graph); e_it.first != e_it.second; ++e_it.first) {
            auto edge = *e_it.first;
            if (boost::source(edge, graph) == src and boost::target(edge, graph) == dest) {
                return graph[*e_it.first];
            }
        }

        throw std::invalid_argument("get_edge(): could not find edge with given src and dest.");
    }

    /**
     * Tell whether is two V1CA are equivalent, i.e if their language are the same.
     * @param other The other V1CA.
     * @return true if they are equivalent, false otherwise.
     */
    bool V1CA::is_equivalent_to(V1CA &other) {
        return is_subset_of(other) and other.is_subset_of(*this);
    }

    /**
     * Tell whether the language of this V1CA is a subset of the language of another V1CA.
     * @param other The other V1CA.
     * @return true if it is a subset, false otherwise.
     */
    bool V1CA::is_subset_of(V1CA& other) {
        auto other_complement = other.complement();
        return inter_with(other_complement).empty();
    }

    /**
     * Recusrive function for empty()
     */
    bool V1CA::empty_(std::set<V1CA::vertex_descriptor_t> &visited, V1CA::vertex_descriptor_t curr) {

        if (visited.contains(curr))
            return true;
        visited.insert(curr);

        if (is_final(graph[curr]))
            return false;

        for (auto edges_it = boost::out_edges(curr, graph); edges_it.first != edges_it.second; ++edges_it.first)
            if (!empty_(visited, boost::target(*edges_it.first, graph)))
                return false;

        return true;
    }

    /**
     * Tell whether the language of a V1CA is empty, i.e if the V1CA cannot accept any word.
     * @return true if empty, false otherwise.
     */
    bool V1CA::empty(){
        // Doing a recursive traversal and checking whether there is an accessible final state
        std::set<V1CA::vertex_descriptor_t> visited;
        // There need to be only one starting state though
        auto init_state = V1CA_builder::get_vertex_by_name(*this, *init_states_.begin());
        return empty_(visited, init_state);
    }

    /**
     * Recursive function for inter_with()
     */
    void inter_with_(V1CA &automaton1, V1CA &automaton2,
                    std::set<V1CA::vertex_descriptor_t> &visited1,
                    std::set<V1CA::vertex_descriptor_t> &visited2,
                    V1CA::vertex_descriptor_t curr1, V1CA::vertex_descriptor_t curr2,
                    V1CA &res, V1CA::vertex_descriptor_t res_curr) {
        if (visited1.contains(curr1) or visited2.contains(curr2))
            return;
        visited1.insert(curr1);
        visited2.insert(curr2);

        auto &graph1 = automaton1.get_mutable_graph();
        auto &graph2 = automaton2.get_mutable_graph();

        // TODO Handle final states (when they need to added or removed)
        for (auto edges_it1 = boost::out_edges(curr1, graph1); edges_it1.first != edges_it1.second; ++edges_it1.first) {

            // Checking if both state have an outer edge in common
            auto contains = false;
            auto symbol = '\0';
            V1CA::vertex_descriptor_t dest2 = 0;
            for (auto edges_it2 = boost::out_edges(curr1, graph1); edges_it2.first != edges_it2.second and not contains; ++edges_it2.first) {
                contains = graph1[*edges_it1.first].symbol == graph2[*edges_it2.first].symbol;
                symbol = graph1[*edges_it1.first].symbol;
                dest2 = boost::target(*edges_it2.first, graph2);
            }

            if (contains) {
                // Adding new vertex to result
                auto vertex = V1CA_vertex(graph1[curr1].name + graph2[curr2].name, graph1[curr1].cv);
                auto new_v = boost::add_vertex(vertex, res.get_mutable_graph());
                auto edge = V1CA_edge(symbol);
                boost::add_edge(res_curr, new_v, edge, res.get_mutable_graph());
                auto dest1 = boost::target(*edges_it1.first, graph1);

                inter_with_(automaton1, automaton2, visited1, visited2, dest1, dest2, res, new_v);
            }
        }

    }

    /**
     * Get the V1CA whose language is the intersection of the languages of two given V1CA
     * @param other The other V1CA
     * @return The intersection V1CA
     */
    V1CA V1CA::inter_with(V1CA &other) {
        auto &automaton1 = *this;
        auto &automaton2 = other;
        // Initializing result v1ca
        V1CA res;
        auto res_graph = res.get_mutable_graph();
        auto init_v = V1CA_vertex("", 0);
        auto new_v = boost::add_vertex(init_v, res_graph);

        // Initialising set of visited states
        std::set<V1CA::vertex_descriptor_t> visited;
        std::set<V1CA::vertex_descriptor_t> visited_other;

        // Getting init state
        auto init_state1 = V1CA_builder::get_vertex_by_name(automaton1, *automaton1.init_states_.begin());
        auto init_state2 = V1CA_builder::get_vertex_by_name(automaton2, *automaton2.init_states_.begin());

        inter_with_(automaton1, automaton2, visited, visited_other, init_state1, init_state2, res, new_v);

        return res;
    }

    /**
     * Get a V1CA whose language is the complementary language of this V1CA
     * @return The complement V1CA
     */
    V1CA V1CA::complement() {
        V1CA res = V1CA(*this);
        auto &g = res.get_mutable_graph();
        for (auto v_it = boost::vertices(g); v_it.first != v_it.second; ++v_it.first) {
            auto v = *v_it.first;
            if (not res.final_states_.contains(graph[v].name))
                res.final_states_.insert(graph[v].name);
            else
                res.final_states_.erase(graph[v].name);
        }

        return res;
    }

    /**
     * Constructor for a state attributes object
     * @param name The name of the vertex (should be unique)
     * @param cv The counter value of the state (should match the name)
     */
    V1CA_vertex::V1CA_vertex(std::string name, unsigned int cv) : name(std::move(name)), cv(cv) {}

    /**
     * Constructor for a transition attribute object
     * @param symbol the symbol of the transition
     */
    V1CA_edge::V1CA_edge(char symbol) : symbol(symbol) {}
}
