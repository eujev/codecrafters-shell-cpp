#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <termios.h>
#include <unistd.h>

void get_non_can_input(std::string& input);
void handle_tab(std::string& input);
void handle_command(std::string command, std::string command_args);
void function_echo(std::string command_args);
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


void get_non_can_input(std::string& input)
{
    termios new_termio, old_termio;
    tcgetattr(STDIN_FILENO, &old_termio);
    new_termio = old_termio;
    new_termio.c_lflag &=~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &new_termio);
    
    const char BACKSPACE_KEY = 127;
    char c;
    while(true) {
        c = getchar();
        if (c == '\n') {
            std::cout << std::endl;
            break;
        }
        else if (c == '\t') {
            handle_tab(input);
        }
        else if (c == BACKSPACE_KEY) {
            if (!input.empty()) {
                input.pop_back();
                std::cout << "\b \b";
            }
        }
        else {
            std::cout << c;
            input += c;
        }
    }
    tcsetattr(STDIN_FILENO, TCSANOW, &old_termio);
}


void handle_tab(std::string& input)
{
    if (input == "ech") {
        input+= "o ";
        std::cout << "o ";
    }
    else if (input == "exi") {
        input+= "t ";
        std::cout << "t ";
    }
}


void handle_command(std::string command, std::string command_args)
{
    if (command == "echo") {
        function_echo(command_args);
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


void function_echo(std::string command_args)
{
    command_args = check_quotes(command_args);
    auto it = command_args.find(">");
    if (it != command_args.npos) {
        if (command_args.at(it-1) == '1') {
            if (command_args.at(it+1) == '>') {
                std::ofstream append_file;
                append_file.open(command_args.substr(it+3,command_args.length()), std::ios::app);
                append_file << command_args.substr(0, it-1) << '\n';
                append_file.close();
            }
            else {
                std::ofstream new_file(command_args.substr(it+2,command_args.length()));
                new_file << command_args.substr(0, it-1) << '\n';
                new_file.close();
            }
        }
        else if (command_args.at(it-1) == '2') {
            if (command_args.at(it+1) == '>') {
                std::ofstream append_file;
                append_file.open(command_args.substr(it+3,command_args.length()), std::ios::app);
                std::cout << command_args.substr(0, it-1) << '\n';
                append_file.close();
            }
            else {
                std::ofstream new_file(command_args.substr(it+2,command_args.length()));
                std::cout << command_args.substr(0, it-1) << "\n";
                new_file.close();
            }
        }
        else {
            if (command_args.at(it+1) == '>') {
                std::ofstream append_file;
                append_file.open(command_args.substr(it+3,command_args.length()), std::ios::app);
                append_file << command_args.substr(0, it) << '\n';
                append_file.close();
            }
            else {
                std::ofstream new_file(command_args.substr(it+2,command_args.length()));
                new_file << command_args.substr(0, it) << '\n';
                new_file.close();
            }
        }
    }
    else {
        std::cout << command_args << "\n";
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
