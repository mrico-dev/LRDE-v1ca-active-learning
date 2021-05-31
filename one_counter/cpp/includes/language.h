#ifndef V1C2AL_LANGUAGE_H
#define V1C2AL_LANGUAGE_H

#include <string>
#include <map>
#include "teacher.h"
//#include "dataframe.h"

namespace active_learning {

    using alphabet_t = std::map<char, int>;

    int get_cv(std::string word, alphabet_t alphabet);

    bool is_O_equivalent(const std::string &word1, const std::string &word2, RST &rst, teacher &teacher, alphabet_t &alphabet);

    std::set<std::string> get_congruence_set(const std::string word, RST &rst, teacher &teacher, alphabet_t alphabet);

    std::vector<std::string> get_all_prefixes(const std::string &word);
}

#endif // V1C2AL_LANGUAGE_H
