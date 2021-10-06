#include "automaton_teacher.h"

namespace active_learning {

    bool automaton_teacher::membership_query(const std::string &word) {
        return ref_.evaluate(word).second;
    }

    int automaton_teacher::count_query(const std::string &word) {
        return ref_.evaluate(word).first;
    }

    std::optional<std::string> automaton_teacher::partial_equivalence_query(const R1CA &behaviour_graph, const std::string& path) {

        behaviour_graph.display(path); // TODO

        std::cout << "Please check the behaviour graph (graph name should be " << path
                  << ".png) and give a counter example, or enter 'OK' if the graph is good ";

        std::string user_input;
        do {
            std::cout << "(make sure you enter a word which counter value is 0) : ";
            std::cin >> user_input;

            if (user_input == "OK" or user_input == "ok" or user_input == "Ok" or user_input == "oK")
                return std::nullopt;

        } while (!is_from_alphabet(user_input, ref_.get_alphabet()) or !ref_.evaluate(user_input).second);

        return user_input;
    }

    std::optional<std::string> automaton_teacher::equivalence_query(const R1CA &automaton, const std::string& path) {
        automaton.display(path); // TODO

        std::cout << "Please check the R1CA automaton (automaton name should be " << path
                  << ".png) and give a counter example, or 'OK' if the automaton is good ";

        std::string user_input;
        do {
            std::cout << "(make sure you enter a word which counter value is 0) : ";
            std::cin >> user_input;

            if (user_input == "OK" or user_input == "ok" or user_input == "Ok" or user_input == "oK")
                return std::nullopt;

            // input counter example must be from alphabet, and its cv must be 0
        } while (!is_from_alphabet(user_input, ref_.get_alphabet()) or !ref_.evaluate(user_input).second);

        return user_input;
    }

    automaton_teacher::automaton_teacher(R1CA &ref) : ref_(ref) {}
}
