#pragma once

#include <string>

namespace active_learning {
    class word_counter {
    public:
        virtual int get_cv(const std::string &word) const = 0;
    };
}