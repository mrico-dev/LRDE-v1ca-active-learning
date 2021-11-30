#pragma once

#include <string>
#include <map>
#include <set>
#include <vector>

#include "teacher.h"
#include "alphabet.h"
#include "dataframe.h"

namespace active_learning {

    bool
    is_O_equivalent(const std::string &word1, const std::string &word2, RST &rst, word_counter &wc, teacher &teacher);

    std::set<std::string> get_congruence_set(const std::string &word, RST &rst, word_counter &wc, teacher &teacher);

    bool is_from_alphabet(const std::string &word, const alphabet &alphabet);
}

// V1C2AL_LANGUAGE_H
