#pragma once

#include "teacher.h"

namespace active_learning {

    class automatic_teacher : public cached_teacher {
    public:

        automatic_teacher(std::function<bool(const std::string &)> checkFunc, V1CA &behaviourRef, V1CA &automatonRef,
                          visibly_alphabet_t alphabet);

        std::optional<std::string> partial_equivalence_query(V1CA &behaviour_graph, const std::string &path) override;

        std::optional<std::string> equivalence_query(V1CA &automaton, const std::string &path) override;

    protected:
        bool belong_query_(const std::string &word) override;

    private:
        std::string find_counter_example(V1CA& automaton);

    private:
        std::function<bool(const std::string &)> check_func_;
        V1CA& behaviour_ref_;
        V1CA& automaton_ref_;
        visibly_alphabet_t alphabet_;
    };

}

