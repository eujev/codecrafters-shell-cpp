#pragma once

#include <cstring>
#include <set>
#include <string>
#include <vector>
#include <readline/readline.h>
#include <readline/history.h>


class CC_Shell {
   public:
    CC_Shell() {
        path_env = std::getenv("PATH");
        get_path_commands();
    };
    void get_non_can_input(std::string &input);
    bool handle_tab(std::string &input, bool double_tap);
    void handle_command(std::string command, std::string command_args);
    void command_echo(std::string command_args);
    void command_type(std::string command_args);
    void command_execute(std::string command, std::string command_args);
    void command_pwd();
    void command_cd(std::string command_args);
    void command_history(std::string command_args);
    void add_to_history(std::string &input);
    void get_path_commands();

    static char **custom_completer(const char* text, int start, int end) {
        rl_attempted_completion_over = 1;  // NOTE: Check again what this exactly does
        if (start == 0) {
            return rl_completion_matches(text, command_generator);
        }
        return nullptr;
    }

   private:
    std::vector<std::string> history;
    std::string get_path(std::string command);
    std::string path_env;
    std::vector<std::string> builtin_commands{"echo", "type", "exit",
                                              "pwd",  "cd",   "history"};
    std::set<std::string> all_commands;

    static CC_Shell& get_Instance() {
        static CC_Shell instance;
        return instance;
    }

    static char *command_generator(const char* text, int state) {
        static size_t list_index;
        static std::set<std::string>::iterator it;
        static size_t len;

        auto& cmd = get_Instance().all_commands;

        if (state == 0) {
            list_index = 0;
            it = cmd.begin();
            len = strlen(text);
        }


        while (it != cmd.end()) {
            const std::string& command = *it++;
            if(command.compare(0, len, text) == 0) {
                return strdup(command.c_str());
            }
        }
        return nullptr;
    }

};
