#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

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

struct redirect {
    char symbol;
    int symidx;
    char *before[10];
    char *after[10];
    char cmd[50];
    char output[50];
};

void join(char str[], char* array[]) {
    strcpy(str, array[0]);

        int i = 1;
        while (array[i] != NULL) {
            strcat(str, " ");
            strcat(str, array[i]);
            i++;
        }
}

struct redirect parseRedirect(char* args[]) {

    struct redirect r;
    r.symbol = '\0';
    bzero(r.before, sizeof(r.before));
    bzero(r.after, sizeof(r.after));

    int i = 0;
    while (args[i] != NULL)
    {
        if (*args[i] == '<')
        {
            r.symbol = '<';
            r.symidx = i;
        }
        else if (*args[i] == '>')
        {
            r.symbol = '>';
            r.symidx = i;
        }
        else if (r.symbol != '\0')
        {
            r.after[i - (r.symidx + 1)] = args[i];
        }
        else
        {
            r.before[i] = args[i];
        }
        i++;
    }

    if (r.symbol == '>') {
        join(r.cmd, r.before);
        join(r.output, r.after);
    } else if (r.symbol == '<') {
        join(r.cmd, r.after);
        join(r.output, r.before);
    }

    return r;
}


int ioRedirect(char *args[])
{
    // iterate over args to see if it contains '<' or '>' ??

    struct redirect redir = parseRedirect(args);

    if (redir.symbol != '\0') {

        FILE *processout;
        FILE *filein;

        if ((processout = popen(redir.cmd, "r")) == NULL) {
            fprintf(stderr,"Error popen with %s\n", redir.cmd);
            exit(1);
        }

        if ((filein = fopen(redir.output, "w+")) == NULL) {
            fprintf(stderr,"Error fopen with %s\n", redir.output);
            pclose(processout);
            exit(1);
        }

        int ch;
        while ((ch = fgetc(processout)) != EOF) {
            fputc(ch, filein);
        }

        pclose(processout);
        fclose(filein);

    }     

    return 0;
}



void signalHandler(int sig)
{
    printf("Exiting shell");
    exit(0);
}

int main()
{
    // handles ctrl+c
    // TODO: Handle ctrl+d (Not signal but sends EOF to stdin)
    signal(SIGINT, signalHandler);

    while (1)
    {
        // get current working directory
        char cwd[100];
        getcwd(cwd, sizeof(cwd));

        printf("%s : ", cwd);

        // take input
        char input[100];
        bzero(input, sizeof(input));
        scanf("%[^\n]", input);
        getchar(); // stops infinite loop bug with scanf

        // read path and args
        char *args[10];
        bzero(args, sizeof(args));
        parseArgs(input, args);

        char *cmd = args[0];

        if (strcmp("cd", cmd) == 0)
        {
            chdir(args[1]);
        }
        else
        {

            ioRedirect(args);

            // // fork process
            // pid_t cpid;

            // if ((cpid = fork()) == 0)
            // {
            //     // filedescriptor of output, 0 -> stdout

            //     if (ioRedirect(args) == 0)
            //     {
            //         printf("Redirect");
            //     }
            //     else
            //     {
            //         execv(cmd, args);
            //     }

            //     exit(0);
            // }

            // int status;
            // waitpid(cpid, &status, 0);
            // printStatus(status, args);
        }
    }
}