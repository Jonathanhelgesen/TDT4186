#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

void parseArgs(char input[], char *args[])
{
    char *token;
    token = strtok(input, " ");

    int i = 0;
    while (token != NULL)
    {
        args[i] = token;
        token = strtok(NULL, " ");
        i++;
    }
}

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

int main()
{
    // get current working directory
    char cwd[100];
    getcwd(cwd, sizeof(cwd));

    // print to terminal
    printf("%s : ", cwd);

    // take input
    char input[100];
    bzero(input, sizeof(input));
    scanf("%[^\n]", input);

    // read path and args
    char *args[10];
    bzero(args, sizeof(args));
    parseArgs(input, args);

    char *cmd = args[0];

    // fork process
    pid_t cpid;
    if ((cpid = fork()) == 0)
    {
        if (strcmp("cd", cmd) == 0) {
            chdir(args[1]);

            // print to check working
            // char s[100];
            // printf("%s\n", getcwd(s, 100));

        } else {
            execv(cmd, args);
        }

        exit(0);
    }

    int status;
    waitpid(cpid, &status, 0);

    printStatus(status, args);
}