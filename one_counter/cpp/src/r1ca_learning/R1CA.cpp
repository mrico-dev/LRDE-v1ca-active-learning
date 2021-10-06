#include "R1CA.h"
#include "dot_writers.h"

#include <utility>
#include <fstream>
#include <boost/graph/graphviz.hpp>

namespace active_learning {

    R1CA_vertex::R1CA_vertex(std::string name) : name(std::move(name)) {}

    R1CA_vertex::R1CA_vertex() {}

    R1CA_edge::R1CA_edge(char symbol, int counterChange) : symbol(symbol), counter_change(counterChange) {
        cond = false;
    }

    R1CA_edge::R1CA_edge(char symbol, int counterChange, int condVal, bool condInfeq) :
            symbol(symbol),
            counter_change(counterChange),
            cond_val(condVal),
            cond_infeq(condInfeq) {
        cond = true;
    }

    R1CA_edge::R1CA_edge() {}

    // TODO optimize with one function to evaluate and one to count (because this one does not break on neg counter)
    std::pair<bool, int> R1CA::evaluate(const std::string &word) {
        // Starting at initial state with counter = 0
        auto curr_state = find_vertex_by_name(init_state);
        auto counter = 0;
        auto acceptable = true;
        // Getting from states to states using symbol of the word
        for (char c : word) {
            auto no_neighbor = true;
            auto out_edges = boost::make_iterator_range(boost::out_edges(curr_state, graph_));
            for (auto e : out_edges) {
                auto e_prop = graph_[e];
                if (e_prop.symbol == c and (!e_prop.cond or (e_prop.cond_infeq ^ (counter > e_prop.cond_val)))) {
                    counter += e_prop.counter_change;
                    curr_state = boost::target(e, graph_);
                    no_neighbor = false;
                    break;
                }
            }
            if (no_neighbor)
                return std::make_pair(false, counter);
            if (counter < 0)
                acceptable = false;
        }

        auto accepted = !counter and acceptable and final_states_.contains(graph_[curr_state].name);
        return std::make_pair(accepted, counter);
    }

    R1CA::vertex_descriptor_t R1CA::find_vertex_by_name(const std::string &vertex_name) {
        for (auto vi = boost::vertices(graph_); vi.first != vi.second; ++vi.first) {
            if (graph_[*vi.first].name == vertex_name) {
                return *vi.first;
            }
        }

        throw std::invalid_argument("Could not find state with name '" + vertex_name + "'.");
    }

    const alphabet_t &R1CA::get_alphabet() const {
        return alphabet_;
    }

    R1CA::R1CA(alphabet_t &alphabet) : one_counter_automaton(), alphabet_(alphabet) {}

    void R1CA::display(const std::string &path) const {
        // Writing dot file
        std::string full_path = path + ".dot";
        std::ofstream file;
        file.open(full_path);
        boost::write_graphviz(file, graph_,
                              vertex_writer<R1CA>(*this),
                              edge_writer<R1CA, alphabet_t>(*this, alphabet_));

        // Creating png file
        // Hoping that you are on linux and have dot installed
        system(("dot -Tpng " + full_path + " > " + path + ".png").c_str());
    }

    R1CA::graph_t &R1CA::get_mutable_graph() {
        return graph_;
    }

    bool R1CA::is_final(const R1CA_vertex &v) {
        return final_states_.contains(v.name);
    }
}