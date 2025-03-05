#include <cstddef>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

void handle_command(std::string command, std::string command_input);
void function_type(std::string command_input);
void function_execute(std::string command, std::string command_input);
void function_pwd();
void function_cd(std::string command_input);
std::string check_quotes(std::string command_input);
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
        std::string command_input = input.substr(seperator + 1, input.length()); 
        handle_command(command, command_input);
    }
}


void handle_command(std::string command, std::string command_input)
{
    if (command == "echo") {
        command_input = check_quotes(command_input);
        std::cout << command_input << "\n";
    }
    else if (command == "type") {
        function_type(command_input);
    }
    else if (command == "pwd") {
        function_pwd();
    }
    else if (command == "cd") {
        function_cd(command_input);
    }
    else {
        function_execute(command, command_input);
    }
}


void function_type(std::string command_input)
{
    if (command_input == "echo" || command_input == "type" || command_input == "exit" || command_input == "pwd") {
        std::cout << command_input << " is a shell builtin\n";
        return;
    }

    std::string path = std::getenv("PATH");
    std::vector<std::string> split_paths = split(path, ":");
    
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


void function_pwd()
{
    std::cout << std::filesystem::current_path().string() << "\n";
}


void function_cd(std::string command_input)
{
    const std::filesystem::path destination_path = command_input;
    if (destination_path.string() == "~") {
        std::string home_dir = std::getenv("HOME");
        std::filesystem::current_path(home_dir);
    }
    else if (std::filesystem::exists(destination_path)) {
        std::filesystem::current_path(destination_path);
    }
    else {
        std::cout << "cd: " << destination_path.string() << ": No such file or directory\n";
    }
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

std::string check_quotes(std::string command_input)
{
    std::string s = command_input;
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
