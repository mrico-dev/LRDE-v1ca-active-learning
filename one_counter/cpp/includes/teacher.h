#pragma once

#include <string>
#include <optional>
#include <map>

#include "dataframe.h"
#include "V1CA.h"

namespace active_learning {

    class teacher {

    public:
        virtual bool membership_query(const std::string &word) = 0;

        virtual std::optional<std::string>
        partial_equivalence_query(V1CA &behaviour_graph, const std::string &path) = 0;

        virtual std::string sum_up_msg() const;

        virtual std::optional<std::string> equivalence_query(V1CA &automaton, const std::string &path) = 0;
    };

    class cached_teacher : public teacher {

    protected:
        virtual bool belong_query_(const std::string &word) = 0;

    public:
        std::string sum_up_msg() const override;
        bool membership_query(const std::string &word) override;

    private:
        std::map<std::string, bool> query_cache_;
    };
}

//V1C2AL_TEACHER_H
