#pragma once

#include <string>
#include <map>
#include <set>
#include <vector>

#include "teacher.h"
#include "language_def.h"

namespace active_learning {
    class RST;
}

namespace active_learning {

    int get_cv(const std::string &word, visibly_alphabet_t &alphabet);

    bool is_O_equivalent(const std::string &word1, const std::string &word2, RST &rst, teacher &teacher,
                         visibly_alphabet_t &alphabet);

    std::set<std::string> get_congruence_set(const std::string &word, RST &rst, teacher &teacher, visibly_alphabet_t alphabet);

    std::vector<std::string> get_all_prefixes(const std::string &word);

    bool is_from_alphabet(const std::string &word, const visibly_alphabet_t &alphabet);
    bool is_from_alphabet(const std::string &word, const alphabet_t &alphabet);
}

// V1C2AL_LANGUAGE_H
