#include <iostream>

int main() {
    // Flush after every std::cout / std:cerr
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;
    int exit = 1;
    // Uncomment this block to pass the first stage
    while(exit != 0) {
        std::cout << "$ ";
        std::string input;
        std::getline(std::cin, input);
        if (input == "exit 0") {
            exit = 0;
        }
        else {
            std::cout << input << ": command not found\n";
        }
    }
}
