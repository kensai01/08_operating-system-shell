#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/errno.h>

#include <readline/readline.h>
#include <readline/history.h>

/*Based on the fileman example from the GNU Readline man pages,*/
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
        add_history(s);
        /* adds tt
        /* clean up! */
    }
    return(0);
}

/*Look up command name, return null ptr if none was found otherwise
 * return the pointer to the command.*/
COMMAND * find_command(name)
    char *name; {
    register int commandNumIterator;
    for (commandNumIterator = 0; commands[commandNumIterator].name; commandNumIterator++) {
    for (commandNumIterator = 0; commands[commandNumIterator].name; commandNumIterator++) {
        if (strcmp(name, commands[commandNumIterator].name) == 0) return (&commands[commandNumIterator]);
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
    printf("Executed com_set.");
}

/* Delete the named environment variable */
com_delete(arg) char *arg;{
    printf("Executed com_delete.");
}

com_print(arg) char *arg;{
    printf("Executed com_print.");
}