#pragma once

#include <iostream>
#include <vector>
namespace Parser {
std::string check_quotes(std::string command_args);
std::string get_longest_common_prefix(const std::vector<std::string> &commands);
} // namespace Parser
