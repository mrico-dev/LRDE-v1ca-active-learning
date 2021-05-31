#ifndef V1C2AL_V1CA_BUILDER_H
#define V1C2AL_V1CA_BUILDER_H

#include "V1CA.h"

namespace active_learning {

    class V1CA_builder {

    public:
        using edges_t = std::vector<std::tuple<V1CA_vertex, V1CA_vertex, char>>;
        using couples_t = V1CA::couples_t;

    private:
        static edges_t get_edges_from_rst(RST &no_dup_rst, const alphabet_t &alphabet, std::vector<V1CA_vertex> &states,
                                          teacher &teacher);

        static V1CA_vertex find_state_from_word(RST &rst, const std::string &state_word, int cv, teacher &teacher);

        static couples_t find_period(unsigned int level, unsigned int width, V1CA &automaton);

        static V1CA get_subgraph(V1CA &automaton, unsigned int level_down, unsigned int level_top);

    public:
        static V1CA build_behaviour_graph_from_RST(RST rst, const alphabet_t &alphabet, teacher &teacher);

        // Inplace
        static V1CA &behaviour_graph_to_V1CA(V1CA &automaton, RST &rst_no_dup, bool verbose=false);

    };

}

#endif //V1C2AL_V1CA_BUILDER_H
