#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
#include <readline/history.h>
#include <readline/readline.h>

#include "cc_shell.hpp"

int main() {
    // Flush after every std::cout / std:cerr
    CC_Shell shell;
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;

    rl_attempted_completion_function = CC_Shell::custom_completer;
    // Uncomment this block to pass the first stage
    while (true) {
        // NOTE: Old style without the readline library
        //std::cout << "$ ";
        //shell.get_non_can_input(input);

        char *buffer = readline("$ ");
        if (buffer == nullptr) {
            std::cout << "\n";
            break;
        }

        std::string input(buffer);
        free(buffer);
        if (!input.empty()) {
            // shell.add_to_history(input);
            add_history(input.c_str());
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
