
#include "nutshell.tab.h"
#include "command_handler.h"
#include <iostream>
#include <vector>

int main(){
    int counter = 0;
    int ret = 1;
    bool isError = false;

    set_mandatory_env_variable();
    setInitialWorkDir();

    while(ret != 0){
        std::cout << "\033[1;32m";
        std::cout << "[nutshell]:"; 
        std::cout << "\033[1;34m";
        std::cout << getCurrWorkDir();
        std::cout << "\033[0;37m";
        std::cout << "$ ";
	    ret = yyparse();
        isError = handle_command();
        if(isError){
            std::cout << "Error! Try again." << std::endl;
            //yyparse();
        }
    }
    displayExecList();
}
