#pragma once

#include "V1CA.h"
#include "R1CA.h"

namespace active_learning {

    // Used for graphviz dot vertex output
    template<class automaton_t>
    class vertex_writer {
        static_assert(std::is_base_of<one_counter_automaton, automaton_t>::value,
                      "automaton_t must derive from one_counter_automaton");

    public:
        explicit vertex_writer(automaton_t automaton) : automaton_(automaton) {}

        void write_v1ca(std::ostream &out, const V1CA::vertex_descriptor_t &v) {
            V1CA &v1ca = *((V1CA*)&automaton_);
            auto &g = v1ca.get_mutable_graph();
            const auto prop = g[v];
            auto final = v1ca.is_final(prop);
            std::string name = prop.name;
            if (prop.name.empty())
                name = "Entry point";
            out << "[label=\"" << prop.cv << " " << prop.name << "\", shape=\""
                << ((final) ? "doublecircle" : "circle") << "\"]";
        }

        void write_r1ca(std::ostream &out, const R1CA::vertex_descriptor_t &v) {
            auto &r1ca = *((R1CA*)(&automaton_));
            auto &g = r1ca.get_mutable_graph();
            const auto prop = g[v];
            auto final = r1ca.is_final(prop);
            std::string name = prop.name;
            if (prop.name.empty())
                name = "Entry point";
            out << "[label=\"" << prop.name << "\", shape=\""
                << ((final) ? "doublecircle" : "circle") << "\"]";
        }

        template<class Vertex>
        void operator()(std::ostream &out, const Vertex &v) {
            const auto &oc_automaton = static_cast<one_counter_automaton>(automaton_); // TODO static cast ?
            if (oc_automaton.get_automaton_type() == automaton_type::V1CA) {
                write_v1ca(out, static_cast<V1CA::vertex_descriptor_t>(v));
            } else if (oc_automaton.get_automaton_type() == automaton_type::R1CA) {
                write_r1ca(out, static_cast<R1CA::vertex_descriptor_t>(v));
            } else {
                throw std::invalid_argument("Cannot display edge for this type of automaton.");
            }
        }

    private:
        automaton_t automaton_;
    };

// Used for graphviz dot edge output
    template<class automaton_t, class automaton_alphabet_t>
    class edge_writer {
        static_assert(std::is_base_of<one_counter_automaton, automaton_t>::value,
                      "automaton_t must derive from one_counter_automaton");

    public:
        edge_writer(automaton_t automaton, automaton_alphabet_t alphabet) : automaton_(automaton), alphabet_{alphabet} {}

        void write_v1ca(std::ostream &out, const V1CA::edge_descriptor_t &e) {
            V1CA &v1ca = *((V1CA*)&automaton_);
            visibly_alphabet_t &alphabet = *((visibly_alphabet_t*)&alphabet_);

            V1CA::graph_t &g = v1ca.get_mutable_graph();
            auto e_pair = make_pair_comp(boost::source(e, g), boost::target(e, g));
            const auto prop = g[e];
            out << "[label=\"";

            if (alphabet[prop.symbol] == -1)
                out << "-";
            else if (alphabet[prop.symbol] == 1)
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

        void write_r1ca(std::ostream &out, const R1CA::edge_descriptor_t &e) {
            R1CA &r1ca = *((R1CA*)&automaton_);

            R1CA::graph_t &g = r1ca.get_mutable_graph();
            const auto prop = g[e];
            out << "[label=\"";

            std::string edge_color = "black";
            std::string loop_cond;

            if (prop.counter_change >= 0) {
                loop_cond += "+";
            }
            loop_cond += std::to_string(prop.counter_change);

            if (prop.cond) {
                loop_cond += ", ";
                if (prop.cond_infeq) {
                    edge_color = "red";
                    loop_cond = "if cv > " + std::to_string(prop.cond_val);
                } else {
                    edge_color = "blue";
                    loop_cond = "if cv <= " + std::to_string(prop.cond_val);
                }
            }

            out << prop.symbol << " " << loop_cond << "\", color=\""
                << edge_color << "\"]";
        }

        template<class Edge>
        void operator()(std::ostream &out, const Edge &e) {
            const auto &oc_automaton = static_cast<one_counter_automaton>(automaton_);
            if (oc_automaton.get_automaton_type() == automaton_type::V1CA) {
                write_v1ca(out, static_cast<V1CA::edge_descriptor_t >(e));
            } else if (oc_automaton.get_automaton_type() == automaton_type::R1CA) {
                write_r1ca(out, static_cast<R1CA::edge_descriptor_t >(e));
            } else {
                throw std::invalid_argument("Cannot display edge for this type of automaton.");
            }
        }

    private:
        automaton_t automaton_;
        automaton_alphabet_t alphabet_;
    };
}
