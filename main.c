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
int com_set(), com_delete(), com_print();

/*Structure which will contain the programs for the environment manipulation.*/
typedef struct {
    char *name;     //Function name
    int (*func)();  //Replaced the Function call with this, Function is depreciated
    char *doc;      //Tooltip
} COMMAND;

COMMAND commands[] = {
        {"set", com_set, "Set the value of the environment variable."},
        {"delete", com_delete, "Remove the environment variable."},
        {"print", com_print, "Print the value of the named environment variable."}
};

/*fwd declarations*/
char *stripwhite();
COMMAND *find_command();

/*done flag*/
int done;

/* Simple example of using gnu readline to get lines of input from a user.
Needs to be linked with -lreadline -lcurses add_history tells the readline
library to add the line to it's internal history, so that using up-arrow (or ^p)
will allows the user to see/edit previous lines. */


int main(int argc, char **argv) {
    char *s, *line;
    while (s=readline("prompt> ")) {
        line = stripwhite(s);
        add_history(line);
        execute_line(line);
        free(s);
        /* clean up! */
    }
    return(0);
}

/*Execute a command line. */
int execute_line (line) char *line;
{
    register int eachLetter;
    COMMAND *command;
    char *wholeWord;

    /*Isolate the command word from the rest of the message.*/
    eachLetter = 0;
    while (line[eachLetter] && whitespace(line[eachLetter])) eachLetter++;
    wholeWord = line + eachLetter;

    while (line[eachLetter] && !whitespace(line[eachLetter])) eachLetter++;

    if (line[eachLetter]) line[eachLetter++] = '\0';

    command = find_command(wholeWord);

    if(!command)
    {
        fprintf(stderr, "%s: No such command for Project. \n", wholeWord);
        return -1;
    }

    while (whitespace (line[eachLetter])) eachLetter++;

    wholeWord = line + eachLetter;

    return ((*(command->func)) (wholeWord));
}

/*Look up command name, return null ptr if none was found otherwise
 * return the pointer to the command.*/
COMMAND * find_command(name)
        char *name; {
    register int commandNum;
    for (commandNum = 0; commands[commandNum].name; commandNum++) {
        for (commandNum = 0; commands[commandNum].name; commandNum++) {
            if (strcmp(name, commands[commandNum].name) == 0) return (&commands[commandNum]);
        }
        return ((COMMAND *) NULL);
    }
}

/*Strip white space
 * Returns a pointer to a string.*/
char * stripwhite(string) char *string; {
    register char *s, *t;
    for(s = string; whitespace (*s); s++);
    if (*s == 0) return (s);
    t = s + strlen(s) - 1;
    while(t>s && whitespace(*t)) t--;
    *++t = '\0';
    return s;
}

/* Set the value of the environment variable to the value specified.*/
com_set(arg) char *arg;{
    //local variables to parse the incoming character array
    char *tmp,*tmp1, *x;

    /*Tokenize the char array to delimit at the equals sign*/
    /*Effectively getting the name of the environment variable to set*/
    tmp = strtok(arg, "=");
    /*Remove any remaining white space*/
    tmp = stripwhite(tmp);
    /*Print result for visual debug*/
    printf("EVarName to be set: %s\n", tmp);
    /*Tokenize the rest of the character array until the terminal character*/
    /*Effectively get the environment variable value to set*/
    tmp1 = strtok(NULL, "\0");
    /*Remove any remaining white space*/
    tmp1 = stripwhite(tmp1);
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
com_delete(arg) char *arg;{
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
com_print(arg) char *arg;{
    char *x;
    x = getenv(arg);
    printf("Environment Variable Name: %s\n", arg);
    printf("Environment Variable Value: %s\n", (x != NULL) ? x : "undefined");
    return 1;
}