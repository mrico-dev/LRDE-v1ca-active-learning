#include <iostream>
#include "V1CA_builder.h"

namespace active_learning {

    V1CA V1CA_builder::build_behaviour_graph_from_RST(RST rst, alphabet_t &alphabet, teacher &teacher) {
        // Removing duplicate rows
        RST no_dup_rst = rst.remove_duplicate_rows();

        // Building vertexes
        std::vector<V1CA_vertex> vertexes;
        for (auto i = 0u; i < no_dup_rst.get_tables().size(); ++i) {
            auto &table = no_dup_rst.get_tables()[i];
            for (auto &word : table.get_row_labels()) {
                vertexes.emplace_back(word, i);
            }
        }
        // Init states
        std::vector<V1CA_vertex> init_states;
        init_states.emplace_back("", 0);

        // Final states
        std::vector<V1CA_vertex> final_states;
        auto &table0 = no_dup_rst.get_tables()[0];
        auto &table0_rows = table0.get_row_labels();
        for (auto i = 0u; i < table0_rows.size(); ++i) {
            if (table0.at(i, "")) {
                final_states.emplace_back(table0_rows[i], 0);
            }
        }

        // Building transitions
        edges_t edges = get_edges_from_rst(no_dup_rst, alphabet, vertexes, teacher);

        V1CA behaviour_graph = V1CA(vertexes, init_states, final_states, alphabet, edges);
        return behaviour_graph;
    }

    V1CA_builder::edges_t
    V1CA_builder::get_edges_from_rst(RST &no_dup_rst, alphabet_t &alphabet, std::vector<V1CA_vertex> &states,
                                     teacher &teacher) {
        edges_t res;
        for (auto &st : states) {
            auto &src = st.name;
            for (auto &symbol : alphabet) {
                char c = symbol.first;
                auto dest_word = src + c;
                int cv = get_cv(dest_word, alphabet);

                if (cv >= static_cast<int>(no_dup_rst.size()) or cv < 0) {
                    continue;
                }

                // RST is closed: the should be a destination to src
                V1CA_vertex dest = find_state_from_word(no_dup_rst, dest_word, cv, teacher);

                res.emplace_back(std::make_tuple(st, dest, c));
            }
        }

        return res;
    }

    V1CA_vertex V1CA_builder::find_state_from_word(RST &rst, const std::string &state_word, int cv, teacher &teacher) {

        if (cv < 0 or cv >= static_cast<int>(rst.size()))
            throw std::invalid_argument("find_state_from_word(): cv out of bound of RST.");

        const auto &cv_table = rst.get_ctables()[cv];
        auto word_pos = std::find(cv_table.get_row_labels().begin(), cv_table.get_row_labels().end(), state_word);
        if (word_pos != cv_table.get_row_labels().end())
            return V1CA_vertex(state_word, cv);

        auto rst_cp = RST(rst);
        rst_cp.add_row_using_query(state_word, cv, teacher, "find_state");
        auto &cp_table = rst_cp.get_tables()[cv];
        // Works by knowing that our word got inserted as last index
        auto &word_row = cp_table.get_data()[cp_table.get_row_labels().size() - 1];

        // Comparing every row to find duplicate
        for (auto i = 0u; i < cp_table.get_data().size() - 1; ++i) {
            auto &row_i = cp_table.get_data()[i];

            if (row_i == word_row)
                return V1CA_vertex(cp_table.get_row_labels()[i], cv);
        }

        throw std::runtime_error("find_state_from_word(): Could not find a matching row in RST."
                                 " Either the RST is not closed, or the word that was asked is out of context.");
    }

    V1CA::vertex_descriptor_t V1CA_builder::get_vertex_by_name(V1CA& automaton, const std::string &name) {
        auto &graph = automaton.get_mutable_graph();
        auto v_its = boost::vertices(graph);
        for (; v_its.first != v_its.second; ++v_its.first) {
            if (graph[*v_its.first].name == name)
                return *v_its.first;
        }

        throw std::invalid_argument("get_vertex_by_name(): Could not find vertex");
    }

    V1CA V1CA_builder::get_subgraph(V1CA &automaton, unsigned int level_down, unsigned int level_top) {
        auto res = V1CA(automaton);
        auto &graph = res.get_mutable_graph();

        // Using a weird workaround on property deletion with VecS
        boost::graph_traits<V1CA::graph_t>::vertex_iterator v_it, v_it_end, v_it_next;
        std::tie(v_it, v_it_end) = boost::vertices(graph);
        unsigned long graph_size = v_it_end - v_it;
        for (unsigned long index = graph_size - 1; index < graph_size; --index) {
            auto vertex = graph[index];
            if (vertex.cv < level_down or vertex.cv > level_top) {
                boost::clear_vertex(index, graph);
                boost::remove_vertex(index, graph);
            }
        }

        return res;
    }

    std::optional<V1CA_builder::couples_t>
    V1CA_builder::find_period(unsigned int level, unsigned int width, V1CA &automaton) {
        if (!width)
            throw std::invalid_argument("find_period(): width cannot be 0.");

        auto subgraph1 = get_subgraph(automaton, level, level + width);
        auto subgraph2 = get_subgraph(automaton, level + width, level + 2 * width);

        return subgraph1.is_isomorphic_to(subgraph2, level, level + width);
    }

    V1CA &V1CA_builder::behaviour_graph_to_V1CA(V1CA &automaton, RST &rst_no_dup, alphabet_t &alphabet,
                                                bool verbose) {
        if (rst_no_dup.size() < 3) {
            if (verbose)
                std::cout << "Behaviour graph does not have enough levels to find a period.";
            return automaton;
        }

        // m (the potential cv of the period) goes from 0 to max_cv
        //for (auto m = 0u; m < rst_no_dup.size(); ++m) {
        for (auto m = 1u; m == 1u; m = 0) {
            // k (the potential width of the period) goes from ((max_cv - m) / 2) to 1
            // trying with only 1 for now
            // for (auto k = (rst_no_dup.size() - m) / 2; k >= 1; --k) {
            for (auto k = 1u; k == 1u; k = 0) {
                // Checking if level m and level m+k are isomorphic
                auto couples = find_period(m, k, automaton);
                if (couples.has_value()) {
                    if (verbose)
                        std::cout << "Periodic pattern found between level " << m << " and " << m + k << ".\n";

                    delete_high_levels(automaton, m + k);
                    auto new_edges = link_period(automaton, *couples, alphabet);
                    automaton.set_period_cv(static_cast<int>(m));
                    color_edges(automaton, new_edges);

                    return automaton;
                }
            }
        }

        if (verbose)
            std::cout << "No periodic pattern found. Returning behaviour graph as it is.\n";

        return automaton;
    }

    void V1CA_builder::delete_high_levels(V1CA &automaton, unsigned int threshold_level) {
        auto &graph = automaton.get_mutable_graph();
        // Using a weird workaround on property deletion with VecS
        boost::graph_traits<V1CA::graph_t>::vertex_iterator v_it, v_it_end, v_it_next;
        std::tie(v_it, v_it_end) = boost::vertices(graph);
        unsigned long graph_size = v_it_end - v_it;
        for (unsigned long index = graph_size - 1; index < graph_size; --index) {
            auto vertex = graph[index];
            if (vertex.cv > threshold_level) {
                boost::clear_vertex(index, graph);
                boost::remove_vertex(index, graph);
            }
        }
    }

    V1CA_builder::edges_descs_t V1CA_builder::get_edges_from_state(V1CA &automaton, V1CA::vertex_descriptor_t state) {
        edges_descs_t res;
        auto &graph = automaton.get_mutable_graph();
        for (auto edge_it = boost::edges(graph); edge_it.first != edge_it.second; ++edge_it.first) {
            auto edge_desc = *edge_it.first;

            if (boost::source(edge_desc, graph) == state)
                res.emplace_back(edge_desc);
        }

        return res;
    }

    V1CA_builder::looped_edges_t
    V1CA_builder::link_period(V1CA &automaton, V1CA_builder::couples_t &couples, alphabet_t &alphabet) {
        automaton.set_periodic(true);

        looped_edges_t new_edges;
        auto &graph = automaton.get_mutable_graph();

        for (const auto &couple : couples) {
            auto state1 = get_vertex_by_name(automaton, couple.first);
            auto state2 = get_vertex_by_name(automaton, couple.second);

            for (auto &edge : get_edges_from_state(automaton, state1)) {
                auto edge_prop = graph[edge];
                if (alphabet[edge_prop.symbol] == 1) {
                    // linking state2 to dest of this edge
                    auto new_edge = boost::add_edge(state2, boost::target(edge, graph), edge_prop, graph);
                    if (!new_edge.second)
                        throw std::runtime_error("link_period(): Could not build edge using boost::add_edge.");
                    new_edges.first.emplace_back(new_edge.first);
                }
            }
            for (auto &edge : get_edges_from_state(automaton, state2)) {
                auto edge_prop = graph[edge];
                if (alphabet[edge_prop.symbol] == -1) {
                    // linking state1 to the dest of this edge
                    auto new_edge = boost::add_edge(state1, boost::target(edge, graph), edge_prop, graph);
                    if (!new_edge.second)
                        throw std::runtime_error("link_period(): Could not build edge using boost::add_edge.");
                    new_edges.second.emplace_back(new_edge.first);
                }
            }
        }

        return new_edges;
    }

    void V1CA_builder::color_edges(V1CA &automaton, V1CA_builder::looped_edges_t &new_edges) {
        auto &graph = automaton.get_mutable_graph();
        // Coloring init (others)
        for (auto edge_it = boost::edges(graph); edge_it.first != edge_it.second; ++edge_it.first) {
            automaton.get_mutable_init_edge_color().insert(make_pair_comp((*edge_it.first).m_source, (*edge_it.first).m_target));
        }

        // Coloring loop in no cond edges
        for (auto &edge : new_edges.first) {
            automaton.get_mutable_loop_in_no_cond_color().insert(make_pair_comp(edge.m_source, edge.m_target));
            automaton.get_mutable_init_edge_color().erase(make_pair_comp(edge.m_source, edge.m_target));
        }

        // Coloring loop in with cond edges
        for (auto &edge : new_edges.second) {
            automaton.get_mutable_loop_in_with_cond_color().insert(make_pair_comp(edge.m_source, edge.m_target));
            automaton.get_mutable_init_edge_color().erase(make_pair_comp(edge.m_source, edge.m_target));
        }

        // Coloring loop out edges
        for (auto &loop_in_edge_with_cond : automaton.get_mutable_loop_in_with_cond_color()) {
            for (auto &other_edge_from_same_source : get_edges_from_state(automaton, loop_in_edge_with_cond.first)) {
                if ((loop_in_edge_with_cond.first != other_edge_from_same_source.m_target
                        or loop_in_edge_with_cond.second != other_edge_from_same_source.m_source)
                        and automaton.get_edge(loop_in_edge_with_cond.first, loop_in_edge_with_cond.second).symbol == graph[other_edge_from_same_source].symbol) {
                    automaton.get_mutable_loop_out_color().insert(make_pair_comp(other_edge_from_same_source.m_source, other_edge_from_same_source.m_target));
                    automaton.get_mutable_init_edge_color().erase(make_pair_comp(other_edge_from_same_source.m_source, other_edge_from_same_source.m_target));
                }
            }
        }

        automaton.set_colored(true);
    }
}
