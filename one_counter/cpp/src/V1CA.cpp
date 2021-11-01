#include <queue>
#include <fstream>
#include <utility>
#include <boost/graph/graphviz.hpp>
#include <boost/graph/adjacency_list.hpp>

#include "V1CA.h"
#include "dot_writers.h"

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
                            throw std::runtime_error(
                                    "link_by_name(): Could not create new edge. Boost won't allow it.");
                        graph[new_edge_index.first].symbol = symbol;
                        return;
                    }
                }
            }
        }

        throw std::invalid_argument(
                "link_by_name(): Could not find states with names '" + src_name + "' and '" + dest_name + "'.");
    }

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
        return init_state_ == state.name;
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
     * @param path The path to the V1CA png and dot file, without the extension
     */
    void V1CA::display(const std::string &path) const {

        // Writing dot file
        std::string full_path = path + ".dot";
        std::ofstream file;
        file.open(full_path);
        boost::write_graphviz(file, graph,
                              vertex_writer((displayable &) *this),
                              edge_writer((displayable &) *this, alphabet_));

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
    V1CA::V1CA(std::vector<V1CA_vertex> &states, std::string &initial_state,
               std::vector<std::string> &final_states, visibly_alphabet_t &al,
               std::vector<std::tuple<std::string, std::string, char>> &edges) :
            one_counter_automaton(al, displayable_type::V1CA) {
        // Adding initial states
        this->init_state_ = initial_state;
        // Adding final states
        for (auto &e : final_states) {
            this->final_states_.insert(e);
        }
        // Adding states to graph
        for (auto &v : states) {
            auto new_v = boost::add_vertex(graph);
            graph[new_v] = v;
        }
        // Adding edges to graph
        for (auto &e : edges) {
            link_by_name(std::get<0>(e), std::get<1>(e), std::get<2>(e));
        }
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
    bool V1CA::is_subset_of(V1CA &other) {
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
    bool V1CA::empty() {
        // Doing a recursive traversal and checking whether there is an accessible final state
        std::set<V1CA::vertex_descriptor_t> visited;
        // There need to be only one starting state though
        auto init_state = get_vertex_by_name(*this, init_state_);
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
            for (auto edges_it2 = boost::out_edges(curr1, graph1);
                 edges_it2.first != edges_it2.second and not contains; ++edges_it2.first) {
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
        auto res = V1CA(dynamic_cast<visibly_alphabet_t&>(alphabet_));
        auto &res_graph = res.get_mutable_graph();
        auto init_v = V1CA_vertex("", 0);
        auto new_v = boost::add_vertex(init_v, res_graph);

        // Initialising set of visited states
        std::set<V1CA::vertex_descriptor_t> visited;
        std::set<V1CA::vertex_descriptor_t> visited_other;

        // Getting init state
        auto init_state1 = V1CA::get_vertex_by_name(automaton1, automaton1.init_state_);
        auto init_state2 = V1CA::get_vertex_by_name(automaton2, automaton2.init_state_);

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

    /**
     * Get the vertex_descriptor of a state using its name
     * @param automaton The V1CA
     * @param name The name of the state
     * @return The state as a vertex_descriptor
     */
    V1CA::vertex_descriptor_t V1CA::get_vertex_by_name(V1CA &automaton, const std::string &name) {
        auto &graph = automaton.get_mutable_graph();
        auto v_its = boost::vertices(graph);
        for (; v_its.first != v_its.second; ++v_its.first) {
            if (graph[*v_its.first].name == name)
                return *v_its.first;
        }

        throw std::invalid_argument("get_vertex_by_name(): Could not find vertex");

    }

    /**
     * Color the V1CA by putting conditions on specific edges
     * @param automaton The given V1CA
     * @param new_edges Edges that were added when looping on the periodic structure
     */
    void V1CA::color_edges(V1CA::looped_edges_t &new_edges) {
        // Coloring init (others)
        for (auto edge_it = boost::edges(graph); edge_it.first != edge_it.second; ++edge_it.first) {
            get_mutable_init_edge_color().insert(make_pair_comp((*edge_it.first).m_source, (*edge_it.first).m_target));
        }

        // Coloring loop in no cond edges
        for (auto &edge : new_edges.first) {
            get_mutable_loop_in_no_cond_color().insert(make_pair_comp(edge.m_source, edge.m_target));
            get_mutable_init_edge_color().erase(make_pair_comp(edge.m_source, edge.m_target));
        }

        // Coloring loop in with cond edges
        for (auto &edge : new_edges.second) {
            get_mutable_loop_in_with_cond_color().insert(make_pair_comp(edge.m_source, edge.m_target));
            get_mutable_init_edge_color().erase(make_pair_comp(edge.m_source, edge.m_target));
        }

        // Coloring loop out edges
        for (auto &loop_in_edge_with_cond : get_mutable_loop_in_with_cond_color()) {
            for (auto &other_edge_from_same_source : get_transitions_from_state(loop_in_edge_with_cond.first)) {
                if ((loop_in_edge_with_cond.first != other_edge_from_same_source.m_target
                     or loop_in_edge_with_cond.second != other_edge_from_same_source.m_source)
                    and get_edge(loop_in_edge_with_cond.first, loop_in_edge_with_cond.second).symbol ==
                        graph[other_edge_from_same_source].symbol) {
                    get_mutable_loop_out_color().insert(
                            make_pair_comp(other_edge_from_same_source.m_source, other_edge_from_same_source.m_target));
                    get_mutable_init_edge_color().erase(
                            make_pair_comp(other_edge_from_same_source.m_source, other_edge_from_same_source.m_target));
                }
            }
        }

        set_colored(true);
    }

    std::vector<V1CA::edge_descriptor_t> V1CA::get_transitions_from_state(V1CA::vertex_descriptor_t from) {
        std::vector<edge_descriptor_t> res;
        for (auto vp = boost::out_edges(from, graph); vp.first != vp.second; ++vp.first) {
            res.emplace_back(*vp.first);
        }

        return res;
    }

    V1CA::V1CA(visibly_alphabet_t &alphabet) : one_counter_automaton(alphabet, displayable_type::V1CA) {}
}