#include "teacher.h"

namespace active_learning {

    bool cached_teacher::belong_query(const std::string &word) {
        if (query_cache_.contains(word)) {
            return query_cache_[word];
        }

        return belong_query_(word);
    }
}
