#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <sys/wait.h>

#include <readline/readline.h>
#include <readline/history.h>

/* Functions that will edit our environment variables. */
int SetCommand(), DeleteCommand(), PrintCommand(), PwdCommand(), ChangeDirCommand(), QuitCommand();

/*Structure which will contain the programs for the environment manipulation.*/
typedef struct {
    char *name;     //Function name
    int (*func)();  //Replaced the Function call with this, Function is depreciated
    char *doc;      //Tooltip
} COMMAND;

COMMAND Commands[] = {
        {"set", SetCommand, "Set the value of the environment variable."},
        {"delete", DeleteCommand, "Remove the environment variable."},
        {"print", PrintCommand, "Print the value of the named environment variable."},
        {"pwd", PwdCommand, "Print the current working directory." },
        {"cd", ChangeDirCommand, "Change the current working directory to the supplied argument."},
        {"exit", QuitCommand, "Terminate the shell."}
};

/*fwd declarations*/
char *StripWhite(char*);
COMMAND *FindCommand();

/* Simple example of using gnu readline to get lines of input from a user.
Needs to be linked with -lreadline -lcurses add_history tells the readline
library to add the line to it's internal history, so that using up-arrow (or ^p)
will allows the user to see/edit previous lines. */

/*Using quit flag to give us a chance to clean up after ourselves. Otherwise
 * prog would quit right from the quit function and free(s) would never get to run again.*/
int quitFlag = 0;

int main(int argc, char **argv) {
    char *s, *line;
    while (s=readline("prompt> ")){
        if(quitFlag == 0) {
            line = StripWhite(s);
            add_history(line);
            ExecuteCommand(line);
            /* clean up! */
            free(s);
        }
        if(quitFlag == 1){
            exit(0);
        }
    }
}

/*Execute a command line. */
ExecuteCommand(line) char *line;
{
    register int EachLetter;
    COMMAND *Command;
    char *WholeWord;
    //printf("line: %c\n", line[0]);
    printf("WholeWord: %s\n", line);
    char this = line[0];
    int flagDot = 0;
    int flagSlash = 0;
    char *sentence;
    sentence = malloc(255*sizeof(char));
    strcpy(sentence, line);
    printf("%s\n", sentence);

    /*Set the flag according to which external command first element user supplied */
    if (this == '.') { flagDot = 1; printf("Dot Flag Set\n");}
    if (this == '/') { flagSlash = 1; printf("Slash Flag Set\n");}

    /*Isolate the command word from the rest of the message.*/
    EachLetter = 0;
    while (line[EachLetter] && whitespace(line[EachLetter])) EachLetter++;
    WholeWord = line + EachLetter;

    while (line[EachLetter] && !whitespace(line[EachLetter])) EachLetter++;

    if (line[EachLetter]) line[EachLetter++] = '\0';

    Command = FindCommand(WholeWord);
    /*Check to see if command supplied is internal or external.*/
    if(!Command)
    {
        char *token;
        char *next_token;
        int counter = 1, position = 0;
        char **StorageArray = malloc(128 * sizeof(char*));

        if (!StorageArray) {
            fprintf(stderr, "Dynamic Memory Allocation Error.\n");
            exit(EXIT_FAILURE);
        }
        token = strtok(sentence, " ");
        next_token = token;
        while (next_token != NULL){
            printf("Token %d: %s\n", counter, token);
            StorageArray[position] = token;
            position++;
            if(next_token = strtok(NULL, " ")){
                token = next_token;
            }
            counter++;
        }
        counter--;
        printf("Token %d: %s\n", counter, token);
        printf("StorageArray: %s\n", StorageArray[0]);
        printf("StorageArray: %s\n", StorageArray[1]);
        printf("StorageArray: %s\n", StorageArray[2]);

        pid_t pid;
        /*The command is external, so check if its prefixes with a / or a ., if so
        * the user is specifying a complete path to the executable file. */
        if (flagSlash == 1 || flagDot == 1) {
            pid = fork();
            if (pid < 0){
                printf("Fork Failed\n");
                return 1;
                }
            else if (pid == 0) {
                printf("My PID is %d\n", getpid());
                printf("Complete Path Supplied.\n");
                printf("Local PID %d\n", pid);
                /*User is supplying the complete path.*/
                execvp(StorageArray[0],StorageArray);
                }

            else {
                printf("My PID is %d\n", getpid());
                printf("Local PID %d\n", pid);
                int cs;
                wait(&cs);
                printf("Child Process Complete. Status %d\n", cs);
                return 0;
            }
        }
        else {
            pid = fork();
            if (pid < 0){
                printf("Fork Failed\n");
                return 1;
            }
            else if (pid == 0) {
                printf("My PID is %d\n", getpid());
                printf("Executable Name Supplied.\n");
                printf("Local PID %d\n", pid);;
                /*User is supplying the executable file name.*/
                execvp(StorageArray[0],StorageArray);
            }

            else {
                printf("My PID is %d\n", getpid());
                printf("Local PID %d\n", pid);
                int cs;
                wait(&cs);
                printf("Child Process Complete. Status %d\n", cs);
                return 0;
            }
        }
    }

    while (whitespace (line[EachLetter])) EachLetter++;

    WholeWord = line + EachLetter;

    return ((*(Command->func)) (WholeWord));
}

/*Look up command name, return null ptr if none was found otherwise
 * return the pointer to the command.*/
COMMAND * FindCommand(Name)
        char *Name; {
    register int CommandNum;

        for (CommandNum = 0; Commands[CommandNum].name; CommandNum++) {
            if (strcmp(Name, Commands[CommandNum].name) == 0) return (&Commands[CommandNum]);
        }
        return ((COMMAND *) NULL);

}

/*Strip white space
 * Returns a pointer to a string.*/
char * StripWhite(string) char *string; {
    register char *s, *t;
    for(s = string; whitespace (*s); s++);
    if (*s == 0) return (s);
    t = s + strlen(s) - 1;
    while(t>s && whitespace(*t)) t--;
    *++t = '\0';
    return s;
}

/* Set the value of the environment variable to the value specified.*/
SetCommand(arg) char *arg;{
    //local variables to parse the incoming character array
    char *tmp,*tmp1;

    /*Tokenize the char array to delimit at the equals sign*/
    /*Effectively getting the name of the environment variable to set*/
    tmp = strtok(arg, "=");
    /*Remove any remaining white space*/
    tmp = StripWhite(tmp);
    /*Print result for visual debug*/
    printf("EVarName to be set: %s\n", tmp);
    /*Tokenize the rest of the character array until the terminal character*/
    /*Effectively get the environment variable value to set*/
    tmp1 = strtok(NULL, "\0");
    /*Remove any remaining white space*/
    tmp1 = StripWhite(tmp1);
    /*Print result for visual debug*/
    printf("EVarVal to be set: %s\n", tmp1);
    /*Set environment variable based on supplied parameters.*/
    if (setenv(tmp,tmp1,1) == 0)
    {
        printf("Environment variable set.\n");
        return 0;
    }
    else
        printf("Unable to set Environment Variable. Please try again.\n");
    return 0;
}

/* Delete the named environment variable. */
DeleteCommand(arg) char *arg;{
    char *x;
    x = getenv(arg);
    printf("Environment Variable to be Deleted: %s\n", arg);
    printf("Environment Variable Value before Deletion: %s\n", x);
    if(unsetenv(arg) == 0) {
        x = getenv(arg);
        printf("Environment Variable Value after Deletion Attempt: %s\n", (x != NULL) ? x : "undefined");
        return 0;
    }
    else
        printf("Unable to Delete Environment Variable. Please Try Again.\n");
    return 1;
}

/*Print the named environment variable. */
PrintCommand(arg) char *arg;{
    char *x;
    x = getenv(arg);
    printf("Environment Variable Name: %s\n", arg);
    printf("Environment Variable Value: %s\n", (x != NULL) ? x : "undefined");
    return 1;
}

PwdCommand(arg) char *arg; {
    char Directory[2048], *DirectoryString;
    DirectoryString = getcwd(Directory, 2048);
    if (DirectoryString == 0){
        printf("error");
    }
    printf("Current Directory: %s\n",Directory);
    return 0;
}

ChangeDirCommand(arg) char *arg; {
    /*Ensure that we don't get an error back when changing directories.*/
    if (chdir(arg) == -1){
        perror(arg);
        return 1;
    }
        /* Otherwise get the new directory and print it.*/
    else
        PwdCommand("");
        return 0;
}

/*Sets the quit flag on so that the program can clean up and exit.*/
QuitCommand(arg) char *arg; {
    /*Set the quit flag on.*/
    quitFlag = 1;
    return 0;
}