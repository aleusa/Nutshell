#pragma once

#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include <vector>
#include <sstream>
#include <unordered_map>
#include <sys/wait.h>
#include <cstring>
#include <fstream>

class exec {
public:
    std::string* command = new std::string;
    std::vector <std::string*>* argList;
};


class filesystem {
    public:
        std::string parent;
        std::string currentDirectory;
        std::vector <std::string> childDirectory;
        std::vector <std::string> files;
};

void displayExecList();

void create_command_object(std::string* command, std::vector <std::string*>* argList);

void create_command_object(std::string* command);

bool handle_command();

std::string* get_env_variable(std::string *var_name);

void set_env_variable(std::string* var, std::string* var_name);

void unset_env_variable(std::string* var_name);

void set_mandatory_env_variable();

void print_env(exec* execCommand);

void goBack();

std::string getCurrWorkDir();

void setInitialWorkDir();

void cd_command_handle(exec* execCommand);

void print_alias(exec* execCommand);

void add_alias(exec* execCommand);

exec* get_alias(std::string aliasName);

void translate(std::vector<std::string*>* values);

std::string search_path(std::string);

void run_external_command();

std::string get_run_external_command(exec* execCommand);

void run_external_command(exec* execCommand);

std::vector<std::string>* divideByDelimeter(exec* execCommand, std::string delimeter);

std::vector<exec*> divideByCommands(std::vector<std::string>* commads);

void output_redirect(exec* execCommand, bool isDouble);

void input_redirect(exec* execCommand);

void input_to_external_command(exec* execCommand, std::string input);


std::string get_str_between_brackets(const std::string &str);

void execute_with_pipes(exec *object1, exec *object2);
