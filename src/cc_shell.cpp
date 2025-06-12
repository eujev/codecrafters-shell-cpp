#include "cc_shell.hpp"

#include <cstddef>
#include <iostream>
#include <termios.h>
#include <unistd.h>
#include <vector>
#include <string>
#include <algorithm>
#include <filesystem>
#include <fstream>

#include "parser.hpp"

CC_Shell::CC_Shell() {
    path_env = std::getenv("PATH");
}

// Changes the terminal mode, so that each individual character can be handled
void CC_Shell::get_non_can_input(std::string &input) {
  termios new_termio, old_termio;
  tcgetattr(STDIN_FILENO, &old_termio);
  new_termio = old_termio;
  new_termio.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &new_termio);

  const char BACKSPACE_KEY = 127;
  char c;
  bool double_tap{false};
  while (true) {
    c = getchar();
    if (c == '\n') {
      std::cout << std::endl;
      break;
    } else if (c == '\t') {
      double_tap = handle_tab(input, double_tap);
    } else if (c == BACKSPACE_KEY) {
      if (!input.empty()) {
        input.pop_back();
        std::cout << "\b \b";
      }
      double_tap = false;
    } else if (c >= 32 && c < BACKSPACE_KEY) {
      std::cout << c;
      input += c;
      double_tap = false;
    }
  }
  tcsetattr(STDIN_FILENO, TCSANOW, &old_termio);
}


// Handles the autocompletion when pressing <TAB>.
// @return if it was double tapped.
bool CC_Shell::handle_tab(std::string& input, bool double_tap) {
    std::vector<std::string> all_commands{builtin_commands};
    std::stringstream path_env_stream(path_env);
    std::string path;
    while (!path_env_stream.eof()) {
        std::getline(path_env_stream, path, ':');
        for (const auto &entry : std::filesystem::directory_iterator(path)) {
            if (entry.exists() && !entry.is_directory() && ((std::filesystem::perms::owner_exec & entry.status().permissions()) != std::filesystem::perms::none) ) {
                auto const start_cmd = entry.path().string().find_last_of('/');
                if (start_cmd != std::string::npos) {
                    std::string cmd = entry.path().string().substr(start_cmd+1);
                    // TODO: Try to remove the find and if condition for 'it' 
                    auto it = std::find(all_commands.begin(), all_commands.end(), cmd);
                    if (it == all_commands.end()) {
                        all_commands.push_back(cmd);
                    }
                }
            }
        }
    }

    std::vector<std::string> ac_candidates;
    for (auto command : all_commands) {
        if (command.starts_with(input) == true) {
            ac_candidates.push_back(command);
        }
    }
    std::sort(ac_candidates.begin(), ac_candidates.end());
    if (ac_candidates.empty()) {
        std::cout << '\a';
        return false;
    }
    else if (ac_candidates.size() == 1) {
        std::string to_add = ac_candidates.at(0).substr(input.size()) + " ";
        input += to_add;
        std::cout << to_add;
        return false;
    }
    else if (ac_candidates.size() > 1 && !double_tap) {
        std::string prefix = Parser::get_longest_common_prefix(ac_candidates);

        if (input.size() == prefix.size()) {
            return true;
        }
        std::string to_add = prefix.substr(input.size());
        input += to_add;
        std::cout << to_add;
        return false;
    }
    else if (double_tap) {
        std::cout << '\a' << '\n';
        for (auto print_candidate : ac_candidates) {
            std::cout << print_candidate << "  ";
        }
        std::cout << '\n' << "$ " << input;
        return false;
    }
    else {
        return true;
    }
}


void CC_Shell::handle_command(std::string command, std::string command_args)
{
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
        command_history();
    }
    else {
        command_execute(command, command_args);
    }
}


// Builtin function 'echo'.
void CC_Shell::command_echo(std::string command_args)
{
    command_args = Parser::check_quotes(command_args);
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


// Builtin function 'type'.
void CC_Shell::command_type(std::string command_args)
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


// Builtin function 'pwd'.
void CC_Shell::command_pwd()
{
    std::cout << std::filesystem::current_path().string() << "\n";
}


// Builtin function 'cd'.
void CC_Shell::command_cd(std::string command_args)
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


// Executes files in the path.
void CC_Shell::command_execute(std::string command, std::string command_args)
{
    std::string path = get_path(command);
    if(path.empty()) {
        std::cout << command << ": command not found\n";
    }
    else {
        std::system((command + " " + command_args).c_str());
    }
}


// Returns the absolute path if the command was found
std::string CC_Shell::get_path(std::string command)
{
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

void CC_Shell::command_history() {
    for (size_t idx = 1; auto command : history) {
         std::cout << "    " << idx << "  " << command << "\n";
         ++idx;
    }
}

void CC_Shell::add_to_history(std::string &input) {
    history.emplace_back(input);
}
