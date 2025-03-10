#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

void handle_command(std::string command, std::string command_args);
void function_type(std::string command_args);
void function_execute(std::string command, std::string command_args);
void function_pwd();
void function_cd(std::string command_args);
std::string get_path(std::string command);
std::string check_quotes(std::string command_args);


std::vector<std::string> builtin_commands{"echo", "type", "exit", "pwd", "cd"};

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
        std::string command_args = input.substr(seperator + 1, input.length()); 
        handle_command(command, command_args);
    }
}


void handle_command(std::string command, std::string command_args)
{
    if (command == "echo") {
        command_args = check_quotes(command_args);
        std::cout << command_args << "\n";
    }
    else if (command == "type") {
        function_type(command_args);
    }
    else if (command == "pwd") {
        function_pwd();
    }
    else if (command == "cd") {
        function_cd(command_args);
    }
    else {
        function_execute(command, command_args);
    }
}


void function_type(std::string command_args)
{
    if (std::find(builtin_commands.begin(), builtin_commands.end(), command_args) != builtin_commands.end()) {
        std::cout << command_args << " is a shell builtin\n";
        return;
    }

    std::string path = get_path(command_args);
    
    if (path.empty()) {
        std::cout << command_args << ": not found\n";
    }
    else {
        std::cout << command_args << " is " << path << "\n";
    }
}


void function_pwd()
{
    std::cout << std::filesystem::current_path().string() << "\n";
}


void function_cd(std::string command_args)
{
    const std::filesystem::path destination_path = command_args;
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


void function_execute(std::string command, std::string command_args)
{
    std::string path = get_path(command);
    if(path.empty()) {
        std::cout << command << ": command not found\n";
    }
    else {
        std::system((command + " " + command_args).c_str());
    }
}


std::string get_path(std::string command)
{
    std::string path_env = std::getenv("PATH");
    std::stringstream path_env_stream(path_env);
    std::string path;
    while (!path_env_stream.eof()) {
        std::getline(path_env_stream, path, ':');
        std::string abs_path = path + '/' + command;

        if(std::filesystem::exists(abs_path)) {
            return abs_path;
        }
    }
    return "";
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
