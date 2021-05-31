#include "language.h"

namespace active_learning {

    int get_cv(std::string word, alphabet_t alphabet) {

        int res = 0;
        for (char c : word) {
            res += alphabet[c];
        }

        return res;
    }

    bool is_O_equivalent(const std::string &word1, const std::string &word2, RST &rst, teacher &teacher, alphabet_t &alphabet) {
        int cv_w = get_cv(word1, alphabet);
        if (cv_w != get_cv(word2, alphabet))
            return false;

        auto rst_copy = RST(rst);
        rst_copy.add_row_using_query(word1, cv_w, teacher);
        rst_copy.add_row_using_query(word2, cv_w, teacher);

        return rst_copy.compare_rows(word1, word2, cv_w);
    }

    std::set<std::string> get_congruence_set(const std::string word, RST &rst, teacher &teacher, alphabet_t alphabet) {
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
        std::vector<std::string> res;
        auto pref = "";
        for (char c : word) {
           pref += c;
           res.emplace_back(pref);
        }

        return res;
    }
}
