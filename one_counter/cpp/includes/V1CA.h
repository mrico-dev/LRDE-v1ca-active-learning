#ifndef V1C2AL_V1CA_H
#define V1C2AL_V1CA_H

/*
#include <string>
#include <optional>
#include <boost/graph/adjacency_list.hpp>
#include "language.h"
*/

namespace active_learning {

    struct V1CA_vertex {
        std::string name;
        unsigned cv;  // Counter value level

        V1CA_vertex(const std::string &name, unsigned int cv);
    };

    struct V1CA_edge {
        char symbol;

        // Constructor
        explicit V1CA_edge(char symbol);
    };

    class V1CA {
    public:
        using couples_t = std::optional<std::vector<std::pair<V1CA_vertex, V1CA_vertex>>>;
        using graph_t = boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS, V1CA_vertex, V1CA_edge>;
        using states_t = std::vector<unsigned long>; // Unsigned long is the index for vertexes
        using label_map_t = std::unordered_map<unsigned long, unsigned long>;

    private:
        void link_by_name(std::string, std::string, char symbol);

        states_t get_all_states_of_level(unsigned int level);

        // recursive function
        bool is_isomorphic_to_(V1CA &other, states_t &states1, states_t &states2, couples_t &res, label_map_t&);
        bool is_state_isomorphic(V1CA &other, unsigned long state1, unsigned long state2, label_map_t&);

    public:
        V1CA();

        V1CA(std::vector<V1CA_vertex> &states, std::vector<V1CA_vertex> &initial_states,
             std::vector<V1CA_vertex> &final_states, alphabet_t &alphabet,
             std::vector<std::tuple<V1CA_vertex, V1CA_vertex, char>> &edges);

        graph_t &get_mutable_graph();

        couples_t is_isomorphic_to(V1CA &other, unsigned int from_level1, unsigned from_level2);

        void display(const std::string &path);

        bool is_final(const V1CA_vertex &state);
        bool is_init(const V1CA_vertex &state);
        std::optional<unsigned long> get_next_index(unsigned long state_index, char c);
        std::optional<unsigned long> get_prev_index(unsigned long state_index, char c);

    private:

        std::unordered_set<V1CA_vertex> init_states_;
        std::unordered_set<V1CA_vertex> final_states_;
        alphabet_t alphabet_;

        graph_t graph;
    };

}

#endif // V1C2AL_V1CA_H
