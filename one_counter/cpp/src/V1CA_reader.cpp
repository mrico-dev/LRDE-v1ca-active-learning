#include <fstream>
#include "V1CA_reader.h"

namespace active_learning {

    std::vector<std::string> get_all_lines(const std::string &path) {

        // Opening file
        auto in_file = std::ifstream(path);
        if (not in_file.is_open())
            throw std::runtime_error("Could not open file '" + path + "'.");

        // Recovering all lines (bad practice)
        auto lines = std::vector<std::string>();
        auto curr_line = std::string();

        while (std::getline(in_file, curr_line)) {
            lines.emplace_back(curr_line);
        }

        in_file.close();
        return lines;
    }

    V1CA read_v1ca_from_file(const std::string &path, const visibly_alphabet_t &alphabet) {

        auto lines = get_all_lines(path);

        // TODO

        return V1CA(alphabet);
    }

}
