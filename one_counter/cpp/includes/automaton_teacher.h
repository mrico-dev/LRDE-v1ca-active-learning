#pragma once

#include "teacher.h"
#include "R1CA.h"

namespace active_learning {

    class automaton_teacher {

    public:
        explicit automaton_teacher(R1CA &ref);
        bool membership_query(const std::string &word);
        int count_query(const std::string &word);
        std::optional<std::string> partial_equivalence_query(const R1CA &automaton, const std::string &path);
        std::optional<std::string> equivalence_query(const R1CA &automaton, const std::string &path);

    private:
        R1CA &ref_;
    };

}