#pragma once

#include "R1CA.h"
#include "dataframe.h"

#include <any>

namespace active_learning {

    class R1CA_builder {

    public:
        using edges_t = std::vector<std::tuple<R1CA_vertex, R1CA_vertex, char>>;
        using couples_t = R1CA::couples_t;

        using edges_descs_t = std::vector<R1CA::edge_descriptor_t>;
        using looped_edges_t = std::pair<std::vector<R1CA::edge_descriptor_t>, std::vector<R1CA::edge_descriptor_t>>;

    private:

        static edges_t
        get_edges_from_rst(RST &no_dup_rst, visibly_alphabet_t &alphabet, std::vector<R1CA_vertex> &states,
                           teacher &teacher);

        static R1CA_vertex find_state_from_word(RST &rst, const std::string &state_word, int cv, teacher &teacher);

        static std::optional<couples_t> find_period(unsigned int level, unsigned int width, R1CA &automaton);

        static R1CA get_subgraph(R1CA &automaton, unsigned int level_down, unsigned int level_top);

        static void delete_high_levels(R1CA &automaton, unsigned int threshold_level);

        static edges_descs_t get_edges_from_state(R1CA &automaton, R1CA::vertex_descriptor_t);

        static looped_edges_t link_period(R1CA &automaton, couples_t &couples, visibly_alphabet_t &alphabet);

        static void color_edges(R1CA &automaton, looped_edges_t &new_edges);

    public:

        static R1CA build_behaviour_graph_from_RST(RST rst, visibly_alphabet_t &alphabet, teacher &teacher);

        // Inplace
        static R1CA &behaviour_graph_to_R1CA(R1CA &automaton, RST &rst_no_dup, visibly_alphabet_t &alphabet,
                                             bool verbose = false);
    };
}
