#include "command_handler.h"
#define READ_END 1
#define WRITE_END 0

std::vector <exec*> execList;
std::unordered_map <std::string, std::string> envVariables;
std::unordered_map <std::string, exec*> aliases;
filesystem *fileSystemObject = new filesystem;

void displayExecList(){
    std::string output;
    int length;
    for(int i = 0; i < execList.size(); i++) {
        std::cout << std::endl;
        std::cout << "---COMMAND---" << std::endl;
        std::cout << *execList[i]->command << std::endl;
        std::cout << "---ARGUMENTS---" << std::endl;
        try{
            length = execList[i]->argList->size();
        }
        catch(const std::out_of_range& oor){
            length = 0;
        }
        for (int j = 0; j < length; j++) {
            try{
                output = *execList[i]->argList->at(j);
            }
            catch(const std::out_of_range& oor){
                output = "";
            }
            std::cout << *execList[i]->argList->at(j) << std::endl;
        }
    }
    
}

void create_command_object(std::string* command, std::vector <std::string*>* argList){
    exec *object = new exec;
    object->command = command;
    object->argList = argList;
    execList.push_back(object);
}

void create_command_object(std::string* command){
    exec *object = new exec;
    object->command = command;
    object->argList = new std::vector<std::string*>;
    execList.push_back(object);
}

bool handle_command(){
    exec* execCommand;
    try{
        execCommand = execList.at(execList.size()-1);
    }
    catch(const std::out_of_range& oor){
        execCommand = nullptr;
    }

    if(execCommand == nullptr){
        return true;
    }
    else if (*execCommand->command == "bye"){
        return false;
    }
    else if(*execCommand->command == "setenv"){
        if(execCommand->argList->size() != 2){
            return true;
        }
        std::string* var_name = execCommand->argList->at(0);
        std::string* var = execCommand->argList->at(1);

        set_env_variable(var_name, var);
    }
    else if(*execCommand->command == "printenv"){
        print_env(execCommand);
    }
    else if(*execCommand->command == "unsetenv"){
        if(execCommand->argList->size() != 1){
            return true;
        }
        unset_env_variable(execCommand->argList->at(0));
    }
    else if(*execCommand->command == "cd"){
        if(execCommand->argList->size() != 1){
            return true;
        }
        else{
            cd_command_handle(execCommand);
        }
    }
    else if(*execCommand->command == "alias"){
        bool isPrintAlias = false;
        
        for(int i = 0; i < execCommand->argList->size(); i++){
            if(*execCommand->argList->at(i) == "|" || *execCommand->argList->at(i) == ">" || *execCommand->argList->at(i) == ">>"){
                isPrintAlias = true;
            }
        }

        if(execCommand->argList->empty() || isPrintAlias){
            print_alias(execCommand);
        }
        else{
            if(execCommand->argList->size() != 2){
                return true;
            }
            else{
                add_alias(execCommand);
            }
        }
    }
    else if(*execCommand->command == "unalias"){
        if(execCommand->argList->size() != 1){
                return true;
        }
        else{
            if(aliases.find(*execCommand->argList->at(0)) == aliases.end())
                return true;
            else
                aliases.erase(*execCommand->argList->at(0));
        }
    }
    else {
        std::vector<std::string> *commandsAndArguments = new std::vector<std::string>;
        std::vector<exec*> execs;
        bool isAPipe = false;
        bool outputRedirection = false;
        bool outputRedirectionDouble = false;
        bool inputRedirection = false;
        
        for(int i = 0; i < execCommand->argList->size(); i++){
            if(*execCommand->argList->at(i) == "|"){
                isAPipe = true;
            }
            else if(*execCommand->argList->at(i) == ">"){
                outputRedirection = true;
            }
            else if(*execCommand->argList->at(i) == ">>"){
                outputRedirectionDouble = true;
            }
            else if(*execCommand->argList->at(i) == "<"){
                inputRedirection = true;
            }
        }
        if(isAPipe){
            commandsAndArguments = divideByDelimeter(execCommand, "|");
            execs = divideByCommands(commandsAndArguments);
            /*for(int i = 0; i < execs.size(); i++){
                std::cout << *execs.at(i)->command << " " << *execs.at(i)->argList->at(0) << std::endl;
            }*/
            execute_with_pipes(execs.at(0), execs.at(1));
        }
        else if(outputRedirection){
            output_redirect(execCommand, false);
        }
        else if(outputRedirectionDouble){
            output_redirect(execCommand, true);
        }
        else if(inputRedirection){
            input_redirect(execCommand);
        }
        else{
            run_external_command(execCommand);
        }
    }

    return false;
        
}

void cd_command_handle(exec* execCommand){
    std::string beforeDirectory = fileSystemObject->currentDirectory;
    if(execCommand->argList->at(0)->at(0) == '/'){
        fileSystemObject->currentDirectory = "";
    }

    std::vector<std::string> tokens;
    char delimeter = '/';
    std::string tempDirection = *execCommand->argList->at(0);

    size_t start;
    size_t end = 0;
    while ((start = tempDirection.find_first_not_of(delimeter, end)) != std::string::npos){
        end = tempDirection.find(delimeter, start);
        tokens.push_back(tempDirection.substr(start, end - start));
    }

    for(int i = 0; i < tokens.size(); i++){
        if(tokens.at(i) == ".."){
            goBack();
        }
        else if(tokens.at(i) == "."){}
        else{
            if((beforeDirectory.size() == 0 && execCommand->argList->at(0)->at(0) != '/' && i == 0) || (execCommand->argList->at(0)->at(0) != '/' && beforeDirectory == "/" && i == 0))
                fileSystemObject->currentDirectory += tokens.at(i);
            else
                fileSystemObject->currentDirectory += delimeter + tokens.at(i);
        }
    }
    if (chdir(fileSystemObject->currentDirectory.c_str()) != 0) {
        fileSystemObject->currentDirectory = beforeDirectory;
        perror("lsh");
    }
}

std::string* get_env_variable(std::string *var_name){
    std::string* value;

    try{
        value = &envVariables.at(*var_name);
    }
    catch(const std::out_of_range& oor){
        value = nullptr;
    }
    
    return value;

}

void set_env_variable(std::string* var_name, std::string* var){
    std::vector<std::string> *tokens = new std::vector<std::string>;
    char delimeter = ' ';
    std::string tempDirection = *var;

    size_t start;
    size_t end = 0;
    while ((start = tempDirection.find_first_not_of(delimeter, end)) != std::string::npos){
        end = tempDirection.find(delimeter, start);
        tokens->push_back(tempDirection.substr(start, end - start));
    }

    exec *object = new exec;

    std::string temp = tokens->at(0);

    if(temp != "setenv" && temp != "printenv" && temp != "unsetenv" && temp != "cd" && temp != "alias" && temp != "unalias" && temp != "bye" && temp != "."){
        tokens->at(0) = search_path(temp);
    }

    bool Aexpansion = false;
    bool Eexpansion = false;
    std::string envExpansion;

    for(auto it = aliases.begin(); it != aliases.end(); it++){
        if(temp == it->first){
            Aexpansion = true;
            object->command = it->second->command;
            object->argList = it->second->argList;
        }
    }
    
    for(int i = 0; i < tokens->size() ; i++){
        tokens->at(i) = get_str_between_brackets(tokens->at(i));
        for(auto it = envVariables.begin(); it != envVariables.end(); it++){
            if(tokens->at(i) == it->first){
                Eexpansion = true;
                tokens->at(i) = it->second;
                envExpansion = it->second;
                break;
            }
        }
    }

    std::string str = "";
    if(Aexpansion){
        str += *object->command;
        for(int i = 0; i < object->argList->size(); i++){
            str += " " + *object->argList->at(i);
        }
        if(Eexpansion){
            str += " " + envExpansion;
        }
        else{
            str += " " + tokens->at(1);
        }
    }
    else{
        str = tokens->at(0);
        for(int i = 1; i < tokens->size(); i++){
            str += " " + tokens->at(i);
        }
    }

    envVariables[*var_name] = str;

}

void unset_env_variable(std::string* var_name){
    if(*var_name == "HOME"){
        std::cout << "You cannot remove HOME variable!" << std::endl;
    }
    else if(*var_name == "PATH"){
        std::cout << "You cannot remove PATH variable!" << std::endl;
    }
    else{
        envVariables.erase(*var_name);
    }
}


void set_mandatory_env_variable(){
    envVariables["HOME"] = getenv("HOME");
    envVariables["PATH"] = getenv("PATH");
}

void print_env(exec* execCommand){
    bool isAPipe = false;
    bool outputRedirection = false;
    bool outputRedirectionDouble = false;
    std::string delimeter;
    
    for(int i = 0; i < execCommand->argList->size(); i++){
        if(*execCommand->argList->at(i) == "|"){
            isAPipe = true;
            delimeter = "|";
        }
        else if(*execCommand->argList->at(i) == ">"){
            outputRedirection = true;
            delimeter = ">";
        }
        else if(*execCommand->argList->at(i) == ">>"){
            outputRedirectionDouble = true;
            delimeter = ">>";
        }
    }
    if(!isAPipe && !outputRedirection && !outputRedirectionDouble){
        for( auto it = envVariables.begin(); it != envVariables.end(); it++){
            std::cout << it->first << ":" << it->second << std::endl;
        }
    }
    else{
        if(execCommand->argList->size() == 2){
            std::vector<std::string> *commandsAndArguments = new std::vector<std::string>;
            std::string output = "";
            commandsAndArguments = divideByDelimeter(execCommand, delimeter);

            if(!isAPipe){
                commandsAndArguments->at(1).erase(0,1);

                for( auto it = envVariables.begin(); it != envVariables.end(); it++){
                    output += it->first + ":" + it->second + "\n";
                }
                
                std::string temp;
                temp = getCurrWorkDir() + "/" + commandsAndArguments->at(1);
                
                std::ofstream MyFile;

                if(outputRedirection)
                    MyFile.open(temp);
                else if(outputRedirectionDouble){
                    MyFile.open(temp, std::ofstream::app);
                }
                MyFile << output;
                MyFile.close();
            }
            else{
                std::vector<exec*> execs;
                execs = divideByCommands(commandsAndArguments);
                /*for(int i = 0; i < execs.size(); i++){
                    std::cout << *execs.at(i)->command << std::endl;
                }*/
                execute_with_pipes(execs.at(0), execs.at(1));
            }
        }
        else{
            std::cout << "Error" << std::endl;
        }
    }
}

void goBack(){
    std::vector<std::string> tokens;
    char delimeter = '/';
    std::string tempDirection = fileSystemObject->currentDirectory;
    fileSystemObject->currentDirectory = "";

    size_t start;
    size_t end = 0;
    while ((start = tempDirection.find_first_not_of(delimeter, end)) != std::string::npos){
        end = tempDirection.find(delimeter, start);
        tokens.push_back(tempDirection.substr(start, end - start));
    }

    if(tokens.size() <= 1){
        fileSystemObject->currentDirectory = "/";
    }
    else{
        for(int i = 0; i < tokens.size() - 1; i++){
            fileSystemObject->currentDirectory += delimeter + tokens.at(i);
        }
    }
}

std::string getCurrWorkDir(){
    return fileSystemObject->currentDirectory;
}

void setInitialWorkDir(){
    fileSystemObject->currentDirectory = get_current_dir_name();
    chdir(fileSystemObject->currentDirectory.c_str());
}

void print_alias(exec* execCommand){
    bool isAPipe = false;
    bool outputRedirection = false;
    bool outputRedirectionDouble = false;
    std::string delimeter;
    
    for(int i = 0; i < execCommand->argList->size(); i++){
        if(*execCommand->argList->at(i) == "|"){
            isAPipe = true;
            delimeter = "|";
        }
        else if(*execCommand->argList->at(i) == ">"){
            outputRedirection = true;
            delimeter = ">";
        }
        else if(*execCommand->argList->at(i) == ">>"){
            outputRedirectionDouble = true;
            delimeter = ">>";
        }
    }
    if(!isAPipe && !outputRedirection && !outputRedirectionDouble){    
        for(auto it = aliases.begin(); it != aliases.end(); it++){
            std::cout << it->first << "=" << *it->second->command << " ";

            for(int i = 0; i < it->second->argList->size(); i++){
                std::cout << *it->second->argList->at(i) << " ";
            }
            std::cout << std::endl;
        }
    }
    else{
        if(execCommand->argList->size() == 2){
            std::vector<std::string> *commandsAndArguments = new std::vector<std::string>;
            std::string output = "";
            commandsAndArguments = divideByDelimeter(execCommand, delimeter);

            if(!isAPipe){
                commandsAndArguments->at(1).erase(0,1);

                for(auto it = aliases.begin(); it != aliases.end(); it++){
                    output += it->first + "=" + *it->second->command + " ";

                    for(int i = 0; i < it->second->argList->size(); i++){
                        output += *it->second->argList->at(i) + " ";
                    }
                    output += "\n";
                }
                
                std::string temp;
                temp = getCurrWorkDir() + "/" + commandsAndArguments->at(1);
                
                std::ofstream MyFile;

                if(outputRedirection)
                    MyFile.open(temp);
                else if(outputRedirectionDouble){
                    MyFile.open(temp, std::ofstream::app);
                }
                MyFile << output;
                MyFile.close();
            }
            else{
                std::vector<exec*> execs;
                execs = divideByCommands(commandsAndArguments);
                /*for(int i = 0; i < execs.size(); i++){
                    std::cout << *execs.at(i)->command << std::endl;
                }*/
                execute_with_pipes(execs.at(0), execs.at(1));
            }
        }
        else{
            std::cout << "Error" << std::endl;
        }
    }
}

exec* get_alias(std::string aliasName){
    exec *object;

    try{
        object = aliases[aliasName];
    }
    catch(const std::out_of_range& oor){
        object = nullptr;
    }
    return object;
}


void add_alias(exec* execCommand){
    std::vector<std::string> *tokens = new std::vector<std::string>;
    char delimeter = ' ';
    std::string tempDirection = *execCommand->argList->at(1);

    size_t start;
    size_t end = 0;
    while ((start = tempDirection.find_first_not_of(delimeter, end)) != std::string::npos){
        end = tempDirection.find(delimeter, start);
        tokens->push_back(tempDirection.substr(start, end - start));
    }

    exec *object = new exec;

    std::string temp = tokens->at(0);

    if(temp != "setenv" && temp != "printenv" && temp != "unsetenv" && temp != "cd" && temp != "alias" && temp != "unalias" && temp != "bye"){
        tokens->at(0) = search_path(temp);
    }

    bool expansion = false;
    for(auto it = aliases.begin(); it != aliases.end(); it++){
        if(temp == it->first){
            expansion = true;
            object->command = it->second->command;
            object->argList = it->second->argList;
        }
    }
    
    if(!expansion){
        if(tokens->size() == 1){
            object->command = &tokens->at(0);
            object->argList = new std::vector<std::string*>;
        }
        else if(tokens->size() == 2){
            std::vector<std::string*> *argVect = new std::vector<std::string*>;
            for(int i = 1; i < tokens->size(); i++){
                argVect->push_back(&tokens->at(i));
            }
            
            object->command = &tokens->at(0);
            object->argList = argVect;
        }
    }
    aliases.emplace(*execCommand->argList->at(0), object);
}

void translate(std::vector<std::string*>* values){
    exec *alias;
        
    alias = get_alias(*values->at(0));

    if(alias != nullptr){
        values->erase(values->begin());
        alias->argList->insert(alias->argList->end(), values->begin(), values->end() );
        execList.push_back(alias);
    }
    else{
        std::string temp = search_path(*values->at(0));
        std::string *external_command = new std::string;  
        external_command = &temp;
        std::vector<std::string*>* args = new std::vector<std::string*>;
        args = values;
        
        if(*external_command != ""){
            args->erase(values->begin());
            create_command_object(external_command, args);
        }


    }
}

std::string search_path(std::string external_command){
    std::string* path = &envVariables.at("PATH");
    std::stringstream path_stream(*path);
    std::vector<std::string> paths;
      
    std::string intermediate;
      
    while(getline(path_stream, intermediate, ':')){
        paths.push_back(intermediate);
    }

    struct dirent *de;

    DIR *dr;

    for(int i = 0; i < paths.size(); i++){
        dr = opendir(paths.at(i).c_str());
        if(dr == NULL){
            continue;
        }

        while ((de = readdir(dr)) != NULL){
            if(de->d_name == external_command){
                closedir(dr);
                return paths.at(i) + "/" + external_command;
            }
        }
    }
    closedir(dr);    
    return external_command;
      
}

void run_external_command(exec* execCommand){
    int link[2];
    pid_t pid;
    char foo[4096];

    std::vector<char*> args;
    args.push_back(strdup(execCommand->command->c_str()));
    for(int i = 0; i < execCommand->argList->size(); i++){
        args.push_back(strdup(execCommand->argList->at(i)->c_str()));
    }
    args.push_back(NULL);

    if (pipe(link)==-1){
        std::cout << "Pipe failure." << std::endl;
    }
    if ((pid = fork()) == -1){
        std::cout << "Fork failure" << std::endl;
    }
    if(pid == 0) {
        dup2 (link[1], STDOUT_FILENO);
        close(link[0]);
        close(link[1]);
        execv(execCommand->command->c_str(), &args[0]);
    } 
    else {
        close(link[1]);
        int nbytes = read(link[0], foo, sizeof(foo));
        printf("%.*s", nbytes, foo);
        wait(NULL);

  }

}

std::string get_run_external_command(exec* execCommand){
    std::string token = "";
    int link[2];
    pid_t pid;
    char foo[4096];

    std::vector<char*> args;
    args.push_back(strdup(execCommand->command->c_str()));
    for(int i = 0; i < execCommand->argList->size(); i++){
        args.push_back(strdup(execCommand->argList->at(i)->c_str()));
    }
    args.push_back(NULL);

    if (pipe(link)==-1){
        std::cout << "Pipe failure." << std::endl;
    }
    if ((pid = fork()) == -1){
        std::cout << "Fork failure" << std::endl;
    }
    if(pid == 0) {
        dup2 (link[1], STDOUT_FILENO);
        close(link[0]);
        close(link[1]);
        execv(execCommand->command->c_str(), &args[0]);
    } 
    else {
        close(link[1]);
        int nbytes = read(link[0], foo, sizeof(foo));
        std::string s = "";
        for (int i = 0; i < nbytes; i++) {
            s = s + foo[i];
        }
        token = s;
        wait(NULL);

  }        
  return token;
}

std::vector<std::string>* divideByDelimeter(exec* execCommand, std::string delimeter){
    std::vector<std::string>* tokens = new std::vector<std::string>;
    std::string tempDirection = *execCommand->command;

    for(int i = 0; i < execCommand->argList->size(); i++){
        tempDirection += " " + *execCommand->argList->at(i);
    }

    size_t start;
    size_t end = 0;
    while ((start = tempDirection.find_first_not_of(delimeter, end)) != std::string::npos){
        end = tempDirection.find(delimeter, start);
        tokens->push_back(tempDirection.substr(start, end - start));
    }

    return tokens;
}

std::vector<exec*> divideByCommands(std::vector<std::string>* commands){
    std::vector<exec*> execs;
    int count = 0;
    for(int i = 0; i < commands->size(); i++){
        std::vector<std::string> *tokens = new std::vector<std::string>;
        char delimeter = ' ';
        std::string tempDirection = commands->at(i);

        size_t start;
        size_t end = 0;
        while ((start = tempDirection.find_first_not_of(delimeter, end)) != std::string::npos){
            end = tempDirection.find(delimeter, start);
            tokens->push_back(tempDirection.substr(start, end - start));
        }
        exec *object = new exec;
        std::vector<std::string*> *argVect = new std::vector<std::string*>;

        if(i > 0){
            if(tokens->at(0) != "setenv" && tokens->at(0) != "printenv" && tokens->at(0) != "unsetenv" && tokens->at(0) != "cd" && tokens->at(0) != "alias" && tokens->at(0) != "unalias" && tokens->at(0) != "bye"){
                tokens->at(0) = search_path(tokens->at(0));
            }
        }
        
        object->command = &tokens->at(0);

        for(int i = 1; i < tokens->size(); i++){
            argVect->push_back(&tokens->at(i));
        }
        object->argList = argVect;
        execs.push_back(object);
    }
    return execs;
}

void output_redirect(exec* execCommand, bool isDouble){
    std::vector<std::string> *commandsAndArguments = new std::vector<std::string>;
    std::string output = "";
    if(isDouble)
        commandsAndArguments = divideByDelimeter(execCommand, ">>");
    else
        commandsAndArguments = divideByDelimeter(execCommand, ">");
    
    std::vector<std::string> *tokens = new std::vector<std::string>;
    char delimeter = ' ';
    std::string tempDirection = commandsAndArguments->at(0);

    size_t start;
    size_t end = 0;
    while ((start = tempDirection.find_first_not_of(delimeter, end)) != std::string::npos){
        end = tempDirection.find(delimeter, start);
        tokens->push_back(tempDirection.substr(start, end - start));
    }
    exec *object = new exec;
    std::vector<std::string*> *argVect = new std::vector<std::string*>;
    object->command = &tokens->at(0);
    for(int i = 1; i < tokens->size(); i++){
        argVect->push_back(&tokens->at(i));
    }
    object->argList = argVect;
    
    commandsAndArguments->at(1).erase(0,1);

    output = get_run_external_command(object);
    std::string temp;
    temp = getCurrWorkDir() + "/" + commandsAndArguments->at(1);
    
    std::ofstream MyFile;

    if(!isDouble)
        MyFile.open(temp);
    else{
        MyFile.open(temp, std::ofstream::app);
    }
    MyFile << output;
    MyFile.close();
}

void input_redirect(exec* execCommand){
    std::string completeInput = "";
    std::vector<std::string> *commandsAndArguments = new std::vector<std::string>;
    commandsAndArguments = divideByDelimeter(execCommand, "<");
    
    std::vector<std::string> *tokens = new std::vector<std::string>;
    char delimeter = ' ';
    std::string tempDirection = commandsAndArguments->at(0);

    size_t start;
    size_t end = 0;
    while ((start = tempDirection.find_first_not_of(delimeter, end)) != std::string::npos){
        end = tempDirection.find(delimeter, start);
        tokens->push_back(tempDirection.substr(start, end - start));
    }
    exec *object = new exec;
    std::vector<std::string*> *argVect = new std::vector<std::string*>;
    object->command = &tokens->at(0);
    for(int i = 1; i < tokens->size(); i++){
        argVect->push_back(&tokens->at(i));
    }
    
    commandsAndArguments->at(1).erase(0,1);

    std::string temp;
    temp = getCurrWorkDir() + "/" + commandsAndArguments->at(1);

    argVect->push_back(&temp);
    object->argList = argVect;
    run_external_command(object);
}

void input_to_external_command(exec* execCommand, std::string input){
    int fds[2];

    std::vector<char*> args;

    for(int i = 0; i < execCommand->argList->size(); i++){
        args.push_back(strdup(execCommand->argList->at(i)->c_str()));
    }
    args.push_back(NULL);

    pipe(fds);
    close(STDIN_FILENO);
    dup2(fds[0], STDIN_FILENO);
    write(fds[1], input.c_str(), strlen(input.c_str()));

    execv(execCommand->command->c_str(), &args[0]);
}

void execute_with_pipes(exec *object1, exec *object2){
    pid_t pid;
    int fd[2];

    std::string cmd1 = *object1->command;
    std::string cmd2 = *object2->command;

    std::vector<char*> args1;
    args1.push_back(strdup(cmd1.c_str()));
    for(int i = 0; i < object1->argList->size(); i++){
        args1.push_back(strdup(object1->argList->at(i)->c_str()));
    }
    args1.push_back(NULL);

    std::vector<char*> args2;
    args2.push_back(strdup(cmd2.c_str()));
    for(int i = 0; i < object2->argList->size(); i++){
        args2.push_back(strdup(object2->argList->at(i)->c_str()));
    }
    args2.push_back(NULL);

    pipe(fd);
    pid = fork();

    if(pid==0){
        dup2(fd[WRITE_END], STDOUT_FILENO);
        close(fd[READ_END]);
        close(fd[WRITE_END]);
        execv(cmd1.c_str(), &args1[0]);
        fprintf(stderr, "Failed to execute '%s'\n", cmd1);
        exit(1);
    }
    else{ 
        pid=fork();

        if(pid==0){
            dup2(fd[READ_END], STDIN_FILENO);
            close(fd[WRITE_END]);
            close(fd[READ_END]);
            execv(cmd2.c_str(), &args2[0]);
            fprintf(stderr, "Failed to execute '%s'\n", cmd2);
            exit(1);
        }
        else{
            int status;
            close(fd[READ_END]);
            close(fd[WRITE_END]);
            waitpid(pid, &status, 0);
        }
    }
}

std::string get_str_between_brackets(const std::string &str){
    unsigned firstPos = str.find("{");
    unsigned endPos_of_firstPos = firstPos + 1;
    unsigned lastPos = str.find("}");
    return str.substr(endPos_of_firstPos, lastPos - endPos_of_firstPos);
}