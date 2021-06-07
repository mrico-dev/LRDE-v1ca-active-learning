#include "language.h"

#include <iostream>

namespace active_learning {

    int get_cv(const std::string &word, alphabet_t &alphabet) {

        int res = 0;
        for (char c : word) {
            res += alphabet[c];
        }

        return res;
    }

    bool is_O_equivalent(const std::string &word1, const std::string &word2, RST &rst, teacher &teacher, alphabet_t &alphabet) {
        if (word1 == word2)
            return true;

        int cv_w = get_cv(word1, alphabet);
        if (cv_w != get_cv(word2, alphabet))
            return false;

        auto rst_copy = RST(rst);
        rst_copy.add_row_using_query_if_not_present(word1, cv_w, teacher, "is_O_equivalent");
        rst_copy.add_row_using_query_if_not_present(word2, cv_w, teacher, "is_O_equivalent");

        return rst_copy.compare_rows(word1, word2, cv_w);
    }

    std::set<std::string> get_congruence_set(const std::string &word, RST &rst, teacher &teacher, alphabet_t alphabet) {
        int cv_w = get_cv(word, alphabet);

        if (cv_w > static_cast<int>(rst.size())) {
            throw std::runtime_error("get_congruence_set(): given cv is out of bound of RST.");
        }

        auto res = std::set<std::string>();
        for (auto &table : rst.get_tables()) {
            for (auto &label : table.get_row_labels()) {
                if (is_O_equivalent(word, label, rst, teacher, alphabet)) {
                    res.insert(label);
                }
            }
        }

        return res;
    }

    std::vector<std::string> get_all_prefixes(const std::string &word) {
        std::vector<std::string> res({""});
        std::string pref;
        for (char c : word) {
           pref.push_back(c);
           res.emplace_back(pref);
        }

        return res;
    }

    bool is_from_alphabet(const std::string &word, const alphabet_t &alphabet) {
        for (const char &c : word) {
            if (!alphabet.count(c)) {
                return false;
            }
        }

        return true;
    }
}
