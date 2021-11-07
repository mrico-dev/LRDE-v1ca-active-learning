#include <iostream>
#include "automaton_teacher.h"
#include "language.h"
#include "behaviour_graph.h"

namespace active_learning {

    bool automaton_teacher::membership_query(const std::string &word) {
        return ref_.evaluate(word);
    }

    int automaton_teacher::count_query(const std::string &word) {
        return ref_.count(word);
    }

    std::optional<std::string> automaton_teacher::partial_equivalence_query(behaviour_graph &behaviour_graph, const std::string& path) {
        behaviour_graph.display(path);

        std::cout << "Please check the behaviour graph (graph name should be " << path
                  << ".png) and give a counter example, or enter 'OK' if the graph is good ";

        std::string user_input;
        do {
            std::cout << "(make sure you enter a word which counter value is 0) : ";
            std::cin >> user_input;

            if (user_input == "OK" or user_input == "ok" or user_input == "Ok" or user_input == "oK")
                return std::nullopt;

        } while (!is_from_alphabet(user_input, ref_.get_alphabet()) or not ref_.count(user_input));

        return user_input;
    }

    std::optional<std::string> automaton_teacher::equivalence_query(one_counter_automaton &automaton, const std::string& path) {
        auto &r1ca = oca_to_r1ca(automaton);
        r1ca.display(path);

        std::cout << "Please check the R1CA automaton (automaton name should be " << path
                  << ".png) and give a counter example, or 'OK' if the automaton is good ";

        std::string user_input;
        do {
            std::cout << "(make sure you enter a word which counter value is 0) : ";
            std::cin >> user_input;

            if (user_input == "OK" or user_input == "ok" or user_input == "Ok" or user_input == "oK")
                return std::nullopt;

            // input counter example must be from alphabet, and its cv must be 0
        } while (!is_from_alphabet(user_input, ref_.get_alphabet()) or not ref_.count(user_input));

        return user_input;
    }

    automaton_teacher::automaton_teacher(R1CA &ref) : ref_(ref) {}

    R1CA &automaton_teacher::oca_to_r1ca(one_counter_automaton &automaton) {
        auto r1ca_ptr = dynamic_cast<R1CA*>(&automaton);
        if (!r1ca_ptr)
            throw std::runtime_error("automaton_teacher must take a R1CA as argument");

        return *r1ca_ptr;
    }

    int automaton_teacher::get_cv(const std::string &word) {
        return count_query(word);
    }

}
