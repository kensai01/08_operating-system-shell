# OperatingSystemShell
Project one for CSC4420, Operating Systems - Create a basic shell based on 5 outlined tasks. 

# Citations
https://www.ibm.com/support/knowledgecenter/en/SSB23S_1.1.0.13/gtpc2/cpp_setenv.html#cpp_setenv
https://www.ibm.com/support/knowledgecenter/en/SSLTBW_2.1.0/com.ibm.zos.v2r1.bpxbd00/setenv.htm
http://web.mit.edu/gnu/doc/html/rlman_2.html
https://stackoverflow.com/questions/43797645/c-function-is-deprecated
https://stackoverflow.com/questions/4334969/parsing-an-array-of-chars
https://www.ibm.com/support/knowledgecenter/en/SSB23S_1.1.0.14/gtpc2/cpp_unsetenv.html
https://stackoverflow.com/questions/19074456/how-to-store-temp-value-for-strtok-without-changing-the-initial-token
https://stackoverflow.com/questions/11818491/dynamic-parameter-passing-to-execlp-function-in-c
www.cs.loyola.edu/~jglenn/702/S2005/Examples/dup2.html
https://stackoverflow.com/questions/16828378/readline-get-a-new-prompt-on-sigint
TODO - put in redirection pages


#Known Bugs / Deficiencies:
After certain commands fail, using Control-C returns user to the input prompt. In some instances I haven't been able to replicate consistantly the user has to then press enter again in order for the prompt> to show up. Minor issue.

Program hangs when you run "sort" without any input or output. Pressing Control-C returns user to the input prompt without crashing the shell. However, I haven't been able to figure out why 'sort' in particular hangs the process while other unknown commands like 'booboomcgooboo' for example return an error and prompt for further input. Minor issue.

Piping doesn't work correctly. Running ls -al | sort prints only the ls portion of the pipe. Major issue.

Piping in combination with input redirection does not work at this time. Major issue.
