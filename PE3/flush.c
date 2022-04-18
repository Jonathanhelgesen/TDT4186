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
    printf("\nExiting shell\n");
    exit(0);
}

// Returns 1 if redirection symbol is present, 0 if not
int check_redirection(char **arguments, char **input_file, int *length, char symbol) {
    for(int i = 0; i < *length; i++){
        if(arguments[i][0] == symbol){
            *input_file = arguments[i + 1];
            // Reorganize arguments
            for(int ii = i; arguments[ii + 2] != NULL; ii++){
                arguments[ii] = arguments[ii + 2];
            }
            arguments[*length - 1] = NULL;
            arguments[*length - 2] = NULL;
            *length -= 2;
            //Success
            return 1;
        }
    }
    //Failure
    return 0;
}

// Checks "&" is one of the arguments
int check_ampersand(char **arguments, int length){
    if (arguments[length -1][0] == '&'){
        arguments[length -1] = NULL;
        return 1;
    } else {
        return 0;
    }
}

// Node for linked list data structure
typedef struct Node{
    int pid;
    int status;
    int hasExited;
    char* args;
    struct Node* next;
} Node;

Node* create_node() {
    Node *temp;
    temp = malloc(sizeof(Node));
    temp->next = NULL;
    temp->pid = -1;
    temp->hasExited = 0;
    temp->status = -1;
    return temp;
}

Node* add_node(Node **head, int pid, char* args){
    Node *temp;
    temp = create_node();
    temp->pid = pid;
    temp->args = args;
    if ((*head)->pid == -1) {
        // Linked list is empty
        *head = temp;
    } else {
        Node *p;
        p = *head;
        while(p->next != NULL) {
            //Runs until p is last node
            p = p->next;
        }
        p->next = temp; 
    }
    return *head;
}

// Handles terminated processes
void update_nodes(Node* head){
    if (head->pid != -1) {
        // If linked list is not empty
        Node* ptr = head;
        while(ptr != NULL){
            if(ptr->hasExited == 0){
                int status;
                if (waitpid(ptr->pid, &status, WNOHANG) == ptr->pid){
                    if (WIFEXITED(status)) {
                        int exitStatus = WEXITSTATUS(status);
                        ptr->hasExited = 1;
                        ptr->status = exitStatus;
                    }
                }
            }
            ptr = ptr->next;
        }
    }
}


int main()
{
    // handles ctrl+c
    signal(SIGINT, signalHandler);

    // Head of linked list data structure
    Node* head = create_node();

    while (1)
    {
        int redirect_i, redirect_o, ampersand;
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
            printf("\nExiting shell because of EOF\n");
            exit(0);
        }

        // If input is empty, continue
        if (input[0] == '\0') {
            continue;
        }

        // read path and args
        char *args[30];
        bzero(args, sizeof(args));
        int arg_count;
        arg_count = parseArgs(input, args);

        // Make copy of command line to be stored in linked list
        char commandLine[128];
        bzero(commandLine, sizeof(commandLine));
        for(int i = 0; i < arg_count - 1; i++){
            strcat(commandLine, args[i]);
            strcat(commandLine, " ");
        }

        char *cmd = args[0];

        redirect_i = check_redirection(args, &input_file, &arg_count, '<');
        redirect_o = check_redirection(args, &output_file, &arg_count, '>');
        ampersand = check_ampersand(args, arg_count);

        if (strcmp("cd", cmd) == 0)
        {
            chdir(args[1]);
        } 
        else if (strcmp("jobs", cmd) == 0){
            
            update_nodes(head);

            // Print terminated process with exit status
            if (head->pid != -1){
                printf("\n------------------  Status of running background processes  ------------------\n");
                Node* ptr = head;
                while (ptr != NULL) {
                    if (ptr->hasExited == 0){
                        printf("PID: %d, arguments: [ %s]\n", ptr->pid, ptr->args);
                    }
                    ptr = ptr->next;
                }
                printf("------------------------------------------------------------------------------\n\n");
            }
        }
        else
        {
            // fork process
            pid_t cpid;

            if ((cpid = fork()) == 0)
            {
                FILE *input_stream, *output_stream;
                if(redirect_i) {
                    input_stream = freopen(input_file, "r", stdin);
                }
                if(redirect_o) {
                    output_stream = freopen(output_file, "w", stdout);
                }
                execv(cmd, args);
                if (input_stream != NULL) fclose(input_stream);
                if (output_stream != NULL) fclose(output_stream);
                exit(0);
            }

            if (!ampersand){
                // Wait for child process and print status
                int status;
                waitpid(cpid, &status, 0);
                printStatus(status, args);
            } else {
                // Add node to linked list
                add_node(&head, cpid, strdup(commandLine));
            }

            update_nodes(head);

            // Print terminated process with exit status
            if (head->pid != -1){
                printf("\n------------------  Status of terminated background processes  ------------------\n");
                Node* ptr = head;
                while (ptr != NULL) {
                    if (ptr->hasExited == 1){
                        printf("PID: %d, arguments: [ %s], exit status: %d\n", ptr->pid, ptr->args, ptr->status);
                    }
                    ptr = ptr->next;
                }
                printf("---------------------------------------------------------------------------------\n\n");
            }
        }
    }
}