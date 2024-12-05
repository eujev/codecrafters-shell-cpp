#include <iostream>

void function_type(std::string command_input);

int main() {
    // Flush after every std::cout / std:cerr
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;
    // Uncomment this block to pass the first stage
    while(true) {
        std::cout << "$ ";
        std::string input;
        std::getline(std::cin, input);
        if (input == "exit 0") {
            return 0;
        }
        std::string command = input.substr(0, input.find(" "));
        std::string command_input = input.substr(input.find(" ") + 1, input.length()); 
        if (command == "echo") {
            std::cout << command_input << "\n";
        }
        else if (command == "type") {
            function_type(command_input);
        }
        else {
            std::cout << input << ": command not found\n";
        }
    }
}

void function_type(std::string command_input)
{
    if (command_input == "echo" || command_input == "type" || command_input == "exit") {
        std::cout << command_input << " is a shell builtin\n";
    }
    else {
        std::cout << command_input << ": not found\n";
    }
}

