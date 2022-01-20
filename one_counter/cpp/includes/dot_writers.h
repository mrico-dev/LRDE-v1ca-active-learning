#pragma once

#include "V1CA.h"
#include "R1CA.h"
#include "behaviour_graph.h"
#include "alphabet.h"

namespace active_learning {

    class writer {

    public:
        static void write_v1ca(std::ostream &out, V1CA& automaton);
    };

    // Used for graphviz dot vertex output
    class vertex_writer {

    public:
        explicit vertex_writer(displayable &display_obj);

        void write_behaviour_graph(std::ostream &out, const behaviour_graph::vertex_descriptor_t &v);

        template<class Vertex>
        void operator()(std::ostream &out, const Vertex &v) {

            if (to_display_.get_displayable_type() == displayable_type::behaviour_graph) {
                write_behaviour_graph(out, static_cast<behaviour_graph::vertex_descriptor_t>(v));
            } else {
                throw std::invalid_argument("Cannot display edge for this type of automaton.");
            }
        }

    private:
        displayable &to_display_;
    };

    // Used for graphviz dot edge output
    class edge_writer {

    public:
        explicit edge_writer(displayable &display_obj) : to_display_(display_obj) {}

        void write_behaviour_graph(std::ostream &out, const behaviour_graph::edge_descriptor_t &e);

        template<class Edge>
        void operator()(std::ostream &out, const Edge &e) {
            if (to_display_.get_displayable_type() == displayable_type::behaviour_graph) {
                write_behaviour_graph(out, static_cast<behaviour_graph::edge_descriptor_t>(e));
            } else {
                throw std::invalid_argument("Cannot display edge for this type of object.");
            }
        }

    private:
        displayable &to_display_;
    };
}
