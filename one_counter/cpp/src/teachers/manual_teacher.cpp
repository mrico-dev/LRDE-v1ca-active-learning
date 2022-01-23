#include "teachers/manual_teacher.h"

namespace active_learning {

    bool manual_teacher::membership_query_(const std::string &word) {

        std::string res;
        while (res != "y" and res != "n") {
            std::cout << "Is the word '" << word << "' accepted by the language ? [y/n]: ";
            std::cin >> res;
        }

        return res == "y";
    }

    manual_teacher::manual_teacher(visibly_alphabet_t &alphabet) {
        alphabet_ = alphabet;
    }

}