#include "behaviour_graph.h"
#include "dataframe.h"

#include <boost/graph/graphviz.hpp>
#include <fstream>
#include <dot_writers.h>
#include <queue>

namespace active_learning {

    // Constructors
    bg_vertex_attr::bg_vertex_attr(const std::string &name, int level) : name(name), level(level) {}

    bg_vertex_attr::bg_vertex_attr() = default;

    bg_edge_attr::bg_edge_attr(char symbol, int effect) : symbol(symbol), effect(effect) {}

    bg_edge_attr::bg_edge_attr(char symbol) : symbol(symbol) {}

    bg_edge_attr::bg_edge_attr() = default;

    behaviour_graph::behaviour_graph() : displayable(displayable_type::behaviour_graph) {}

    behaviour_graph::behaviour_graph(const vertexes_t &states,
                                     const edges_t &edges,
                                     const std::string &init_state,
                                     std::set<std::string> final_states) : displayable(
            displayable_type::behaviour_graph) {
        // Adding initial state
        init_state_ = init_state;
        // Adding final states
        for (auto &e : final_states) {
            this->final_states_.insert(e);
        }

        std::map<std::string, vertex_descriptor_t> name_to_desc;

        // Adding vertexes to graph
        for (auto &v : states) {
            auto new_v = boost::add_vertex(bg_vertex_attr(v.first, v.second), graph_);
            name_to_desc[v.first] = new_v;

            if (static_cast<size_t>(v.second) > max_level_)
                max_level_ = v.second;
        }

        // Adding edges to graph
        for (auto &e : edges) {
            vertex_descriptor_t src = name_to_desc[std::get<0>(e)];
            vertex_descriptor_t dest = name_to_desc[std::get<3>(e)];
            char symbol = std::get<1>(e);
            int effect = std::get<2>(e);
            auto new_edge = boost::add_edge(src, dest, bg_edge_attr(symbol, effect), graph_);
            if (!new_edge.second)
                throw std::runtime_error("Could not add edge while creating R1CA, boost won't allow it");
        }
    }

    // Others
    behaviour_graph::vertex_descriptor_t behaviour_graph::find_vertex_by_name(const std::string &name) {
        for (auto vi = boost::vertices(graph_); vi.first != vi.second; ++vi.first) {
            if (graph_[*vi.first].name == name) {
                return *vi.first;
            }
        }

        throw std::invalid_argument("Could not find state with name '" + name + "'.");
    }

    behaviour_graph::graph_t &behaviour_graph::get_mutable_graph() {
        return graph_;
    }

    void behaviour_graph::display(const std::string &path) {
        // Writing dot file
        std::string full_path = path + ".dot";
        std::ofstream file;
        file.open(full_path);
        auto dummy_alphabet = basic_alphabet();
        boost::write_graphviz(file, graph_,
                              vertex_writer((displayable &) *this),
                              edge_writer((displayable &) *this));

        // Creating png file
        // Hoping that you are on linux and have dot installed
        system(("dot -Tpng " + full_path + " > " + path + ".png").c_str());
    }

    bool behaviour_graph::is_final(const std::string &v_name) {
        return final_states_.contains(v_name);
    }

    bool behaviour_graph::is_init(const std::string &v_name) {
        return v_name == init_state_;
    }

    /**
     * Get the edges of a behaviour graph using a RST.
     * @param no_dup_rst The source RST with no duplicated rows
     * @param alphabet The reference target language alphabet
     * @param states A list of the states of the result behaviour graph
     * @param teacher A teacher that may be used for membership queries
     * @return A set of edges deduced from the RST
     */
    behaviour_graph::edges_t
    behaviour_graph::get_edges_from_rst(RST &no_dup_rst, word_counter &wc, vertexes_t &states, teacher &teacher,
                                        alphabet &alphabet) {
        edges_t res;
        for (auto &st : states) {
            auto &src = st.first;
            for (auto &c : alphabet.symbols()) {
                auto dest_word = src + c;
                int cv = wc.get_cv(dest_word);

                if (cv >= static_cast<int>(no_dup_rst.size()) or cv < 0) {
                    continue;
                }

                // RST is closed: the should be a destination to src
                auto dest = find_state_from_word(no_dup_rst, dest_word, cv, teacher);

                auto char_as_str = std::string() + c;
                res.emplace_back(std::make_tuple(src, c, wc.get_cv(char_as_str), dest));
            }
        }

        return res;
    }

    /**
     * Find the name of the corresponding row in a RST from a given word.
     * Note that the given word may not be the name of a row of the RST.
     * @param rst The source RST
     * @param state_word The word which row needs to be found
     * @param cv The counter value of the state_word
     * @param teacher A teacher that may be used for membership queries.
     * @return The name and cv of the matching row as a V1CA_Vertex object
     * @throws invalid_argument if the cv is incorrect
     * @throws runtime_error if no matching state was found. This may be due to a RST that was not closed
     * or an out of context state_word.
     */
    std::string
    behaviour_graph::find_state_from_word(RST &rst, const std::string &state_word, int cv, teacher &teacher) {

        if (cv < 0 or cv >= static_cast<int>(rst.size()))
            throw std::invalid_argument("find_state_from_word(): cv out of bound of RST.");

        const auto &cv_table = rst.get_ctables()[cv];
        auto word_pos = std::find(cv_table.get_row_labels().begin(), cv_table.get_row_labels().end(), state_word);
        if (word_pos != cv_table.get_row_labels().end())
            return state_word;

        auto rst_cp = RST(rst);
        rst_cp.add_row_using_query(state_word, cv, teacher, "find_state");
        auto &cp_table = rst_cp.get_tables()[cv];
        // Works by knowing that our word got inserted as last index
        auto &word_row = cp_table.get_data()[cp_table.get_row_labels().size() - 1];

        // Comparing every row to find duplicate
        for (auto i = 0u; i < cp_table.get_data().size() - 1; ++i) {
            auto &row_i = cp_table.get_data()[i];

            if (row_i == word_row)
                return cp_table.get_row_labels()[i];
        }

        throw std::runtime_error("find_state_from_word(): Could not find a matching row in RST."
                                 " Either the RST is not closed, or the word that was asked is out of context.");
    }

    behaviour_graph::behaviour_graph(RST &rst, word_counter &wc, teacher &teacher, alphabet &alphabet) : displayable(
            displayable_type::behaviour_graph) {

        // Removing duplicate rows
        RST no_dup_rst = rst.remove_duplicate_rows();

        // Building vertexes
        std::vector<std::pair<std::string, int>> vertexes;
        for (auto i = 0u; i < no_dup_rst.get_tables().size(); ++i) {
            auto &table = no_dup_rst.get_tables()[i];
            for (auto &word : table.get_row_labels()) {
                vertexes.emplace_back(word, i);
            }
        }

        // Final states
        std::set<std::string> final_states;
        auto &table0 = no_dup_rst.get_tables()[0];
        auto &table0_rows = table0.get_row_labels();
        for (auto i = 0u; i < table0_rows.size(); ++i) {
            if (table0.at(i, "")) {
                final_states.insert(table0_rows[i]);
            }
        }

        // Building transitions
        auto edges = get_edges_from_rst(no_dup_rst, wc, vertexes, teacher, alphabet);

        *this = behaviour_graph(vertexes, edges, "", final_states);
    }

    // Deprecated because result of this is incompatible with to_v1ca and to_r1ca
    behaviour_graph::new_edges_t
    behaviour_graph::link_period(behaviour_graph::couples_t &couples) {
        new_edges_t new_edges;

        for (const auto &couple : couples) {
            auto state1 = find_vertex_by_name(couple.first);
            auto state2 = find_vertex_by_name(couple.second);

            for (auto &edge : get_edges_from_state(state1)) {
                auto &edge_prop = graph_[edge];
                if (edge_prop.effect == 1) {
                    // linking state2 to dest of this edge
                    auto new_edge = boost::add_edge(state2, boost::target(edge, graph_), edge_prop, graph_);
                    if (!new_edge.second)
                        throw std::runtime_error("link_period(): Could not build edge using boost::add_edge.");

                    new_edges.first.emplace_back(new_edge.first.m_source, new_edge.first.m_target);
                }
            }
            for (auto &edge : get_edges_from_state(state2)) {
                auto &edge_prop = graph_[edge];
                if (edge_prop.effect == -1) {
                    // linking state1 to the dest of this edge
                    auto new_edge = boost::add_edge(state1, boost::target(edge, graph_), edge_prop, graph_);
                    if (!new_edge.second)
                        throw std::runtime_error("link_period(): Could not build edge using boost::add_edge.");

                    new_edges.second.emplace_back(new_edge.first.m_source, new_edge.first.m_target);
                }
            }
        }

        return new_edges;
    }

    void behaviour_graph::delete_high_levels(unsigned int threshold_level) {
        // Using a weird workaround on property deletion with VecS
        boost::graph_traits<graph_t>::vertex_iterator v_it, v_it_end, v_it_next;
        std::tie(v_it, v_it_end) = boost::vertices(graph_);
        unsigned long graph_size = v_it_end - v_it;
        for (unsigned long index = graph_size - 1; index < graph_size; --index) {
            auto vertex = graph_[index];
            if (vertex.level > static_cast<int>(threshold_level)) {
                boost::clear_vertex(index, graph_);
                boost::remove_vertex(index, graph_);
            }
        }

        max_level_ = threshold_level;
    }

    behaviour_graph behaviour_graph::get_subgraph(unsigned int level_down, unsigned int level_top) {
        auto res = behaviour_graph(*this);

        // Using a weird workaround on property deletion with VecS
        boost::graph_traits<graph_t>::vertex_iterator v_it, v_it_end;
        std::tie(v_it, v_it_end) = boost::vertices(res.graph_);
        unsigned long graph_size = v_it_end - v_it;
        for (unsigned long index = graph_size - 1; index < graph_size; --index) {
            auto vertex = res.graph_[index];
            if (vertex.level < static_cast<int>(level_down) or vertex.level > static_cast<int>(level_top)) {
                boost::clear_vertex(index, res.graph_);
                boost::remove_vertex(index, res.graph_);
            }
        }

        return res;
    }

    std::optional<behaviour_graph::couples_t>
    behaviour_graph::find_period(unsigned int level, unsigned int width, alphabet &alphabet) {
        if (!width)
            throw std::invalid_argument("find_period(): width cannot be 0.");

        auto subgraph1 = get_subgraph(level, level + width);
        auto subgraph2 = get_subgraph(level + width, level + 2 * width);

        return subgraph1.is_isomorphic_to(subgraph2, level, level + width, alphabet);
    }

    std::set<behaviour_graph::edge_descriptor_t>
    behaviour_graph::get_edges_from_state(behaviour_graph::vertex_descriptor_t state) {
        std::set<edge_descriptor_t> res;
        for (auto edge = boost::out_edges(state, graph_); edge.first < edge.second; ++edge.first)
            res.insert(*edge.first);

        return res;
    }

    /**
     * Recursive implementation of is_isomorphic_to
     */
    bool behaviour_graph::is_isomorphic_to_(behaviour_graph &other,
                                            behaviour_graph::states_t &states1,
                                            behaviour_graph::states_t &states2,
                                            couples_t &res,
                                            behaviour_graph::label_map_t &label_map,
                                            alphabet &alphabet) {
        if (states1.empty() and states2.empty()) {
            return true;
        }

        for (auto state_index1 : states1) {
            for (auto state_index2 : states2) {

                auto label_map_cp = label_map_t(label_map);
                if (is_state_isomorphic(other, state_index1, state_index2, label_map_cp, alphabet)) {
                    // Let's remove the two states from the list (by copy because recursion) and look for other couples
                    auto new_states1 = states_t(states1);
                    auto new_states2 = states_t(states2);
                    new_states1.erase(std::remove(new_states1.begin(), new_states1.end(), state_index1),
                                      new_states1.end());
                    new_states2.erase(std::remove(new_states2.begin(), new_states2.end(), state_index2),
                                      new_states2.end());

                    auto isomorphism_found = is_isomorphic_to_(other, new_states1, new_states2, res, label_map_cp,
                                                               alphabet);
                    if (isomorphism_found) {
                        res.emplace_back(graph_[state_index1].name,
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
    std::optional<behaviour_graph::couples_t>
    behaviour_graph::is_isomorphic_to(behaviour_graph &other, unsigned int from_level1, unsigned from_level2,
                                      alphabet &alphabet) {

        states_t starting_states_1 = get_all_states_of_level(from_level1);
        states_t starting_states_2 = other.get_all_states_of_level(from_level2);

        if (starting_states_1.size() != starting_states_2.size()) {
            return std::nullopt;
        }

        couples_t res;
        label_map_t labels;
        if (is_isomorphic_to_(other, starting_states_1, starting_states_2, res, labels, alphabet)) {
            return res;
        }

        return std::nullopt;
    }

    /**
     * Extract all states of a specific level (counter value) of a V1CA.
     * @param level The level of the states
     * @return A list of the states
     */
    behaviour_graph::states_t behaviour_graph::get_all_states_of_level(unsigned int level) {
        states_t res;
        for (auto vp = boost::vertices(graph_); vp.first != vp.second; ++vp.first) {
            auto state_index = *vp.first;
            if (graph_[state_index].level == static_cast<int>(level)) {
                res.emplace_back(state_index);
            }
        }

        return res;
    }

    bool behaviour_graph::is_state_isomorphic(behaviour_graph &other, behaviour_graph::vertex_descriptor_t state1,
                                              behaviour_graph::vertex_descriptor_t state2,
                                              behaviour_graph::label_map_t &label_map,
                                              alphabet &alphabet) {
        auto queue1 = std::queue<vertex_descriptor_t>();
        auto queue2 = std::queue<vertex_descriptor_t>();
        auto visited = std::set<vertex_descriptor_t>();

        // Checking if both states are final, or not, or initial, or not
        // Using xor to check if values are different
        if ((is_final(graph_[state1].name) ^ other.is_final(other.graph_[state2].name))
            or (is_init(graph_[state1].name) ^ other.is_init(other.graph_[state2].name))) {
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
            for (auto symbol : alphabet.symbols()) {
                // Checking if same next
                auto next1 = get_next_vertex(q1, symbol);
                auto next2 = other.get_next_vertex(q2, symbol);

                if (next1.has_value() ^ next2.has_value())
                    return false;
                if (is_init(graph_[*next1].name) ^ other.is_init(other.graph_[*next2].name))
                    return false;
                if (is_final(graph_[*next1].name) ^ other.is_final(other.graph_[*next2].name))
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
                auto prev1 = get_prev_vertex(q1, symbol);
                auto prev2 = other.get_prev_vertex(q2, symbol);

                if (prev1.has_value() ^ prev1.has_value())
                    return false;
                if (is_init(graph_[*prev1].name) ^ other.is_init(other.graph_[*prev2].name))
                    return false;
                if (is_final(graph_[*prev1].name) ^ other.is_final(other.graph_[*prev2].name))
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

    std::optional<behaviour_graph::vertex_descriptor_t>
    behaviour_graph::get_next_vertex(vertex_descriptor_t from, char c) {
        for (auto vp = boost::out_edges(from, graph_); vp.first != vp.second; ++vp.first) {
            if (graph_[*vp.first].symbol == c) {
                return boost::target(*vp.first, graph_);
            }
        }

        return std::nullopt;
    }

    std::optional<behaviour_graph::vertex_descriptor_t>
    behaviour_graph::get_prev_vertex(vertex_descriptor_t to, char c) {
        for (auto vp = boost::edges(graph_); vp.first != vp.second; ++vp.first) {
            if (boost::target(*vp.first, graph_) == to and graph_[*vp.first].symbol == c) {
                return boost::source(*vp.first, graph_);
            }
        }

        return std::nullopt;
    }

    V1CA::couples_t behaviour_graph::to_v1ca_couple(couples_t couples) {
        V1CA::couples_t res;

        for (const auto &e: couples) {
            res.push_back({static_cast<V1CA::state_t>(find_vertex_by_name(e.first)),
                             static_cast<V1CA::state_t>(find_vertex_by_name(e.second))});
        }

        return res;
    }

    std::shared_ptr<V1CA> behaviour_graph::to_v1ca(RST &rst_no_dup, visibly_alphabet_t &alphabet, bool verbose) {

        if (rst_no_dup.size() < 3) {
            if (verbose)
                std::cout << "Behaviour graph does not have enough levels to find a period.";
            return std::make_shared<V1CA>(to_v1ca_direct(alphabet));
        }

        // m (the potential cv of the period) goes from 0 to max_cv
        //for (auto m = 0u; m < rst_no_dup.size(); ++m) {
        for (auto m = 1u; m == 1u; m = 0) {
            // k (the potential width of the period) goes from ((max_cv - m) / 2) to 1
            // trying with only 1 for now
            // for (auto k = (rst_no_dup.size() - m) / 2; k >= 1; --k) {
            for (auto k = 1u; k == 1u; k = 0) {
                // Checking if level m and level m+k are isomorphic
                auto couples = find_period(m, k, alphabet);
                if (couples.has_value()) {
                    if (verbose) {
                        std::cout << "Periodic pattern found between level " << m << " and " << m + k << ".\n";
                        std::cout << "Couples are: ";
                        for (const auto& couple : couples.value())
                            std::cout << '(' << couple.first << ", " << couple.second << ")";
                        std::cout << "\nMax level is " << m + k << "." << std::endl;
                    }

                    auto bg_cp = behaviour_graph(*this);
                    bg_cp.delete_high_levels(m + k);

                    auto res = bg_cp.to_v1ca_direct(alphabet);
                    auto v1ca_couples = bg_cp.to_v1ca_couple(couples.value());
                    res.link_and_color_edges(v1ca_couples);

                    return std::make_shared<V1CA>(res);
                }
            }
        }

        if (verbose)
            std::cout << "No periodic pattern found. Returning behaviour graph as it is.\n";

        return std::make_shared<V1CA>(to_v1ca_direct(alphabet));
    }

    V1CA behaviour_graph::to_v1ca_direct(visibly_alphabet_t &alphabet) {
        auto vp = boost::vertices(graph_);
        auto states = static_cast<size_t>(vp.second - vp.first);
        auto states_props = std::vector<V1CA::state_prop>(states);

        auto finals = std::vector<V1CA::state_t>();

        for (; vp.first != vp.second; ++vp.first) {
            auto prop = graph_[*vp.first];
            for (auto &name: final_states_) {
                if (name == prop.name)
                    finals.emplace_back(static_cast<V1CA::state_t>(*vp.first));
            }
            states_props[*vp.first] = {static_cast<size_t>(prop.level), prop.name};
        }

        std::cout << "Creating V1CA, adding edges:\n";
        auto transitions = std::vector<std::tuple<size_t, size_t, char>>();
        for (auto ep = boost::edges(graph_); ep.first != ep.second; ++ep.first) {
            auto prop = graph_[*ep.first];
            transitions.emplace_back(std::make_tuple(boost::source(*ep.first, graph_),
                                               boost::target(*ep.first, graph_),
                                               prop.symbol));
            std::cout << "(" << boost::source(*ep.first, graph_) << " " << boost::target(*ep.first, graph_) << ") ";
        }
        std::cout << '\n';

        return V1CA(states_props, 0lu, finals, alphabet, transitions);
    }

    R1CA behaviour_graph::to_r1ca_direct(basic_alphabet &alphabet, const behaviour_graph::new_edges_t &new_edges, size_t new_edge_lvl) {
        auto vertices = boost::vertices(graph_);
        auto states = static_cast<size_t>(vertices.second - vertices.first);

        std::vector<vertex_descriptor_t> finals;

        for (; vertices.first != vertices.second; ++vertices.first) {
            if (is_final(graph_[*vertices.first].name))
                finals.emplace_back(*vertices.first);
        }

        std::vector<std::tuple<size_t, size_t, char, int>> transitions;
        for (auto ep = boost::edges(graph_); ep.first != ep.second; ++ep.first) {
            auto edge = *ep.first;
            auto src = boost::source(edge, graph_);
            auto dest = boost::target(edge, graph_);
            auto prop = graph_[edge];
            transitions.emplace_back(std::make_tuple(src, dest, prop.symbol, prop.effect));
        }

        std::map<utils::triple_comp<size_t, size_t, char>, utils::pair_comp<bool, size_t>> colors;
        for (auto new_e : new_edges.second) {
            auto src = new_e.first;
            auto dest = new_e.second;
            auto symbol = graph_[find_edge(new_e.first, new_e.second)].symbol;
            colors[{src, dest, symbol}] = {false, new_edge_lvl};

            for (auto &trans: transitions) {
                auto trans_src = std::get<0>(trans);
                auto trans_dest = std::get<1>(trans);
                auto trans_symbol = std::get<2>(trans);
                if (trans_src == src and trans_symbol == symbol)
                    colors[{trans_src, trans_dest, trans_symbol}] = {true, new_edge_lvl};
            }
        }

        return R1CA(states, max_level_, finals, transitions, colors, alphabet);
    }

    std::shared_ptr<R1CA> behaviour_graph::to_r1ca(RST &rst_no_dup, basic_alphabet &alphabet, bool verbose) {
        if (rst_no_dup.size() < 3) {
            if (verbose)
                std::cout << "Behaviour graph does not have enough levels to find a period.";
            return std::make_shared<R1CA>(to_r1ca_direct(alphabet));
        }

        // m (the potential cv of the period) goes from 0 to max_cv
        //for (auto m = 0u; m < rst_no_dup.size(); ++m) {
        for (auto m = 1u; m == 1u; m = 0) {
            // k (the potential width of the period) goes from ((max_cv - m) / 2) to 1
            // trying with only 1 for now
            // for (auto k = (rst_no_dup.size() - m) / 2; k >= 1; --k) {
            for (auto k = 1u; k == 1u; k = 0) {
                // Checking if level m and level m+k are isomorphic
                auto couples = find_period(m, k, alphabet);
                if (couples.has_value()) {
                    if (verbose) {
                        std::cout << "Periodic pattern found between level " << m << " and " << m + k << ".\n";
                        std::cout << "Couples are: ";
                        for (const auto& couple : couples.value())
                            std::cout << '(' << couple.first << ", " << couple.second << ")";
                        std::cout << "\n";
                    }

                    auto bg_cp = behaviour_graph(*this);
                    bg_cp.delete_high_levels(m + k);
                    auto new_edges = bg_cp.link_period(*couples);

                    return std::make_shared<R1CA>(to_r1ca_direct(alphabet, new_edges, m));
                }
            }
        }

        if (verbose)
            std::cout << "No periodic pattern found. Returning behaviour graph as it is.\n";

        return std::make_shared<R1CA>(to_r1ca_direct(alphabet));
    }

    R1CA behaviour_graph::to_r1ca_direct(basic_alphabet &alphabet) {
        // Creating a finite state automaton, leaving the counter useless
        auto vertices = boost::vertices(graph_);
        auto states = static_cast<size_t>(vertices.second - vertices.first);

        std::vector<vertex_descriptor_t> finals;

        for (; vertices.first != vertices.second; ++vertices.first) {
            if (is_final(graph_[*vertices.first].name))
                finals.emplace_back(*vertices.first);
        }

        std::vector<std::tuple<size_t, size_t, char, int>> transitions;
        for (auto ep = boost::edges(graph_); ep.first != ep.second; ++ep.first) {
            auto edge = *ep.first;
            auto src = boost::source(edge, graph_);
            auto dest = boost::target(edge, graph_);
            auto prop = graph_[edge];
            transitions.emplace_back(std::make_tuple(src, dest, prop.symbol, prop.effect));
        }

        std::map<utils::triple_comp<size_t, size_t, char>, utils::pair_comp<bool, size_t>> colors;
        // max_lvl is infinite cause we do not use the counter
        return R1CA(states, UINT64_MAX, finals, transitions, colors, alphabet);
    }

    behaviour_graph::edge_descriptor_t
    behaviour_graph::find_edge(behaviour_graph::vertex_descriptor_t src, behaviour_graph::vertex_descriptor_t dst) {
        for (auto edges = boost::edges(graph_); edges.first != edges.second; ++edges.first) {
            auto ed = *edges.first;
            if (boost::target(ed, graph_) == dst and boost::source(ed, graph_) == src)
                return ed;
        }

        throw std::invalid_argument("No edge found for given vertex descriptors");
    }

    behaviour_graph behaviour_graph::from_v1ca(const V1CA &v1ca) {
        // TODO
        (void) v1ca;
        return behaviour_graph();
    }

}
