#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>

#define MAXSIZE (8092 * 1024)
#define PORT 8000

char body[MAXSIZE], message[MAXSIZE], buffer[MAXSIZE];

void error(const char *message)
{
    perror(message);
    exit(1);
}

// Find path of requested file
const char *parseRequest(char *request)
{
    char *token;
    token = strtok(request, " ");
    bzero(&request, sizeof(request));
    token = strtok(NULL, " ");
    if (token[0] == '/')
    {
        memmove(token, token + 1, strlen(token));
    }
    return token;
}

void getHtml(char *path, char text[])
{
    FILE *htmlData = fopen(path, "r");
    char line[100];
    char responseData[8000];
    bzero(responseData, sizeof(responseData));
    bzero(line, sizeof(line));
    while (fgets(line, 100, htmlData) != 0)
    {
        strcat(responseData, line);
    }
    strcat(text, responseData);
}

int main()
{

    // File descriptors
    int socket_fd, new_socket_fd;

    socklen_t client_len;
    struct sockaddr_in server_address, client_address;
    int n;
    socket_fd = socket(PF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0)
    {
        error("ERROR opening socket");
    }

    // Define the address
    bzero((char *)&server_address, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(PORT);

    if (bind(socket_fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        error("ERROR on binding");
    }

    // Listen for incoming requests
    if ((listen(socket_fd, 10)) < 0)
    {
        error("ERROR on listen");
    }

    char htmlCode[300];
    char path[50];

    // Start main loop
    while (1)
    {

        client_len = sizeof(client_address);

        // Accept new request
        new_socket_fd = accept(socket_fd, (struct sockaddr *)&client_address, &client_len);
        if (new_socket_fd < 0)
            error("ERROR on accept");

        bzero(body, sizeof(body));
        bzero(buffer, sizeof(buffer));
        bzero(message, sizeof(message));
        bzero(htmlCode, sizeof(htmlCode));
        bzero(path, sizeof(path));

        // Read the HTTP request
        n = read(new_socket_fd, buffer, sizeof(buffer) - 1);
        if (n < 0)
            error("ERROR reading from socket");

        // Find the requested html code
        printf("Request from client:\n%s\n", buffer);

        strcpy(path, parseRequest(buffer));

        if (strstr(path, "favicon") != NULL)
        {
            continue;
        }

        getHtml(path, htmlCode);


        // Generate response
        snprintf(body, sizeof(body), "%s", htmlCode);
        snprintf(message, sizeof(message),
                 "HTTP/0.9 200 OK\n"
                 "Content-Type: text/html\n"
                 "Content-Length: %lu\n\n%s",
                 strlen(body), body);

        // Send response
        n = write(new_socket_fd, message, strlen(message));
        if (n < 0)
            error("ERROR writing to socket");

        // Close the connection to client
        close(new_socket_fd);
    }
}