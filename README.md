Implemented:
● setenv variable word ---- This command sets the value of the variable variable to be word.
● printenv ---- This command prints out the values of all the environment variables, in the format
variable=value, one entry per line.
● unsetenv variable ---- This command will remove the binding of variable. If the variable is
unbound, the command is ignored.
● Mandatory environment variables include HOME which shows the home directory of the user
and PATH which shows the list of paths to be searched to find a command’s executable file.
● Specific conventions regarding the PATH environment variable. Your shell should interpret
the value of PATH to be a list of colon-separated words (for instance, word:word:word). Your
shell should reparse and do tilde (~) expansion at the beginning (first character) of each of these
words whenever the value of the variable is reset
● cd word ---- This command changes the current directory to word, where word is a directory name.
The directory name may be absolute (starts with root which is /) or relative to the current
directory. You must handle cd with no arguments, to take you back to the home directory, i.e., it
should have the same effect as cd ~ (see Tilde Expansion).
● alias name word ---- Adds a new alias to the shell. See the subsection on aliases for more
information.
● unalias name ---- Remove the alias for name.
● alias ---- lists all available aliases
● bye ---- Gracefully quit the shell. The shell should also exit if it receives the end-of-file character.