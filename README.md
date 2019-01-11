# Command Shell
==================================

This is  a simplified version of a command shell.
I use C++ for this program, so that I can demonstrate object orientation programming.


Part 1. Running command
-----------------------

   1. user can either give the command with a relative or absolute path
   or give the command itself, in this case, myShell
   will search the PATH of user's system
   2. user can add arguments after their commandname
   3. user can type "exit" or use eof to quit the shell
   4. myShell will print the current directory at its prompt.
   5. user can type whatever number of spaces

Part 2. Directories and variables
---------------------------------

   1. Support the "cd" command to change the current directory.
   2. My shell provides access to variables. A variable name
      must be a combination of letters (case sensitive), underscores,
      and numbers, (e.g., PATH, xyz, abc42, my_var).  Note that
      $var1$var2 is legal: if var1=a and var2=b, then $var1$var2
      expands to ab.  Likewise, $var1- is legal: it is $var1 followed
      immediately by - (so again if var1=a, it would be a-).
   3. If the user writes $varname on the command line, my shell will
      replace it with the current value of that variable.
   4. My shell provides three built in commands:
      - set var value
        This set thes variable var to the string on the rest of the
	command line (even if it contains spaces, etc.) My shell will
	remember this value, and make use of it in future $ evaluations;
	however, it should not be placed in the environment for other
	programs.
      - export var
        This puts the current value of var into the environment for
	other programs.
      - inc var
        Interprets the value of var as a number (base10), and increments it.
        If var is not currently set, or does not represent a number
	in base10, then the intiial value is treated as 0 (and thus
        incremented to 1)
	  For example
	    set v hello
	    //v is now "hello"
	    inc v
	    //v is now "1"
	    inc v
	    //v is now "2"
   5. If user "set" a variable (but dont "export" it) the new value should
      not show up in "env." If user "export" it, the new value should show
      up in "env".

  Note also that if the user changes PATH, it should affect where my shell
  searches for programs.

Part 3: Pipes and redirection
-----------------------------

  - Implement input redirection (<) and output redirection (>)
      < filename   redirects standard input for the command
      > filename   redirects standard ouput
      2> filename  redirects standard error

  - Implement pipes (|)
    My shell is able to run one command and pipe its output to another's
    input:
      ./myProgram | ./anotherProgram

  Note that my shell is able to mix and match these in ways that make
  sense, along with having command line arguments:
    ./myProgram a b c < anInputFile | ./anotherProgram 23 45 > someOutputFile
