#include "dot_writers.h"

void active_learning::writer::write_v1ca(std::ostream &out, active_learning::V1CA &automaton) {

    out << "Digraph G {\n";

    // Printing states
    for (auto st=0ul; st < automaton.states_n_; ++st) {
        auto prop = automaton.state_props_[st];
        out << st
            << "[label=\""
            << prop.level
            << "-"
            << prop.name
            << "\", shape=\""
            << ((automaton.final_states_.contains(st)) ? "doublecircle" : "circle")
            << "\"];\n";
    }

    // Printing transitions
    for (auto trans : automaton.transitions_) {

        auto sign = "";
        auto symbol_cv = automaton.alphabet_.get_cv(trans.first.symbol);
        if (symbol_cv < 0)
            sign = "-";
        else if (symbol_cv > 0)
            sign = "+";

        // Matching color with loop type
        auto color = (trans.second.color == V1CA::transition_color::loop_in_bottom) ? "red" :
                     (trans.second.color == V1CA::transition_color::loop_in_top) ? "gold4" :
                     (trans.second.color == V1CA::transition_color::loop_out) ? "blue" :
                     "black";

        out << trans.first.state
            << "->"
            << trans.second.state
            << " [label=\""
            << sign
            << trans.first.symbol
            << " "
            << trans.first.counter
            << "\", color=\""
            << color
            << "\"];\n";
    }

    out << "}\n";
}

active_learning::vertex_writer::vertex_writer(active_learning::displayable &display_obj) : to_display_(display_obj) {}

void active_learning::vertex_writer::write_behaviour_graph(std::ostream &out,
                                                           const active_learning::behaviour_graph::vertex_descriptor_t &v) {
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

void active_learning::edge_writer::write_behaviour_graph(std::ostream &out,
                                                         const active_learning::behaviour_graph::edge_descriptor_t &e) {

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
