#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
#include <readline/readline.h>

#include "cc_shell.hpp"

int main() {
    // Flush after every std::cout / std:cerr
    CC_Shell shell;
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;
    // Uncomment this block to pass the first stage
    while (true) {
        //std::cout << "$ ";
        std::string input;
        input = readline("$ ");
        //shell.get_non_can_input(input);
        if (!input.empty()) {
            shell.add_to_history(input);
        }
        if (input == "exit") {
            return 0;
        }
        int seperator{0};
        if (input.front() == '\'' || input.front() == '\"') {
            char c = input.front();
            seperator = input.find(c, 1) + 1;
        }
        else {
            seperator = input.find(" ");
        }
        std::string command = input.substr(0, seperator);
        if (command.front() == '\'' || command.front() == '\"') {
            command = "cat";
        }
        std::string command_args = input.substr(seperator + 1, input.length());
        shell.handle_command(command, command_args);
    }
}
