#pragma once

#include "V1CA.h"
#include "R1CA.h"
#include "behaviour_graph.h"
#include "alphabet.h"

namespace active_learning {

    // Used for graphviz dot vertex output
    class vertex_writer {

    public:
        explicit vertex_writer(displayable &display_obj) : to_display_(display_obj) {}

        void write_behaviour_graph(std::ostream &out, const behaviour_graph::vertex_descriptor_t &v) {
            auto &bg = dynamic_cast<behaviour_graph &>(to_display_);
            auto &g = bg.get_mutable_graph();
            const auto prop = g[v];
            auto final = bg.is_final(prop.name);
            std::string name = prop.name;
            if (prop.name.empty())
                name = "_";
            out << "[label=\"" << prop.level << " " << prop.name << "\", shape=\""
                << ((final) ? "doublecircle" : "circle") << "\"]";
        }

        void write_v1ca(std::ostream &out, const V1CA::vertex_descriptor_t &v) {
            auto &v1ca = dynamic_cast<V1CA &>(to_display_);
            auto &g = v1ca.get_mutable_graph();
            const auto prop = g[v];
            auto final = v1ca.is_final(prop);
            std::string name = prop.name;
            if (prop.name.empty())
                name = "Entry point";
            out << "[label=\"" << prop.cv << " " << prop.name << "\", shape=\""
                << ((final) ? "doublecircle" : "circle") << "\"]";
        }


        template<class Vertex>
        void operator()(std::ostream &out, const Vertex &v) {

            if (to_display_.get_displayable_type() == displayable_type::V1CA) {
                write_v1ca(out, static_cast<V1CA::vertex_descriptor_t>(v));
            } else if (to_display_.get_displayable_type() == displayable_type::behaviour_graph) {
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
        edge_writer(displayable &display_obj, alphabet &alphabet) : to_display_(display_obj), alphabet_{alphabet} {}

        void write_behaviour_graph(std::ostream &out, const behaviour_graph::edge_descriptor_t &e) {

            auto &bg = dynamic_cast<behaviour_graph &>(to_display_);
            auto &graph = bg.get_mutable_graph();

            const auto prop = graph[e];
            out << "[label=\"";

            if (prop.effect == -1)
                out << "-";
            else if (prop.effect == 1)
                out << "+";

            out << prop.symbol << R"(", color="black"])";
        }

        void write_v1ca(std::ostream &out, const V1CA::edge_descriptor_t &e) {

            auto &v1ca = dynamic_cast<V1CA &>(to_display_);
            auto &alphabet = dynamic_cast<visibly_alphabet_t &>(alphabet_);

            V1CA::graph_t &g = v1ca.get_mutable_graph();
            auto e_pair = utils::make_pair_comp(boost::source(e, g), boost::target(e, g));
            const auto prop = g[e];
            out << "[label=\"";

            if (alphabet.get_cv(prop.symbol) == -1)
                out << "-";
            else if (alphabet.get_cv(prop.symbol) == 1)
                out << "+";

            std::string edge_color = "black";
            std::string loop_cond;

            if (v1ca.colored()) {
                if (v1ca.get_mutable_loop_in_no_cond_color().contains(e_pair)) {
                    edge_color = "gold4";
                } else if (v1ca.get_mutable_loop_in_with_cond_color().contains(e_pair)) {
                    edge_color = "red";
                    loop_cond = "if cv > " + std::to_string(v1ca.period_cv());
                } else if (v1ca.get_mutable_loop_out_color().contains(e_pair)) {
                    edge_color = "blue";
                    loop_cond = "if cv <= " + std::to_string(v1ca.period_cv());
                }
            }

            out << prop.symbol << " " << loop_cond << "\", color=\""
                << edge_color << "\"]";
        }

        template<class Edge>
        void operator()(std::ostream &out, const Edge &e) {
            if (to_display_.get_displayable_type() == displayable_type::V1CA) {
                write_v1ca(out, static_cast<V1CA::edge_descriptor_t>(e));
            } else if (to_display_.get_displayable_type() == displayable_type::behaviour_graph) {
                write_behaviour_graph(out, static_cast<behaviour_graph::edge_descriptor_t>(e));
            } else {
                throw std::invalid_argument("Cannot display edge for this type of object.");
            }
        }

    private:
        displayable &to_display_;
        alphabet &alphabet_;
    };
}
