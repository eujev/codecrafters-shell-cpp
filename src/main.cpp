#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>

#include "cc_shell.hpp"

std::string check_quotes(std::string command_args);
std::string get_longest_common_prefix(const std::vector<std::string> &commands);


int main() {
    // Flush after every std::cout / std:cerr
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;
    // Uncomment this block to pass the first stage
    while(true) {
        std::cout << "$ ";
        std::string input;
        get_non_can_input(input);
        if (input == "exit 0") {
            return 0;
        }
        int seperator{0};
        if (input.front() == '\'' || input.front() == '\"') {
            char c = input.front();
            seperator = input.find(c, 1)+1;
        }
        else {
            seperator = input.find(" ");
        }
        std::string command = input.substr(0, seperator);
        if (command.front() == '\'' || command.front() == '\"') {
            command = "cat";
        }
        std::string command_args = input.substr(seperator + 1, input.length()); 
        handle_command(command, command_args);
    }
}


std::string get_longest_common_prefix(const std::vector<std::string> &commands) {
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


std::string check_quotes(std::string command_args)
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
