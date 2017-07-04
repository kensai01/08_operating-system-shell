#CSC4420 Program 1 Readme

#File Descriptions:
1.	Main.c -> Contains the program, all the code is within this file. 

#How to run program:
Create a make file & run it. Use -lreadline and -lcurses. After the makefile has been executed, run the output file to start the shell.
	
	Example: 
		username@username:~/Desktop/CSC4420P1$ ./SimpleShell

#Design Notes
##Task 1: 
Setting an environmental variable is currently configured to allow spaces in the name or value. This is a design decision on my part because setenv allows it. If setenv threw an error code doing this I would have limited the number of tokens to 3 to ensure that it only contained the ‘name=value’ format. Currently it allows ‘name name name=value value value’ etc. also as well as ‘name=value’ of course. 
Works Fully: set, delete, print

##Task 2:
Works Fully: pwd, cd [dir], exit

##Task 3:
Works Fully: If command name starts with . or / ie. /bin/ls -al or ./test.o, also if user supplies file name ie. ls -al etc all works. Unknown commands do not crash the console. 

##Task 4:
Works Fully: Only Input Redirection works ie. sort < infile, only Output redirection works ie. ls > save and both input and output redirection works. Ie. sort < infile > outfile.

##Task 5:
Works Partially: Basic piping works such as ls | sort, piping with additional arguments and/or input redirection does not (see deficiencies).

##Task 6:
Works Fully: Control-C does not terminate shell, terminates foreground command currently running and prints a message indicating that SIGINT terminated the process.

#Problems, bugs and deficiencies:

##General Issues:
The primary problem was creating the tokenizer function; this took considerable effort. The outstanding problem I have not been able to solve is related to piping and input redirection. The other general problems were minor and mostly related to my unfamiliarity with C.

##Known Bugs / Deficiencies:
After certain commands fail (piping), using Control-C returns user to the input prompt. In some instances I haven’t been able to replicate consistently, the user has to then press enter again in order for the prompt> to show up again. Minor issue.

Program hangs when you run “sort” by itself, without any input or output. Pressing Control-C returns user to the input prompt without crashing the shell. However, I haven’t been able to figure out why ‘sort’ in particular hangs the process while unknown commands like ‘booboomcgooboo’ for example return an error and prompt for further input. Minor issue.

Only basic piping works such as “ls | sort”. Adding additional arguments or input redirection hangs the process and can cause unknown behavior that doesn’t crash the shell but makes it unresponsive even after a “Control-C”. This is a known behavior because I have not been able to fully implement pipes correctly, I tried my best but I simply have run out of time. I feel like I am really close too. Theoretically, I think my piping doesn’t work with more arguments because I need to create a pipe for each argument passed in rather than one pipe for an array of arguments. I don’t have the time to try this as it dawned on me just recently and I’ve been going in the wrong direction for a while.


References:
https://www.ibm.com/support/knowledgecenter/en/SSB23S_1.1.0.13/gtpc2/cpp_setenv.html#cpp_setenv – used for Task 1
https://www.ibm.com/support/knowledgecenter/en/SSLTBW_2.1.0/com.ibm.zos.v2r1.bpxbd00/setenv.htm - used for Task 1
http://web.mit.edu/gnu/doc/html/rlman_2.html - Basis for Task 1 / 2 & 3
https://stackoverflow.com/questions/43797645/c-function-is-deprecated - Fixing stuff
https://stackoverflow.com/questions/4334969/parsing-an-array-of-chars - Tokenization
https://www.ibm.com/support/knowledgecenter/en/SSB23S_1.1.0.14/gtpc2/cpp_unsetenv.html - Task 1
https://stackoverflow.com/questions/19074456/how-to-store-temp-value-for-strtok-without-changing-the-initial-token - Tokenization
https://stackoverflow.com/questions/11818491/dynamic-parameter-passing-to-execlp-function-in-c - Fixing stuff
www.cs.loyola.edu/~jglenn/702/S2005/Examples/dup2.html - Task 4
https://stackoverflow.com/questions/16828378/readline-get-a-new-prompt-on-sigint  - Task 6
http://www.unix.com/programming/122360-c-piping-redirect-operator.html - Task 5
https://www.cs.rutgers.edu/~pxk/416/notes/c-tutorials/pipe.html - Task 5

