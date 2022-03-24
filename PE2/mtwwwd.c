#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include "sem.h"
#include "bbuffer.h"
#define MAXSIZE (4096*1024)
#define PORT 8000

char body[MAXSIZE], message[MAXSIZE], buffer[MAXSIZE];

void error(const char *message) {
    perror(message); exit(1);
    }

// Find path of requested file
const char* getPath(char *request){
    char *token;
    token = strtok(request, " ");
    bzero(&request, sizeof(&request));
    token = strtok(NULL, " ");
    if (token[0] == '/') {
        memmove(token, token + 1, strlen(token));
    }
    return token;
}

// Return HTML code from given path
void getHtml(char *path, char text[])
{
    FILE *htmlData = fopen(path, "r");
    char line[100];
    char responseData[8000];
    bzero(responseData, sizeof(responseData)); // Mulig disse kan fjernes ved concurrency
    bzero(line, sizeof(line)); 
    while (fgets(line, 100, htmlData) != 0) {
        strcat(responseData, line);
    }
    strcat(text, responseData);
    fclose(htmlData);
}


void* handle_connection(void* client_socket_ptr){
    int client_socket = *((int*) client_socket_ptr);
    free(client_socket_ptr);
    int n;
    char htmlCode[300];
    char path[30];

    bzero(body, sizeof(body));
    bzero(buffer, sizeof(buffer));
    bzero(message, sizeof(message));
    bzero(htmlCode, sizeof(htmlCode));

    // Read the HTTP request
    n = read(client_socket, buffer, sizeof(buffer) - 1);
    if (n < 0) error("ERROR reading from socket");

    // Ignore pre-flight requests
    if (buffer[0] == '\0') {
        printf("[Pre-flight request ignored]\n\n");
        close(client_socket);
        return NULL;
    }

    // Find the requested html code
    printf("Request from client:\n%s\n", buffer);

    strcpy(path, getPath(buffer));

    if (strstr(path, "favicon") != NULL) {
        printf("[favicon-request ignored]\n\n");
        close(client_socket);
        return NULL;
    }

    getHtml(path, htmlCode);

    // Generate response
    snprintf(body, sizeof(body) + 86, "%s", htmlCode);
    snprintf(message, sizeof(message) + 86,
        "HTTP/0.9 200 OK\nContent-Type: text/html\nContent-Length: %ld\n\n%s", 
        strlen(body), body);
        
    // Send response
    n = write(client_socket, message, strlen(message));
        if (n < 0) error("ERROR writing to socket");
        
        // Close the connection to client
        close(client_socket);
        return NULL;
    }


int main(){

    // File descriptors
    int server_socket_fd, client_socket_fd;

    socklen_t client_len;
    struct sockaddr_in server_address, client_address;
    int n;

    // Create socket, using TCP
    server_socket_fd = socket(PF_INET, SOCK_STREAM, 0);
    if (server_socket_fd < 0){
        error("ERROR opening socket");
    }
    
    // Define the address
    bzero((char *) &server_address, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(PORT);

    // Bind socket to server address
    if (bind(server_socket_fd, (struct sockaddr *) &server_address, sizeof(server_address)) < 0) {
        error("ERROR on binding");
    }

    // Listen for incoming requests
    if((listen(server_socket_fd, 10)) < 0){
        error("ERROR on listen");
    }


    // Start main loop
    while (1) {

        client_len = sizeof(client_address);

        // Accept new request
        client_socket_fd = accept(server_socket_fd, (struct sockaddr *) &client_address, &client_len);
        if (client_socket_fd < 0) error("ERROR on accept");

        //handle_connection(client_socket_fd);

        pthread_t thread;
        int* client_fd_ptr = malloc(sizeof(int));
        *client_fd_ptr = client_socket_fd;
        pthread_create(&thread, NULL, &handle_connection, client_fd_ptr);

    }

}