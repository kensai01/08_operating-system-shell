#include <stdio.h>
#define _POSIX_SOURCE
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/errno.h>

#include <readline/readline.h>
#include <readline/history.h>

/* Functions that will edit our environment variables. */
int SetCommand(), DeleteCommand(), PrintCommand(), PwdCommand();

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
        {"pwd", PwdCommand, "Print the current working directory." }
};

/*fwd declarations*/
char *StripWhite();
COMMAND *FindCommand();

/*done flag*/
int done;

/* Simple example of using gnu readline to get lines of input from a user.
Needs to be linked with -lreadline -lcurses add_history tells the readline
library to add the line to it's internal history, so that using up-arrow (or ^p)
will allows the user to see/edit previous lines. */


int main(int argc, char **argv) {
    char *s, *line;
    while (s=readline("prompt> ")) {
        line = StripWhite(s);
        add_history(line);
        ExecuteCommand(line);
        free(s);
        /* clean up! */
    }
    return(0);
}

/*Execute a command line. */
int ExecuteCommand(line) char *line;
{
    register int EachLetter;
    COMMAND *Command;
    char *WholeWord;

    /*Isolate the command word from the rest of the message.*/
    EachLetter = 0;
    while (line[EachLetter] && whitespace(line[EachLetter])) EachLetter++;
    WholeWord = line + EachLetter;

    while (line[EachLetter] && !whitespace(line[EachLetter])) EachLetter++;

    if (line[EachLetter]) line[EachLetter++] = '\0';

    Command = FindCommand(WholeWord);

    if(!Command)
    {
        fprintf(stderr, "%s: No such command for Project. \n", WholeWord);
        return -1;
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
        for (CommandNum = 0; Commands[CommandNum].name; CommandNum++) {
            if (strcmp(Name, Commands[CommandNum].name) == 0) return (&Commands[CommandNum]);
        }
        return ((COMMAND *) NULL);
    }
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
    char *tmp,*tmp1, *x;

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
}