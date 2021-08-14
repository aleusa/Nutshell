%{
#include "command_handler.h"
#include <stdio.h>

int yylex();

int yyparse();

void yyerror(char* e) {
    
}

%}

%code requires {
#include "command_handler.h"
}

%define api.value.type union


%token <std::string*> aliasCMD envCMD dirCMD DATA END BYE DOLLAR OPENBRACE CLOSEBRACE STRING_LITERAL QUOTE

%nterm <std::vector<std::string*>*> data_list
%nterm <std::string*> data_item


%%

cmd_line:
    BYE END                     { create_command_object($1); return 0; };
    | envCMD data_list END      { create_command_object($1, $2); return 1; }
    | envCMD END                { create_command_object($1); return 1;}
    | dirCMD data_list END      { create_command_object($1, $2); return 1; }
    | dirCMD END                { create_command_object($1); return 1; }
    | aliasCMD data_list END    { create_command_object($1, $2); return 1; }
    | aliasCMD END              { create_command_object($1); return 1; }
    | data_list END             { translate($1); return 1; }

data_list:
    data_item               {$$ = new std::vector<std::string*>; $$->push_back($1); }
    | data_list data_item   {$$ = $1; $$->push_back($2);}

data_item:
    DATA                                    { $$ = $1; }
    | STRING_LITERAL                        { $$ = $1; }
    | DOLLAR OPENBRACE DATA CLOSEBRACE      { $$ = get_env_variable($3); }
