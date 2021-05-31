#ifndef V1C2AL_MANUAL_EQ_QUERIES_TEACHER_H
#define V1C2AL_MANUAL_EQ_QUERIES_TEACHER_H

#include <iostream>
#include "teacher.h"

namespace active_learning {

    class manual_eq_queries_teacher : public cached_teacher {

    public:

        std::optional<std::string> partial_equivalence_query(V1CA &behaviour_graph, const std::string &path) override {

            behaviour_graph.display(path);

            std::cout << "Please check the behaviour graph (graph name should be " << path
                      << ".pdf) and give a counter example (or 'OK' if the graph is good): ";

            std::string user_input;
            std::cin >> user_input;

            if (user_input == "OK" or user_input == "ok" or user_input == "Ok" or user_input == "oK")
                return std::nullopt;

            return user_input;
        }

        std::optional<std::string> equivalence_query(V1CA &automaton, const std::string &path) override {

            automaton.display(path);

            std::cout << "Please check the V1CA automaton (automaton name should be " << path
                      << ".pdf) and give a counter example (or 'OK' if the automaton is good): ";

            std::string user_input;
            std::cin >> user_input;

            if (user_input == "OK" or user_input == "ok" or user_input == "Ok" or user_input == "oK")
                return std::nullopt;

            return user_input;
        }

    };
}


#endif //V1C2AL_MANUAL_EQ_QUERIES_TEACHER_H
