#pragma once

#include "teacher.h"
#include "R1CA.h"
#include "word_counter.h"

namespace active_learning {

    class automaton_teacher : public teacher, public word_counter {

    public:
        explicit automaton_teacher(R1CA &ref);

        bool membership_query(const std::string &word) override;

        int count_query(const std::string &word);

        std::optional<std::string>
        partial_equivalence_query(behaviour_graph &behaviour_graph, const std::string &path) override;

        std::optional<std::string>
        equivalence_query(one_counter_automaton &automaton, const std::string &path) override;

    private:
        int get_cv(const std::string &word) override;

        static R1CA &oca_to_r1ca(one_counter_automaton &automaton);

    private:
        R1CA &ref_;
    };

}