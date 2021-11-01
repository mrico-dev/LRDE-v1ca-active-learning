#pragma once

#include "teacher.h"
#include "V1CA.h"

namespace active_learning {

    class automatic_teacher : public cached_teacher {
    public:

        automatic_teacher(std::function<bool(const std::string &)> checkFunc, behaviour_graph &behaviourRef, V1CA &automatonRef,
                          visibly_alphabet_t alphabet);

        std::optional<std::string> partial_equivalence_query(behaviour_graph &behaviour_graph, const std::string &path) override;

        std::optional<std::string> equivalence_query(one_counter_automaton &automaton, const std::string &path) override;

    protected:
        bool membership_query_(const std::string &word) override;

    private:
        std::string find_counter_example(V1CA& automaton);

        static V1CA &oca_to_v1ca(one_counter_automaton& v1ca);

    private:
        std::function<bool(const std::string &)> check_func_;
        behaviour_graph& behaviour_ref_;
        V1CA& automaton_ref_;
        visibly_alphabet_t alphabet_;
    };

}

