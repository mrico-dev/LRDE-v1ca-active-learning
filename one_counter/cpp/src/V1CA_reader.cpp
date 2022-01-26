#include <fstream>
#include "V1CA_reader.h"

namespace active_learning {

    static std::vector<std::string> get_all_lines(const std::string &path) {

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

    static std::vector<std::pair<V1CA::transition_x, V1CA::transition_y>>
    read_transition(const std::string &line) {
        auto i = 0u;
        while (i < line.size() and isdigit(line[i]))
            ++i;

        auto from_str = line.substr(0, i);

        if (i + 1 >= line.size()
            or line[i] != '-'
            or line[i + 1] != '>')
            throw std::runtime_error("Could not parse transition: missing '->'. Line: " + line);

        i += 2;

        auto j = i;
        while (i < line.size() and isdigit(line[i]))
            ++i;

        auto to_str = line.substr(j, i - j);

        if (i >= line.size() or line[i] != ' ')
            throw std::runtime_error("Could not parse transition: missing space after x->x. Line: " + line);
        ++i;
        if (i >= line.size())
            throw std::runtime_error("Could not parse transition: missing symbol. Line: " + line);

        auto symbol = line[i];

        ++i;
        if (i >= line.size())
            throw std::runtime_error("Could not parse transition: missing space after symbol. Line: " + line);
        ++i;

        j = i;
        while (i < line.size() and isdigit(line[i]))
            ++i;

        auto counter1_str = line.substr(j, i - j);
        size_t from = 424242, to = 424242, counter1 = 424242;
        try {
            std::stringstream(from_str) >> from;
            std::stringstream(to_str) >> to;
            std::stringstream(counter1_str) >> counter1;
        } catch (std::exception &e) {
            throw std::runtime_error("Could not parse transition: Error while reading a number (probably empty number). Line: " + line);
        }

        auto res = std::vector<std::pair<V1CA::transition_x, V1CA::transition_y>>();
        if (i >= line.size() or line[i] != '-') {
            res.emplace_back(std::make_pair<V1CA::transition_x, V1CA::transition_y>({from, counter1, symbol}, {to, V1CA::transition_color::init}));
            return res;
        }

        if (i >= line.size() or line[i] != '-')
            throw std::runtime_error("Could not parse transition: missing '-' after first counter value. Line: " + line);
        i++;
        j = i;

        j = i;
        while (i < line.size() and isdigit(line[i]))
            ++i;

        auto counter2_str = line.substr(j, i - j);
        size_t counter2 = 424242;
        try {
            std::stringstream(counter2_str) >> counter2;
        } catch (std::exception &e) {
            throw std::runtime_error("Could not parse transition: Error while reading a number (probably empty number). Line: " + line);
        }

        for (auto cv = counter1; i <= counter2; ++i) {
            res.emplace_back(std::make_pair<V1CA::transition_x, V1CA::transition_y>({from, cv, symbol}, {to, V1CA::transition_color::init}));
        }

        return res;
    }

    V1CA read_v1ca_from_file(const std::string &path, const visibly_alphabet_t &alphabet) {
        auto lines = get_all_lines(path);
        if (lines.size() < 2)
            throw std::runtime_error("Could not parse V1CA: Not enough lines");

        auto &state_n_line = lines[0];
        auto &max_lvl_line = lines[1];

        size_t max_lvl = 424242, state_n = 424242;
        try {
            std::stringstream(state_n_line) >> state_n;
            std::stringstream(max_lvl_line) >> max_lvl;
        } catch (std::exception &e) {
            throw std::runtime_error("Could not parse V1CA: Could not parse state_n or max_level");
        }

        auto res = V1CA(alphabet);
        res.states_n_ = state_n;
        res.max_level_ = max_lvl;

        // FIXME add actual states props in file
        for (auto i = 0u; i < state_n; ++i)
            res.state_props_.insert({i, {0, "x"}});

        for (const auto &line : lines) {
            if (not line.empty()) {
                for (auto &trans : read_transition(line))
                res.transitions_.insert(trans);
            }
        }

        return res;
    }

}
