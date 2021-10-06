#include "teacher.h"

#include <iostream>

namespace active_learning {

    bool cached_teacher::membership_query(const std::string &word) {
        if (query_cache_.contains(word)) {
            return query_cache_[word];
        }

        auto res = belong_query_(word);
        query_cache_[word] = res;
        return res;
    }

    std::string cached_teacher::sum_up_msg() const {
        return "Learning took " + std::to_string(query_cache_.size()) + " membership queries.";
    }

    std::string teacher::sum_up_msg() const {
        return std::string();
    }
}
