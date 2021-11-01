#pragma once

#include <string>
#include <optional>
#include <map>

#include "one_counter_automaton.h"

namespace active_learning {

    class behaviour_graph;

    class teacher {

    public:
        virtual bool membership_query(const std::string &word) = 0;

        virtual std::optional<std::string>
        partial_equivalence_query(behaviour_graph &behaviour_graph, const std::string &path) = 0;

        virtual std::string sum_up_msg() const;

        virtual std::optional<std::string> equivalence_query(one_counter_automaton &automaton, const std::string &path) = 0;
    };

    class cached_teacher : public teacher {

    protected:
        virtual bool membership_query_(const std::string &word) = 0;

    public:
        [[nodiscard]] std::string sum_up_msg() const override;
        bool membership_query(const std::string &word) override;

    private:
        std::map<std::string, bool> query_cache_;
    };
}

//V1C2AL_TEACHER_H
