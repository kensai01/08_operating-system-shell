/**Developer: Mirza Besic - BR9975
 * Dates: Started - 6/25/2017 Completed: N/A
 * All Tasks are contained within this c file.**/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <signal.h>
#include <setjmp.h>
#include <readline/readline.h>
#include <readline/history.h>

/* Functions that will edit our environment variables & launch internal commands. */
int SetCommand(), DeleteCommand(), PrintCommand(), PwdCommand(), ChangeDirCommand(), QuitCommand();

/*Structure which will contain the function calls for the environment manipulation.*/
typedef struct {
    char *name;     //Function name
    int (*func)();  //Replaced the Function call with this, Function is depreciated
    char *doc;      //Tooltip
} COMMAND;

/*Internal Commands.*/
COMMAND Commands[] = {
        {"set", SetCommand, "Set the value of the environment variable."},
        {"delete", DeleteCommand, "Remove the environment variable."},
        {"print", PrintCommand, "Print the value of the named environment variable."},
        {"pwd", PwdCommand, "Print the current working directory." },
        {"cd", ChangeDirCommand, "Change the current working directory to the supplied argument."},
        {"exit", QuitCommand, "Terminate the shell."}
};

/**Fwd declarations**/
char *StripWhite(char*);
COMMAND *FindCommand(char*);
char ExecuteCommand(char*);
void Tokenize(char *s, char** arr, char * delimiter);
void SigintHandler(int SignalNumber);
void ExecuteDestination(int pfd[], char **arr1, char **arr2);
void ExecuteSource(int pfd[], char **arr1);

/**Global Variables**/
/*Using quit flag to give us a chance to clean up after ourselves. Otherwise
 * prog would quit right from the quit function and free(s) would never get to run again.*/
int quitFlag = 0;
/* Flags for the external commands. */
int flagDot = 0;
int flagSlash = 0;
int flagInput = 0;
int flagOutput = 0;
int flagPipeInst = 0;

/*Signal handling buffer var*/
sigjmp_buf SignalBuffer;

/*Token Count*/
int TokenCount = 0;

int main(int argc, char **argv) {

    /*Signal Handling*/
    if(signal(SIGINT, SigintHandler) == SIG_ERR) { printf("Failed to register interrupts with kernel.\n"); }
    while (sigsetjmp(SignalBuffer, 1) != 0);

    /*main loop*/
    char *s, *line;
    while ((s=readline("prompt> "))){
        TokenCount = 0;
        if(quitFlag == 0) {
            line = StripWhite(s);
            add_history(line);
            ExecuteCommand(line);
            /* clean up! */
            free(line);
        }
        if(quitFlag == 1){
            exit(0);
        }
    }
}
/*Control-C handling.*/
void SigintHandler(int SignalNumber)
{
    if (SignalNumber == SIGINT){
        printf("\nYou pressed CTRL+C (Signal -> SIGINT), returning control to input terminal.\n");
        siglongjmp(SignalBuffer, 1);
    }
}
/**Executes all the tasks**/
char ExecuteCommand(line) char *line;
{
    /**General Variable Setup for generation of tokens & argument storage.**/
    register int EachLetter;
    COMMAND *Command;
    char *WholeWord;
    // Holds external command prefix
    char externalCmdPrefix = line[0];

    // output file token
    char * OutFileToken;
    // input file token
    char * InputFileToken;

    // Initial line for tokenization
    char *sentence;
    // Second line for tokenization
    char *sentence1;
    // Third line for tokenization
    char *sentence2;

    /*Allocate space and copy over all the commands.*/
    sentence = malloc(255*sizeof(char));
    strcpy(sentence, line);
    sentence1 = malloc(255*sizeof(char));
    strcpy(sentence1, line);
    sentence2 = malloc(255*sizeof(char));
    strcpy(sentence1, line);

    /*Set the flag according to which external command first element user supplied, either '.' or '/' */
    if (externalCmdPrefix == '.') { flagDot = 1; }
    if (externalCmdPrefix == '/') { flagSlash = 1; }

    /*Isolate the internal command word from the rest of the message.
     * Used for internal commands. */
    EachLetter = 0;
    while (line[EachLetter] && whitespace(line[EachLetter])) EachLetter++;
    WholeWord = line + EachLetter;
    while (line[EachLetter] && !whitespace(line[EachLetter])) EachLetter++;
    if (line[EachLetter]) line[EachLetter++] = '\0';

    /**Internal Commands flow here**/
    /*Try to find the internal command word in the available list internal commands.*/
    Command = FindCommand(WholeWord);
    if(Command){
        /*Internal Commands return here.*/
        while (whitespace (line[EachLetter])) EachLetter++;
        WholeWord = line + EachLetter;
        return ((*(Command->func)) (WholeWord));
    }

    /**External Commands flow here**/
    /*If the command supplied is NOT internal, do the external commands logic.*/
    if(!Command)
    {
        /*Storage Arrays that will hold our broken up tokenized commands.*/
        char **StorageSimpleExternalCmdArray = malloc(128 * sizeof(char*));
        char **StorageInArr = malloc(128 * sizeof(char*));
        char **StorageOutArr = malloc(128 * sizeof(char*));
        char **StorageParentArgs = malloc(128 * sizeof(char*));
        char **StoragePipelined = malloc(128 * sizeof(char*));
        char **StoragePipeChild = malloc(128 * sizeof(char*));
        char **StoragePipeWholeChildCmd = malloc(128 * sizeof(char*));

        /*Ensure memory was allocated.*/
        if (!StoragePipelined |
                !StorageSimpleExternalCmdArray |
                !StorageInArr |
                !StorageOutArr |
                !StorageParentArgs)
        {
            fprintf(stderr, "Dynamic Memory Allocation Error.\n");
            return 1;
        }

        /**Tokenization START**/
        /*Initial tokenization based on space.*/
        Tokenize(sentence, StorageSimpleExternalCmdArray, " ");

        /*Runs if it's simple pipelining of normal external cmd, ie:
         * ls -al | sort  */
        if (flagPipeInst == 1 && flagInput == 0 && flagOutput == 0) {
            /*Grab the arguments & command for child pipe.*/
            Tokenize(sentence1, StoragePipeChild, "|");
            Tokenize(StoragePipeChild[0], StorageParentArgs, " ");
            Tokenize(StoragePipeChild[1], StoragePipeWholeChildCmd, " ");

            /* What's left should be what is before the pipe line |.*/
            /*Tokenize one more time, arguments to pass to execvp.*/
            //Tokenize(sentence1, StorageParentArgs, " ");
        }
        /*Runs if we found i/o redirection and piping. ie:
         * ls -al < infile | sort > outfile */
        if (flagPipeInst == 1 && flagInput == 1 && flagOutput == 1){

            /*Do another tokenization to seperate the things we need for o redirection.*/
            Tokenize(sentence1, StoragePipelined, ">");

            /*Grab the last element, should be the output file.*/
            int ArrSize = sizeof(**StoragePipelined);
            OutFileToken = StoragePipelined[ArrSize];
            OutFileToken = StripWhite(OutFileToken);

            /*Grab the arguments & command for child pipe.*/
            Tokenize(sentence1, StoragePipeChild, "|");
            Tokenize(StoragePipeChild[1], StoragePipeChild, " ");

            /*Grab the entire child pipe (everything after the |)*/
            Tokenize(sentence2, StoragePipeWholeChildCmd, "|");
            Tokenize(StoragePipeWholeChildCmd[1],StoragePipeWholeChildCmd, " ");

            /*Tokenize again to get the input file.*/
            Tokenize(sentence1, StorageInArr, "<");

            /*Grab the last element, should be the input file.*/
            ArrSize = sizeof(**StorageInArr);
            InputFileToken = StorageInArr[ArrSize];
            InputFileToken = StripWhite(InputFileToken);

            /*Tokenize one more time, what's left should be the arguments to pass to execvp.*/
            Tokenize(sentence1, StorageParentArgs, " ");

            //printf("Input File Token: %s\n", InputFileToken);
            //printf("Output File Token: %s\n", OutFileToken);
        }
        /*Runs if we found both input & output markers during first tokenization.*/
        /* ie: sort < infile > outfile */
        if (flagInput == 1 && flagOutput == 1 && flagPipeInst == 0){
            /*Do another tokenization to seperate the things we need for i/o redirection.*/
            Tokenize(sentence1, StorageOutArr, ">");
            /*Grab the last element, should be the output file.*/
            int ArrSize = sizeof(**StorageOutArr);
            OutFileToken = StorageOutArr[ArrSize];
            OutFileToken = StripWhite(OutFileToken);
            /*Tokenize again to get the input file.*/
            Tokenize(sentence1, StorageInArr, "<");
            /*Grab the last element, should be the input file.*/
            ArrSize = sizeof(**StorageInArr);
            InputFileToken = StorageInArr[ArrSize];
            InputFileToken = StripWhite(InputFileToken);
            /*Tokenize one more time, what's left should be the arguments to pass to execvp.*/
            Tokenize(sentence1, StorageParentArgs, " ");
        }
        /*If we only found input marker.*/
        /*ie: sort < foo*/
        if (flagInput == 1 && flagOutput == 0 && flagPipeInst == 0){
            Tokenize(sentence1, StorageInArr, "<");
            int ArrSize = sizeof(**StorageInArr);
            InputFileToken = StorageInArr[ArrSize];
            InputFileToken = StripWhite(InputFileToken);
            Tokenize(sentence1, StorageParentArgs, " ");
        }
        /*If we only found output marker.*/
        /* ls > save for example..*/
        if (flagInput == 0 && flagOutput == 1 && flagPipeInst == 0){
            Tokenize(sentence1, StorageOutArr, ">");
            int ArrSize = sizeof(**StorageOutArr);
            OutFileToken = StorageOutArr[ArrSize];
            OutFileToken = StripWhite(OutFileToken);
            Tokenize(sentence1, StorageParentArgs, " ");
        }
        /**Tokenization END**/

        /**External commands START**/
        /*Setup of external command variables*/
        pid_t pid;

        /*The command is external, so check if its prefixes with a / or a ., if so
        * the user is specifying a complete path to the executable file. Also
         * checks if we have input/output redirection.*/
        if (flagSlash == 1 || flagDot == 1 || flagInput == 1 || flagOutput == 1) {
            pid = fork();
            if (pid < 0){
                printf("ERROR: %s\n", strerror(errno));
                return 1;
                }
            /**Complete file path**/
            /*No I/O Redirection, supplying complete path starting with . or /
             * ie. /bin/ls or ./test if it's an executable.*/
            else if (pid == 0 && (flagSlash == 1 || flagDot == 1)) {
                if((execvp(StorageSimpleExternalCmdArray[0],StorageSimpleExternalCmdArray)) == -1)
                {
                    printf("ERROR: %s\n", strerror(errno));
                }
                return 1;
            }

            /**I/O Redirection logic.**/
            /*Both input and output.*/
            else if (pid == 0 && (flagInput == 1 && flagOutput == 1) && (flagPipeInst == 0)){
                int InFile, OutFile;
                /*Open Input & Output files.*/
                if ((InFile = open(InputFileToken, O_RDONLY)) == -1){
                    printf("ERROR: %s\n", strerror(errno));
                    return 1;
                }
                if ((OutFile = open(OutFileToken,
                                    O_WRONLY |
                                    O_TRUNC |
                                    O_CREAT,
                                    S_IRUSR |
                                    S_IRGRP |
                                    S_IWGRP |
                                    S_IWUSR)) == -1){
                    printf("ERROR: %s\n", strerror(errno));
                    return 1;
                }
                /*Replace file descriptors /w dup2.*/
                if ((dup2(InFile, STDIN_FILENO)) == -1){
                    printf("ERROR: %s\n", strerror(errno));
                    return 1;
                }
                if ((dup2(OutFile, STDOUT_FILENO)) == -1){
                    printf("ERROR: %s\n", strerror(errno));
                    return 1;
                }
                /*Close open files.*/
                if(close(InFile) == -1){
                    printf("ERROR: %s\n", strerror(errno));
                    return 1;
                }
                if(close(OutFile) == -1){
                    printf("ERROR: %s\n", strerror(errno));
                    return 1;
                }

                if ((execvp(StorageParentArgs[0],StorageParentArgs)) == -1){
                    printf("ERROR: %s\n", strerror(errno));
                    return 1;
                }
            }
                /*Input Redirection only.*/
            else if (pid == 0 && (flagInput == 1 && flagOutput == 0) && (flagPipeInst == 0)){
                int InFile;
                /*Open Input & Output files.*/
                if ((InFile = open(InputFileToken, O_RDONLY)) == -1){
                    printf("ERROR: %s\n", strerror(errno));
                    return 1;
                }
                /*Replace file descriptors /w dup2.*/
                if ((dup2(InFile, STDIN_FILENO)) == -1){
                    printf("ERROR: %s\n", strerror(errno));
                    return 1;
                }
                /*Close open files.*/
                if (close(InFile) == -1){
                    printf("ERROR: %s\n", strerror(errno));
                    return 1;
                }

                if ((execvp(StorageParentArgs[0],StorageParentArgs)) == -1){
                    printf("ERROR: %s\n", strerror(errno));
                    return 1;
                }
            }
                /*Output Redirection only.*/
            else if (pid == 0 && (flagInput == 0 && flagOutput == 1) && (flagPipeInst == 0)){
                int OutFile;
                /*Open Input & Output files.*/
                if ((OutFile = open(OutFileToken,
                                    O_WRONLY |
                                    O_TRUNC |
                                    O_CREAT,
                                    S_IRUSR |
                                    S_IRGRP |
                                    S_IWGRP |
                                    S_IWUSR)) == -1){
                    printf("ERROR: %s\n", strerror(errno));
                    return 1;
                }
                /*Replace file descriptors /w dup2.*/
                if ((dup2(OutFile, STDOUT_FILENO)) == -1){
                    printf("ERROR: %s\n", strerror(errno));
                    return 1;
                }
                /*Close open files.*/
                if(close(OutFile) == -1){
                    printf("ERROR: %s\n", strerror(errno));
                    return 1;
                }

                if ((execvp(StorageParentArgs[0],StorageParentArgs)) == -1){
                    printf("ERROR: %s\n", strerror(errno));
                    return 1;
                }
            }

            else {
                int cs;
                if(wait(&cs) == -1){
                    printf("ERROR: %s\n", strerror(errno));
                    return 1;
                }
                /*Reset the i/o markers*/
                flagInput = 0;
                flagOutput = 0;
                /*Reset External Command markers.*/
                flagDot = 0;
                flagSlash = 0;
                flagPipeInst = 0;
                return 0;
            }
        }

        /**Piping Logic**/
        if (flagPipeInst == 1){
            int status;
            pid_t pipePid;
            int pipefd[2];

            /*Create the pipe.*/
            if (pipe(pipefd) == -1){
                printf("ERROR pipe: %s\n", strerror(errno));
                //return 1;
            }
            /*Handle first portion before | */
            ExecuteSource(pipefd, StorageParentArgs);
            /*Handle second portion after | */
            ExecuteDestination(pipefd, StoragePipeChild, StoragePipeWholeChildCmd);

            if (close(pipefd[0]) == -1){
                printf("ERROR close: %s\n", strerror(errno));
                return 1;
            }
            if (close(pipefd[1]) == -1){
                printf("ERROR close: %s\n", strerror(errno));
                return 1;
            }

            while ((pipePid = wait(&status)) != -1) //Reap kids.
                fprintf(stderr, "Process %d exits with %d\n", pipePid, WEXITSTATUS(status));

            //Reset piping flag.
            flagPipeInst = 0;
            return 0;

            /*Piping with input redirection, run first child.
             * ie: ls -al < infile | cat for example.*//*
            //TODO implement piping /w input redirection

            *//*Piping with output redirection, run first child.
             * ie: ls -al | sort > outfile for example.*//*
            //TODO implement piping /w output redirection

            *//*Piping with Input & Output redirection, run first child.
            * ie: ls -al < infile | sort > outfile for example.*/
            //TODO implement piping /w input & output redirection

        }
        /**Simple external cmd**/
        /*Simple external command that is not I/O redirect & not a . or / prefixed command.
         * ie. ls -al or sort ...*/
        else {
            pid = fork();
            if (pid < 0){
                printf("ERROR: %s\n", strerror(errno));
                exit(1);
            }
            else if (pid == 0) {
                if(execvp(StorageSimpleExternalCmdArray[0],StorageSimpleExternalCmdArray) == -1){
                    printf("ERROR: %s\n", strerror(errno));
                    exit(1);
                }
            }
            else {
                int cs;
                if(wait(&cs) == -1){
                    printf("ERROR: %s\n", strerror(errno));
                    return 1;
                }
                return 0;
            }
        }
    }
    return 0;
}

/*Second part of the pipe ie. what comes after | */
void ExecuteDestination(int pipefd[], char **ChildArgArrOne, char **ChildArgArrTwo)
{
    pid_t pid;
    switch (pid = fork()) {
        case 0:
            /*Replace stdin with input part of pipe.*/
            if(dup2(pipefd[0], STDIN_FILENO) == -1){
                printf("ERROR dup2: %s\n", strerror(errno));
                exit(1);
            }
            /*Close the other pipe.*/
            if (close(pipefd[1]) == -1)
            {
                printf("ERROR dup2: %s\n", strerror(errno));
                exit(1);
            }
            /*Execute the first pipe.*/
            if(execvp(ChildArgArrOne[0], ChildArgArrTwo) == -1){
                printf("ERROR execvp: %s\n", strerror(errno));
                exit(1);
            }
        default:
            break;

        case -1:
            printf("ERROR fork: %s\n", strerror(errno));
            exit(1);
    }
}

/*First part of the pipe, what comes before |*/
void ExecuteSource(int pipefd[], char **ParentArgArray)
{
    pid_t pid;
    switch (pid = fork()) {
        case 0:
            /*Replace stdin with input part of pipe.*/
            if(dup2(pipefd[1], STDOUT_FILENO) == -1){
                printf("ERROR: %s\n", strerror(errno));
                exit(1);
            }
            /*Close the other pipe.*/
            if (close(pipefd[0]) == -1)
            {
                printf("ERROR: %s\n", strerror(errno));
                exit(1);
            }
            /*Execute the first pipe.*/
            if(execvp(ParentArgArray[0], ParentArgArray) == -1){
                printf("ERROR execvp: %s\n", strerror(errno));
                exit(1);
            }
        default:
            break;

        case -1:
            printf("ERROR fork: %s\n", strerror(errno));
            exit(1);
    }
}

/*Look up command name, return null ptr if none was found otherwise
 * return the pointer to the command.*/
COMMAND * FindCommand(Name) char *Name; {
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
int SetCommand(arg) char *arg;{
    /*Local Variable Setup*/
    char **TokenizedArguments = malloc(128 * sizeof(char*));
    char * ArgumentCopy;
    ArgumentCopy = malloc(255*sizeof(char));
    strcpy(ArgumentCopy, arg);
    /*Split incomming arguments via space.*/
    Tokenize(ArgumentCopy, TokenizedArguments, " ");
    //local variables to parse the incoming character array
    char *tmp,*tmp1;
    int i;
    bool spaceChar = false;
    while(TokenizedArguments[i] != NULL){
        if(strcmp(TokenizedArguments[i], "=") == 0){
            spaceChar = true;
        }
        i++;
    }
    if(spaceChar == false) {printf("ERROR: use is 'name = variable'\n"); return 1;}

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
    if (setenv(tmp,tmp1,1) == -1)
    {
        printf("ERROR: %s\n", strerror(errno));
        return 1;
    }
    else
        printf("Environment variable set.\n");
    return 0;
}

/* Delete the named environment variable. */
int DeleteCommand(arg) char *arg;{
    char *x;
    if((x = getenv(arg)) == NULL)
    {
        printf("Unable to get environment variable! Unable to complete delete operation.\n");
        return 1;
    }
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
int PrintCommand(arg) char *arg;{
    char *EnvironmentVariable;
    EnvironmentVariable = getenv(arg);
    if(EnvironmentVariable == NULL) {
        printf("Environment Variable does not exist.");
        return 1;
    }

    printf("Environment Variable Name: %s\n", arg);
    printf("Environment Variable Value: %s\n", (EnvironmentVariable != NULL) ? EnvironmentVariable : "undefined\n");
    return 1;
}

int PwdCommand(arg) char *arg; {
    char Directory[2048], *DirectoryString;
    DirectoryString = getcwd(Directory, 2048);
    if (DirectoryString == NULL){
        printf("ERROR: %s\n", strerror(errno));
        return 1;
    }
    printf("Current Directory: %s\n",Directory);
    return 0;
}

int ChangeDirCommand(arg) char *arg; {
    /*Ensure that we don't get an error back when changing directories.*/
    if (chdir(arg) == -1){
        printf("ERROR: %s\n", strerror(errno));
        return 1;
    }
        /* Otherwise get the new directory and print it.*/
    else
        PwdCommand("");
        return 0;
}

/*Sets the quit flag on so that the program can clean up and exit.*/
int QuitCommand(arg) char *arg; {
    /*Set the quit flag on.*/
    quitFlag = 1;
    return 0;
}
/*Tokenizer, splits the arguments and detects presence of i/o or piping.*/
void Tokenize(char *sentence, char** StorageArray, char * delimiter)
{
    char *token;
    char *next_token;
    int counter = 1, position = 0;
    token = strtok(sentence, delimiter);
    next_token = token;
    while (next_token != NULL){
        TokenCount++;
        StorageArray[position] = token;

        /*Check for input / output markers*/
        if(strcmp(StorageArray[position], "<") == 0){ flagInput = 1;}
        if(strcmp(StorageArray[position], ">") == 0){ flagOutput = 1;}
        /*Check for pipeline instructions.*/
        if(strcmp(StorageArray[position], "|") == 0){ flagPipeInst = 1;}

        position++;
        if((next_token = strtok(NULL, delimiter))){
            token = next_token;
        }
        counter++;
    }
    counter--;
}



