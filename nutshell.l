%{
#include "nutshell.tab.h"
%}

%option noyywrap
%x EXPECT_STRING


STRING      [.A-Za-z0-9!<>|\/_-][.A-Za-z0-9!<>|\/_-]*
QUOTE       ["]
NOTQUOTE    [^"]            

%%

{QUOTE}                         BEGIN(EXPECT_STRING);
<EXPECT_STRING>{NOTQUOTE}*      { yylval.STRING_LITERAL = new std::string(yytext); return STRING_LITERAL; }
<EXPECT_STRING>{QUOTE}          BEGIN(INITIAL);


[ \t\r] {}

bye {yylval.BYE = new std::string(yytext); return BYE;}

[$] {return DOLLAR;}
[{] {return OPENBRACE;}
[}] {return CLOSEBRACE;}

setenv|printenv|unsetenv    {yylval.envCMD = new std::string(yytext); return envCMD;}
cd                          {yylval.dirCMD = new std::string(yytext); return dirCMD;}
alias|unalias               {yylval.aliasCMD = new std::string(yytext); return aliasCMD;}

{STRING} { yylval.DATA = new std::string(yytext); return DATA; }
{QUOTE}  { return QUOTE; }

"\n" {return END;}
