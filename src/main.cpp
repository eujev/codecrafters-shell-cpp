#include <cstddef>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

void function_type(std::string command_input);
void function_execute(std::string command, std::string command_input);
std::vector<std::string> split(std::string s, std::string delimeter);

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
            function_execute(command, command_input);
        }
    }
}

void function_type(std::string command_input)
{
    std::string path = std::getenv("PATH");
    std::vector<std::string> split_paths = split(path, ":");

    if (command_input == "echo" || command_input == "type" || command_input == "exit") {
        std::cout << command_input << " is a shell builtin\n";
        return;
    }
    for(auto it = split_paths.begin(); it != split_paths.end(); ++it) {
        for (const auto & entry : std::filesystem::directory_iterator(*it)) {
            if (command_input == entry.path().filename().string()) {
                std::cout << command_input << " is " << entry.path().string() << "\n";
                return;
            }
        }
    }
    std::cout << command_input << ": not found\n";
}

void function_execute(std::string command, std::string command_input)
{
    std::string path = std::getenv("PATH");
    std::vector<std::string> split_paths = split(path, ":");
    for(auto it = split_paths.begin(); it != split_paths.end(); ++it) {
        for (const auto & entry : std::filesystem::directory_iterator(*it)) {
            if (command == entry.path().filename().string()) {
                std::system((command + " " + command_input).c_str());
                return;
            }
        }
    }
    std::cout << command << ": command not found\n";
}
std::vector<std::string> split(std::string s, std::string delimeter)
{
    std::vector<std::string> split_strings;
    size_t pos = 0;
    std::string token;
    while((pos = s.find(delimeter)) != std::string::npos) {
        token = s.substr(0, pos);
        split_strings.push_back(token);
        s.erase(0, pos + delimeter.length());
    }
    split_strings.push_back(s);

    return split_strings;
}
