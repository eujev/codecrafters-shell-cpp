#include <iostream>

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
        if (command == "echo") {
            std::cout << input.substr(input.find(" ") + 1, input.length()) << "\n";
        }
        else {
            std::cout << input << ": command not found\n";
        }
    }
}
