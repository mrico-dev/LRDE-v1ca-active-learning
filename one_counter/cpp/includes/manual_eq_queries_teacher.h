#pragma once

#include <iostream>
#include "language.h"
#include "V1CA.h"
#include "behaviour_graph.h"

namespace active_learning {

    class manual_eq_queries_teacher : public cached_teacher {

    protected:

        static V1CA &oca_to_v1ca(one_counter_automaton &automaton) {
            auto v1ca_ptr = dynamic_cast<V1CA *>(&automaton);
            if (!v1ca_ptr)
                throw std::runtime_error("teacher must take a V1CA as argument");

            return *v1ca_ptr;
        }

    public:

        std::optional<std::string>
        partial_equivalence_query(behaviour_graph &behaviour_graph, const std::string &path) override {
            behaviour_graph.display(path);

            std::cout << "Please check the behaviour graph (graph name should be " << path
                      << ".png) and give a counter example, or enter 'OK' if the graph is good ";

            std::string user_input;
            do {
                std::cout << "(make sure you enter a word which counter value is 0) : ";
                std::cin >> user_input;

                if (user_input == "OK" or user_input == "ok" or user_input == "Ok" or user_input == "oK")
                    return std::nullopt;

            } while (!is_from_alphabet(user_input, alphabet_) or alphabet_.get_cv(user_input));

            return user_input;
        }

        std::optional<std::string>
        equivalence_query(one_counter_automaton &automaton, const std::string &path) override {
            automaton.display(path);

            std::cout << "Please check the V1CA automaton (automaton name should be " << path
                      << ".png) and give a counter example, or 'OK' if the automaton is good ";

            std::string user_input;
            do {
                std::cout << "(make sure you enter a word which counter value is 0) : ";
                std::cin >> user_input;

                if (user_input == "OK" or user_input == "ok" or user_input == "Ok" or user_input == "oK")
                    return std::nullopt;

                // input counter example must be from alphabet, and its cv must be 0
            } while (!is_from_alphabet(user_input, alphabet_) or alphabet_.get_cv(user_input));

            return user_input;
        }

    protected:
        visibly_alphabet_t alphabet_;
    };
}


//V1C2AL_MANUAL_EQ_QUERIES_TEACHER_H
