#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

// Handle user input
int parseArgs(char input[], char *args[])
{
    char *token;
    // Split input into arguments based on spaces
    token = strtok(input, " ");

    int i = 0;
    while (token != NULL)
    {
        args[i] = token;
        token = strtok(NULL, " ");
        i++;
    }
    return i;
}

// Prints status of the shell
void printStatus(int status, char *args[])
{
    printf("Exit status [ ");
    int i = 0;
    while (args[i] != NULL)
    {
        printf("%s ", args[i]);
        i++;
    }
    printf("] = %d\n", status);
}

// Handles signals
void signalHandler(int sig)
{
    printf("Exiting shell\n");
    exit(0);
}

// Checks if "<" is an argument
int check_input(char **arguments, char **input_file, int length) {
    for(int i = 0; i < length; i++){
        if(arguments[i][0] == '<'){
            *input_file = arguments[i + 1];
            // Reorganize arguments
            for(int ii = i; arguments[ii + 2] != NULL; ii++){
                arguments[ii] = arguments[ii + 2];
            }
            arguments[length - 1] = NULL;
            arguments[length - 2] = NULL;
            //Success
            return 1;
        }
    }
    //Failure
    return 0;
}

//Checks if ">" is an argument
int check_output(char **arguments, char **output_file, int length){
    for(int i = 0; i < length; i++){
        if(arguments[i][0] == '>'){
            *output_file = arguments[i + 1];
            // Reorganize arguments
            for(int ii = i; arguments[ii + 2] != NULL; ii++){
                arguments[ii] = arguments[ii + 2];
            }
            arguments[length - 1] = NULL;
            arguments[length - 2] = NULL;
            //Success
            return 1;
        }
    }
    //Failure
    return 0;
}


int main()
{
    // handles ctrl+c
    signal(SIGINT, signalHandler);

    while (1)
    {
        int redirect_i, redirect_o;
        char *input_file, *output_file, cwd[100];

        // get current working directory

        getcwd(cwd, sizeof(cwd));

        // prints current working directory
        printf("%s : ", cwd);

        // take input
        char input[100];
        bzero(input, sizeof(input));
        int flag;
        flag = scanf("%[^\n]", input);
        getchar(); // stops infinite loop bug with scanf

        // If scanf returns EOF, exit
        if (flag == EOF) {
            printf("Exiting shell\n");
            exit(0);
        }

        // If input is empty, continue
        if (input[0] == '\0') {
            continue;
        }

        // read path and args
        char *args[10];
        bzero(args, sizeof(args));
        int arg_count;
        arg_count = parseArgs(input, args);

        char *cmd = args[0];

        redirect_i = check_input(args, &input_file, arg_count);
        if (redirect_i) {
            arg_count -= 2;
        }

        redirect_o = check_output(args, &output_file, arg_count);
        if (redirect_o) {
            arg_count -= 2;
        }

        if (strcmp("cd", cmd) == 0)
        {
            chdir(args[1]);
        }
        else
        {
            // fork process
            pid_t cpid;

            if ((cpid = fork()) == 0)
            {
                if(redirect_i) {
                    freopen(input_file, "r", stdin);
                }
                if(redirect_o) {
                    freopen(output_file, "w+", stdout);
                }
                execv(cmd, args);
                exit(0);
            }

            int status;
            waitpid(cpid, &status, 0);
            printStatus(status, args);
        }
    }
}