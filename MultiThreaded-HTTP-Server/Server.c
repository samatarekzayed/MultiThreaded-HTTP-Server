#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define BUFFER_SIZE 1024

void handleClient(int clientSocket) {
    printf("Entered");
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);

    // Receive the HTTP request from the client
    recv(clientSocket, buffer, BUFFER_SIZE, 0);
    char* request = buffer;

    // Extract the HTTP method (GET or POST) and requested file
    char method[5], filename[255];
    sscanf(request, "%4s %254s", method, filename);

    if (strcmp(method, "GET") == 0) {
        // Handle GET request as before
        char fullFilePath[260];
        snprintf(fullFilePath, sizeof(fullFilePath), "./%s", filename);

        FILE* file = fopen(fullFilePath, "rb");
        if (file != NULL) {
            // File exists, send HTTP 200 OK and the file content
            char response[] = "HTTP/1.1 200 OK\r\n\r\n";
            send(clientSocket, response, sizeof(response) - 1, 0);

            size_t bytesRead;
            while ((bytesRead = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
                send(clientSocket, buffer, bytesRead, 0);
            }

            fclose(file);
        } else {
            // File not found, send HTTP 404 Not Found
            printf("Requested File: %s\n", fullFilePath);
            char response[] = "HTTP/1.1 404 Not Found\r\n\r\n";
            send(clientSocket, response, sizeof(response) - 1, 0);
        }
    } else if (strcmp(method, "POST") == 0) {
        // Handle POST request
        // Extract content length
        char* line = strtok(request, "\r\n");
        size_t contentLength = 0;

        while (line != NULL && line[0] != '\0') {
            if (strncmp(line, "Content-Length:", 15) == 0) {
                contentLength = atoi(line + 16);
            }
            line = strtok(NULL, "\r\n");
        }

        // Read the POST data from the body
        char* postData = malloc(contentLength + 1);
        for (size_t i = 0; i < contentLength; ++i) {
            recv(clientSocket, &postData[i], 1, 0);
        }
        postData[contentLength] = '\0';

        // Process the POST data (you can modify this part as needed)
        printf("Received POST data:\n%s\n", postData);

        // Save the POST data to a file
        FILE* outputFile = fopen("output.txt", "w");
        if (outputFile != NULL) {
            fputs(postData, outputFile);
            fclose(outputFile);
            printf("Saved POST data to output.txt\n");
        } else {
            fprintf(stderr, "Error opening output.txt for writing\n");
        }

        // Send a simple response
        char response[] = "HTTP/1.1 200 OK\r\n\r\n";
        send(clientSocket, response, sizeof(response) - 1, 0);
        send(clientSocket, "POST request received", 22, 0);

        free(postData);
    }

    // Close the client socket
    close(clientSocket);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s port_number\n", argv[0]);
        return 1;
    }

    int port = atoi(argv[1]);

    // Create a socket
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        fprintf(stderr, "Error creating socket\n");
        return 1;
    }

    // Bind the socket to an address
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        fprintf(stderr, "Error binding socket\n");
        return 1;
    }

    // Listen for incoming connections
    if (listen(serverSocket, 10) == -1) {
        fprintf(stderr, "Error listening on socket\n");
        return 1;
    }

    printf("Server listening on port %d...\n", port);

    while (1) {
        // Accept a new connection
        struct sockaddr_in clientAddr;
        socklen_t clientAddrLen = sizeof(clientAddr);
        int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);
        if (clientSocket == -1) {
            fprintf(stderr, "Error accepting connection\n");
            continue;
        }

        // Create a new thread to handle the client
        // Note: In C, you may need a threading library like pthread to create threads.
        // Implementing threads in C can be more complex compared to C++, which has native support.
        handleClient(clientSocket);
    }

    // Close the server socket (Note: This part may not be reached in this simple example)
    close(serverSocket);

    return 0;
}