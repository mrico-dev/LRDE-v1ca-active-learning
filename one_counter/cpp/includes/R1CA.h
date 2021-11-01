#pragma once

#include "one_counter_automaton.h"

#include <string>

namespace active_learning {

    struct R1CA_vertex {
        std::string name;
        int cv;

        explicit R1CA_vertex(std::string name, int cv);

        R1CA_vertex();
    };

    struct R1CA_edge {
        char symbol;
        int counter_change;
        bool cond;
        int cond_val;
        bool cond_infeq;

        R1CA_edge(char symbol, int counterChange, int condVal, bool condInfeq);

        R1CA_edge(char symbol, int counterChange);

        R1CA_edge();
    };


    class R1CA : public one_counter_automaton {
    public:
        using graph_t = boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS, R1CA_vertex, R1CA_edge>;
        using vertex_descriptor_t = typename boost::graph_traits<graph_t>::vertex_descriptor;
        using edge_descriptor_t = typename boost::graph_traits<graph_t>::edge_descriptor;
        using couples_t = std::vector<std::pair<std::string, std::string>>;

    private:
        vertex_descriptor_t find_vertex_by_name(const std::string& vertex_name);

    public:
        std::pair<bool, int> evaluate(const std::string &word);

        const alphabet_t &get_alphabet() const;

        R1CA(alphabet_t &alphabet);

        R1CA(std::vector<std::string> &states,
             std::vector<std::tuple<std::string, R1CA_edge, std::string>> edges, const std::string &init_state,
             const std::unordered_set<std::string> &final_states, alphabet_t &alphabet);

        void display(const std::string& path) const override;

        bool is_final(const R1CA_vertex &v);

        graph_t &get_mutable_graph();

    private:
        graph_t graph_;
        std::string init_state_;
        std::unordered_set<std::string> final_states_;
        alphabet_t &alphabet_;
    };
}
