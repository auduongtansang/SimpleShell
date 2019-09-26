#include <stdio.h>  //standard I/O
#include <stdlib.h>  //exit(), malloc()
#include <string.h>  //strtok()
#include <unistd.h>  //fork()
#include <sys/wait.h>  //wait()
#include <sys/types.h>  //pid_t

#define MAX_LEN 256
#define MAX_ARG 256

char* readCommand()
{
    //Allocate mem
    char *command = (char*)malloc(MAX_LEN * sizeof(char));
    //Read command from standard input
    fgets(command, MAX_LEN, stdin);
    //Remove '\n' at the end of the command string (because of the fgets())
    command[strlen(command) - 1] = '\0';
    return command;
}

char** splitCommand(char *command, char sign[])
{
    //Allocate mem for argument array
    char **args = (char**)malloc(MAX_ARG * sizeof(char*));
    //Parse command by strtok()
    char argsCount = 0;
    char *token = strtok(command, sign);
    while (token != NULL)
    {
        args[argsCount++] = token;
        token = strtok(NULL, sign);
    }
    //Add NULL at the end of the array
    args[argsCount] = NULL;
    return args;
}

int runCommand(char **args)
{
    pid_t childID, waitID;
    int status;
    //Create a child process by fork(), to execute the splited command (args)
    childID = fork();
    if (childID == 0)
    {
        //Inside child process
        execvp(args[0], args);  //execvp() will automatic kill the child process after done
        //If execvp() failed, print error, then kill the child process manualy
        perror("execvp() error");
        exit(-1);
    }
    else if (childID > 0)
    {
        //Inside parent process
        do
        {
            //Wait for the child process to be killed
            waitID = wait(&status);
        } while (waitID != childID);
    }
    else
    {
        //Cannot create child process
        perror("fork() error");
    }
    return 0;
}

void mainLoop(void)
{
    char *command = NULL;
    char **args = NULL;
    //Loop until command "exit" is received
    while (1)
    {
        printf("HahaOS > ");
        command = readCommand();
        args = splitCommand(command, " ");

        if (strcmp(args[0], "exit") == 0)
            break;

        runCommand(args);
        free(command);
        free(args);
    }
}

int main(void)
{
    mainLoop();
    return 0;
}