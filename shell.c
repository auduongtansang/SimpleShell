#include <stdio.h>  //standard I/O
#include <stdlib.h>  //exit(), malloc()
#include <string.h>  //strtok()
#include <unistd.h>  //fork()
#include <fcntl.h>  //open()
#include <sys/wait.h>  //wait()
#include <sys/types.h>  //pid_t

#define MAX_LEN 256
#define MAX_ARG 256

char* readCommand(void)
{
    //Allocate memory
    char *command = (char*)malloc(MAX_LEN * sizeof(char));
    //Read command from standard input
    fgets(command, MAX_LEN, stdin);
    //Remove '\n' at the end of the string (because of the fgets())
    command[strlen(command) - 1] = '\0';
    return command;
}

char** parseCommand(char *command)
{
    //Allocate memory
    char **args = (char**)malloc(MAX_ARG * sizeof(char*));
    //Parsing by strtok()
    char sign[] = " ";
    char argsCount = 0;
    char *token = strtok(command, sign);
    while (token != NULL)
    {
        args[argsCount++] = token;
        token = strtok(NULL, sign);
    }
    //Add NULL argument at the end
    args[argsCount] = NULL;
    return args;
}

int isConcurrent(char **args)
{
    //Find "&" argument
    int iter = 0;
    while (args[iter] != NULL && strcmp(args[iter], "&") != 0)
        iter += 1;
    //Not found
    if (args[iter] == NULL)
        return 0;
    //Found
    args[iter] = NULL;
    return 1;
}

int isRedirect(char **args, char *filePath)
{
    //Find any "<", ">", "2>" argument
    int iter = 0;
    while (args[iter] != NULL && strcmp(args[iter], "<") != 0 && strcmp(args[iter], ">") != 0 && strcmp(args[iter], "2>") != 0)
        iter += 1;
    //Not found
    if (args[iter] == NULL)
        return 0;
    //Found
    strncpy(filePath, args[iter + 1], strlen(args[iter + 1]) + 1);
    if (strcmp(args[iter], "<") == 0)
    {
        //Redirect input
        args[iter] = NULL;
        return 1;
    }
    else if (strcmp(args[iter], ">") == 0)
    {
        //Redirect output
        args[iter] = NULL;
        return 2;
    }
    //Redirect error
    args[iter] = NULL;
    return 3;
}

int isPipe(char **args, char **args1)
{
    //Find "|" argument
    int iter = 0;
    while (args[iter] != NULL && strcmp(args[iter], "|") != 0)
        iter += 1;
    //Not found
    if (args[iter] == NULL)
        return 0;
    //Found
    int oldIter = iter;
    do
    {
        iter += 1;
        args1[iter - oldIter - 1] = args[iter];
    } while (args[iter] != NULL);
    args[oldIter] = NULL;
    return 1;
}

int execCommand(char **args, char **args1)
{
    int exitStatus;
    pid_t childID, waitID;

    int concurrent = isConcurrent(args);

    char *reFile = (char*)malloc(MAX_LEN * sizeof(char));
    int redirect = isRedirect(args, reFile);
    int reFileDes = -1;

    int pip = isPipe(args, args1);
    int piFileDes[2];
    pid_t piChildID;

    //Fork a child process
    childID = fork();
    if (childID == 0)
    {
        //Inside child process
        //Check redirect
        switch (redirect)
        {
            case 1:
            {
                //Standard input
                reFileDes = open(reFile, O_RDONLY);
                free(reFile);
                if (reFileDes < 0)
                {
                    fprintf(stderr, "No such file or directory\n");
                    exit(-1);
                }
                dup2(reFileDes, 0);
                break;
            }
            case 2:
            {
                //Standard output
                reFileDes = open(reFile, O_WRONLY | O_CREAT | O_TRUNC, 0777);
                free(reFile);
                dup2(reFileDes, 1);
                break;
            }
            case 3:
            {
                //Standard error
                reFileDes = open(reFile, O_WRONLY | O_CREAT | O_TRUNC, 0777);
                free(reFile);
                dup2(reFileDes, 2);
                break;
            }
        }
        //Check pipe
        if (pip == 1)
        {
            if (pipe(piFileDes) < 0)
            {
                //Cannot make pipe
                perror("pipe() error");
                exit(-1);
            }
            //Fork a child of the child process
            piChildID = fork();
            if (piChildID == 0)
            {
                //Inside the child of the child
                //Make this process write to piFileDes[1]
                dup2(piFileDes[1], 1);
                close(piFileDes[0]);
                //Execute command
                execvp(args[0], args);
                //Execute failed
                perror("execvp() error");
                exit(-1);
            }
            else if (piChildID > 0)
            {
                //Inside child process
                do
                {
                    //Wait for piChildID process is done
                    waitID = wait(&exitStatus);
                } while (waitID != piChildID);
                //Make this process read from piFileDes[0]
                dup2(piFileDes[0], 0);
                close(piFileDes[1]);
                //Excute command
                execvp(args1[0], args1);
                //Excute failed
                perror("execvp() error");
                exit(-1);
            }
            else
            {
                //Cannot fork a child of the child process
                perror("fork() error");
                exit(-1);
            }
        }

        //No pipe
        //execvp() will kill the child process automatically after done
        execvp(args[0], args);
        //If execvp() failed, print error, then kill the child process manually
        perror("execvp() error");
        exit(-1);
    }
    else if (childID > 0)
    {
        //Inside parent process
        if (concurrent == 1)
            return 0;
        do
        {
            //No "&" argument, wait for the child process to be killed
            waitID = wait(&exitStatus);
        } while (waitID != childID);
    }
    else
    {
        //Cannot fork a child process
        perror("fork() error");
        exit(-1);
    }
    return 0;
}

void mainLoop(void)
{
    char *command = NULL;
    char **args = NULL;
    char **args1 = (char**)malloc(MAX_ARG * sizeof(char*));
    char *history = (char*)malloc(MAX_LEN * sizeof(char));
    history[0] = '\0';
    //Loop until command "exit" is received
    while (1)
    {
        printf("HahaOS > ");
        command = readCommand();

        if (strcmp(command, "") == 0)
        {
            //Empty command
            free(command);
            continue;
        }
        else if (strcmp(command, "exit") == 0)
        {
            //Command "exit"
            free(command);
            break;
        }
        else if (strcmp(command, "!!") == 0)
        {
            //History feature
            if (strcmp(history, "") == 0)
            {
                fprintf(stderr, "No history\n");
                free(command);
                continue;
            }
            strncpy(command, history, strlen(history) + 1);
        }

        strncpy(history, command, strlen(command) + 1);
        args = parseCommand(command);
        execCommand(args, args1);
        free(command);
        free(args);
    }
    free(history);
    free(args1);
}

int main(void)
{
    mainLoop();
    return 0;
}