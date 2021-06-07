#pragma once

#include <string>
#include <optional>
#include <boost/graph/adjacency_list.hpp>

#include "language.h"

namespace active_learning {

    struct V1CA_vertex {
        std::string name; // name must be unique
        unsigned cv;  // Counter value level

        V1CA_vertex(std::string name, unsigned int cv);

        V1CA_vertex() = default;
    };

    struct V1CA_edge {
        char symbol;

        // Constructor
        V1CA_edge(char symbol);

        V1CA_edge() = default;
    };

    class V1CA {
    public:
        using graph_t = boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS, V1CA_vertex, V1CA_edge>;
        using vertex_descriptor_t = boost::graph_traits<graph_t>::vertex_descriptor;
        using edge_descriptor_t = boost::graph_traits<graph_t>::edge_descriptor;
        using label_map_t = std::map<unsigned long int, vertex_descriptor_t>;
        using couples_t = std::optional<std::vector<std::pair<vertex_descriptor_t, vertex_descriptor_t>>>;
        using states_t = std::vector<vertex_descriptor_t>;

        using graph_color_t = std::set<edge_descriptor_t>;
        using graph_colors_t = std::tuple<graph_color_t, // 0: "init edges"
                graph_color_t, // 1: "loop in no condition"
                graph_color_t, // 2: "loop in condition"
                graph_color_t>; // 3: "loop out"

    private:
        void link_by_name(std::string, std::string, char symbol);

        states_t get_all_states_of_level(unsigned int level);

        // recursive function
        bool is_isomorphic_to_(V1CA &other, states_t &states1, states_t &states2, couples_t &res, label_map_t &);

        bool is_state_isomorphic(V1CA &other, vertex_descriptor_t state1, vertex_descriptor_t state2, label_map_t &);

    public:
        V1CA();

        V1CA(std::vector<V1CA_vertex> &states, std::vector<V1CA_vertex> &initial_states,
             std::vector<V1CA_vertex> &final_states, const alphabet_t &alphabet,
             std::vector<std::tuple<V1CA_vertex, V1CA_vertex, char>> &edges);

        graph_t &get_mutable_graph();

        couples_t is_isomorphic_to(V1CA &other, unsigned int from_level1, unsigned from_level2);

        void display(const std::string &path);

        void remove_state(vertex_descriptor_t vertex_descriptor);

        bool is_final(const V1CA_vertex &state);

        bool is_init(const V1CA_vertex &state);

        std::optional<unsigned long> get_next_index(vertex_descriptor_t state_index, char c);

        std::optional<unsigned long> get_prev_index(vertex_descriptor_t state_index, char c);

        std::optional<int> get_period_cv();

        void set_period_cv(int cv);

        void set_periodic(bool periodic);

        void set_colored(bool colored);

        graph_color_t &get_mutable_init_edge_color();

        graph_color_t &get_mutable_loop_in_no_cond_color();

        graph_color_t &get_mutable_loop_in_with_cond_color();

        graph_color_t &get_mutable_loop_out_color();

    private:
        std::unordered_set<std::string> init_states_;
        std::unordered_set<std::string> final_states_;
        std::shared_ptr<alphabet_t> alphabet_;
        std::shared_ptr<graph_t> graph = nullptr;
        graph_colors_t colors_;
        int period_cv_ = -1;
        bool periodic_ = false;
        bool colored_ = false;
    };

}

// V1C2AL_V1CA_H
