#ifndef V1C2AL_TEACHER_H
#define V1C2AL_TEACHER_H

#include <string>

#include "V1CA.h"

namespace active_learning {

    class teacher {

    public:
        virtual bool belong_query(const std::string &word) = 0;

        virtual std::optional<std::string> partial_equivalence_query(V1CA &behaviour_graph, const std::string &path) = 0;

        virtual std::optional<std::string> equivalence_query(V1CA &automaton, const std::string &path) = 0;
    };

    class cached_teacher : public teacher {

    protected:
        virtual bool belong_query_(const std::string &word) = 0;

    public:
        bool belong_query(const std::string &word) override;

    private:
        std::map<std::string, bool> query_cache_;
    };
}

#endif //V1C2AL_TEACHER_H
