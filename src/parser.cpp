#include "parser.hpp"

#include <algorithm>

std::string Parser::get_longest_common_prefix(const std::vector<std::string> &commands) {
    if (commands.empty()) {
        return "";
    }
    auto string_min = *std::min_element(commands.begin(), commands.end(), [] (const std::string& s1, const std::string& s2) {return s1.length() < s2.length();} );
    std::string prefix;
    for (size_t i = 0; i < string_min.length(); ++i) {
        char c = commands[0][i];
        for (auto command : commands) {
            if (command[i] != c) {
                return prefix;
            }
        }
        prefix.push_back(c);
    }
    return prefix;
}


std::string Parser::check_quotes(std::string command_args)
{
    std::string s = command_args;
    std::string result{};
    auto it = s.begin();
    while (it != s.end()) {
        if (*it == '\'') {
            ++it;
            while (*it != '\'' && (it+1) != s.end()) {
                result += *it;
                ++it;
            }
            ++it;
        }
        else if (*it == '\"') {
            ++it;
            while (*it != '\"' && (it+1) != s.end()) {
                if (*it == '\\') {
                    if (*(it+1) == '\\' || *(it+1) == '$') {
                        ++it;
                    }
                    else if (*(it+1) == '\"') {
                        result += *++it;
                        ++it;
                    }
                }
                result += *it;
                ++it;
            }
            ++it;
        }
        else if (*it == ' ') {
            while (*(it+1) == ' ') {
            ++it;
            }
            result += *it;
            it++;
        }
        else if (*it == '\\') {
            result += *++it;
            ++it;
        }
        else {
            result += *it;
            ++it;
        }
    }
    return result;
}
