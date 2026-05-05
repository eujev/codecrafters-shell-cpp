#include "cc_shell.hpp"

#include <readline/history.h>
#include <readline/readline.h>
#include <termios.h>
#include <unistd.h>

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <system_error>
#include <vector>

#include "parser.hpp"

namespace {

 bool is_executable(const std::filesystem::path& path) {
    std::filesystem::perms permissions = std::filesystem::status(path).permissions();
    return (permissions & (std::filesystem::perms::owner_exec | std::filesystem::perms::group_exec | std::filesystem::perms::others_exec)) != std::filesystem::perms::none;
  }
}


void CC_Shell::get_path_commands() {
    all_commands.insert(builtin_commands.begin(), builtin_commands.end());
    std::stringstream path_env_stream(path_env);
    std::string path;
    while (std::getline(path_env_stream, path, ':')) {
        std::error_code ec;

        if (!std::filesystem::exists(path, ec) || !std::filesystem::is_directory(path, ec)) {
            continue;
        }
        for (const auto &entry : std::filesystem::directory_iterator(path, ec)) {
            if (ec) {
                break;
            }
            if (entry.exists() && !entry.is_directory() && is_executable(entry.path())) {
                auto const cmd = entry.path().filename().string();
                all_commands.insert(cmd);
            }
        }
    }
}


void CC_Shell::handle_command(std::string command, std::string command_args) {
    if (command == "echo") {
        command_echo(command_args);
    }
    else if (command == "type") {
        command_type(command_args);
    }
    else if (command == "pwd") {
        command_pwd();
    }
    else if (command == "cd") {
        command_cd(command_args);
    }
    else if (command == "history") {
        command_history(command_args);
    }
    else {
        command_execute(command, command_args);
    }
}

// Builtin function 'echo'.
void CC_Shell::command_echo(std::string command_args) {
    command_args = Parser::check_quotes(command_args);
    auto it = command_args.find(">");
    if (it != command_args.npos) {
        if (command_args.at(it - 1) == '1') {
            if (command_args.at(it + 1) == '>') {
                std::ofstream append_file;
                append_file.open(
                    command_args.substr(it + 3, command_args.length()),
                    std::ios::app);
                append_file << command_args.substr(0, it - 1) << '\n';
                append_file.close();
            }
            else {
                std::ofstream new_file(
                    command_args.substr(it + 2, command_args.length()));
                new_file << command_args.substr(0, it - 1) << '\n';
                new_file.close();
            }
        }
        else if (command_args.at(it - 1) == '2') {
            if (command_args.at(it + 1) == '>') {
                std::ofstream append_file;
                append_file.open(
                    command_args.substr(it + 3, command_args.length()),
                    std::ios::app);
                std::cout << command_args.substr(0, it - 1) << '\n';
                append_file.close();
            }
            else {
                std::ofstream new_file(
                    command_args.substr(it + 2, command_args.length()));
                std::cout << command_args.substr(0, it - 1) << "\n";
                new_file.close();
            }
        }
        else {
            if (command_args.at(it + 1) == '>') {
                std::ofstream append_file;
                append_file.open(
                    command_args.substr(it + 3, command_args.length()),
                    std::ios::app);
                append_file << command_args.substr(0, it) << '\n';
                append_file.close();
            }
            else {
                std::ofstream new_file(
                    command_args.substr(it + 2, command_args.length()));
                new_file << command_args.substr(0, it) << '\n';
                new_file.close();
            }
        }
    }
    else {
        std::cout << command_args << "\n";
    }
}

// Builtin function 'type'.
void CC_Shell::command_type(std::string command_args) {
    if (std::find(builtin_commands.begin(), builtin_commands.end(),
                  command_args) != builtin_commands.end()) {
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

// Builtin function 'pwd'.
void CC_Shell::command_pwd() {
    std::cout << std::filesystem::current_path().string() << "\n";
}

// Builtin function 'cd'.
void CC_Shell::command_cd(std::string command_args) {
    const std::filesystem::path destination_path = command_args;
    if (destination_path.string() == "~") {
        std::string home_dir = std::getenv("HOME");
        std::filesystem::current_path(home_dir);
    }
    else if (std::filesystem::exists(destination_path)) {
        std::filesystem::current_path(destination_path);
    }
    else {
        std::cout << "cd: " << destination_path.string()
                  << ": No such file or directory\n";
    }
}

// Executes files in the path.
void CC_Shell::command_execute(std::string command, std::string command_args) {
    std::string path = get_path(command);
    if (path.empty()) {
        std::cout << command << ": command not found\n";
    }
    else {
        std::system((command + " " + command_args).c_str());
    }
}

// Returns the absolute path if the command was found
std::string CC_Shell::get_path(std::string command) {
    std::stringstream path_env_stream(path_env);
    std::string path;
    while (!path_env_stream.eof()) {
        std::getline(path_env_stream, path, ':');
        std::string abs_path = path + '/' + command;

        if (std::filesystem::exists(abs_path) && is_executable(abs_path)) {
            return abs_path;
        }
    }
    return "";
}


bool isNumber(const std::string &str) {
    return std::all_of(str.begin(), str.end(),
                       [](unsigned char c) { return std::isdigit(c); });
}

bool isWhiteeSpace(const std::string &str) {
    return std::all_of(str.begin(), str.end(),
                       [](unsigned char c) { return std::isspace(c); });
}

void CC_Shell::command_history(std::string command_args) {
    int start = history_base;
    int x{0};
    if (!command_args.empty() && isNumber(command_args)) {
        try {
            x = std::stoi(command_args);
        } catch (...) {
            std::cerr << "history: invalid number\n";
        }
    }
    if (x > 0 && x < history_length) {
        start = history_base + history_length - x;
    }

    for (int i = start; i < history_base + history_length; i++) {
        HIST_ENTRY* entry = history_get(i);
        if (entry != nullptr) {
            std::cout << "    " << i << "  " << entry->line << "\n";
        }
    }
}
