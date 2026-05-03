#pragma once

#include <string>
#include <vector>

char **custom_completer(const char* text, int start, int end);
char *command_generator(const char* text, int state);

class CC_Shell {
   public:
    CC_Shell();
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

   private:
    std::vector<std::string> history;
    std::string get_path(std::string command);
    std::string path_env;
};
