#pragma once

#include <string>
#include <boost/graph/adjacency_list.hpp>
#include "V1CA.h"
#include "R1CA.h"
#include "displayable.h"
#include "dataframe.h"

namespace active_learning {

    class teacher;

    struct bg_vertex_attr {
        std::string name;
        int level{};

        bg_vertex_attr(const std::string &name, int level);

        bg_vertex_attr();
    };

    struct bg_edge_attr {
        char symbol = 0;
        int effect = 0;

        bg_edge_attr(char symbol, int effect);

        explicit bg_edge_attr(char symbol);

        bg_edge_attr();
    };

    class behaviour_graph : public displayable {
    public:
        using graph_t = boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS, bg_vertex_attr, bg_edge_attr>;
        using vertex_descriptor_t = typename boost::graph_traits<graph_t>::vertex_descriptor;
        using edge_descriptor_t = typename boost::graph_traits<graph_t>::edge_descriptor;
        using couples_t = std::vector<std::pair<std::string, std::string>>;
    private:
        using edges_t = std::vector<std::tuple<std::string, char, int, std::string>>;
        using vertexes_t = std::vector<std::pair<std::string, int>>;
        using edges_descs_t = std::vector<V1CA::edge_descriptor_t>;
        using looped_edges_t = std::pair<std::vector<V1CA::edge_descriptor_t>, std::vector<V1CA::edge_descriptor_t>>;
        using label_map_t = std::map<unsigned long int, vertex_descriptor_t>;
        using states_t = std::vector<vertex_descriptor_t>;

    private:
        vertex_descriptor_t find_vertex_by_name(const std::string &name);

        edges_t
        get_edges_from_rst(RST &rst, word_counter &wc, vertexes_t &states, teacher &teacher, alphabet &alphabet);

        static std::string find_state_from_word(RST &rst, const std::string &state_word, int cv, teacher &teacher);

        std::set<edge_descriptor_t> get_edges_from_state(vertex_descriptor_t state);

        looped_edges_t link_period(couples_t &couples);

        void delete_high_levels(unsigned int threshold_level);

        behaviour_graph get_subgraph(unsigned int level_down, unsigned int level_top);

        std::optional<couples_t> find_period(unsigned int level, unsigned int width, alphabet &alphabet);

        bool is_isomorphic_to_(behaviour_graph &other, states_t &states1, states_t &states2, couples_t &res,
                               label_map_t &label_map, alphabet &alphabet);

        states_t get_all_states_of_level(unsigned int level);

        bool is_state_isomorphic(behaviour_graph &other, vertex_descriptor_t state1, vertex_descriptor_t state2,
                                 label_map_t &, alphabet &);

        std::optional<vertex_descriptor_t> get_next_vertex(vertex_descriptor_t from, char c);

        std::optional<vertex_descriptor_t> get_prev_vertex(vertex_descriptor_t to, char c);


    public:
        behaviour_graph();

        behaviour_graph(const vertexes_t &states,
                        const edges_t &edges,
                        const std::string &init_state,
                        std::set<std::string> final_states);

        behaviour_graph(RST &rst, word_counter &wc, teacher &teacher, alphabet &alphabet);

        void display(const std::string &path) override;

        graph_t &get_mutable_graph();

        std::shared_ptr<V1CA> to_v1ca(RST &rst_no_dup, visibly_alphabet_t &alphabet, bool verbose);

        V1CA to_v1ca_direct(visibly_alphabet_t &alphabet);

        std::shared_ptr<R1CA> to_r1ca(RST &rst_no_dup, basic_alphabet &alphabet, bool verbose);

        R1CA to_r1ca_direct(basic_alphabet &alphabet);

        R1CA to_r1ca_direct(basic_alphabet &alphabet,
                            const looped_edges_t &new_edges,
                            size_t new_edge_lvl);

        bool is_final(const std::string &v_name);

        bool is_init(const std::string &v_name);

        std::optional<behaviour_graph::couples_t>
        is_isomorphic_to(behaviour_graph &other, unsigned int from_level1, unsigned int from_level2,
                         alphabet &alphabet);

    private:
        graph_t graph_;
        std::string init_state_;
        std::unordered_set<std::string> final_states_;
        size_t max_level_ = 0;
    };
}
