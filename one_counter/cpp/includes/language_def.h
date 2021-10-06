#pragma once

#include <set>
#include <map>

namespace active_learning {
    using alphabet_t = std::set<char>;
    using visibly_alphabet_t = std::map<char, int>;
}
