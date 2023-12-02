#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUFFER_SIZE 1024

void client_get(const char *file_path, const char *host_name, int port_number) {
    // Create a socket
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1) {
        fprintf(stderr, "Error creating socket\n");
        exit(1);
    }

    // Prepare server address structure
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port_number);
    if (inet_pton(AF_INET, host_name, &(serverAddr.sin_addr)) <= 0) {
        fprintf(stderr, "Invalid address or address not supported\n");
        close(clientSocket);
        exit(1);
    }

    // Connect to the server
    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        fprintf(stderr, "Error connecting to server\n");
        close(clientSocket);
        exit(1);
    }

    // Send GET request to the server
    char request[BUFFER_SIZE];
    snprintf(request, sizeof(request), "GET %s HTTP/1.1\r\nHost: %s\r\n\r\n", file_path, host_name);
    send(clientSocket, request, strlen(request), 0);

    // Receive and display data from the server
    char buffer[BUFFER_SIZE];
    ssize_t bytesRead;
    while ((bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0)) > 0) {
        fwrite(buffer, 1, bytesRead, stdout);
    }

    // Close the connection
    close(clientSocket);
}

void client_post(const char *file_path, const char *host_name, int port_number) {
    // Create a socket
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1) {
        fprintf(stderr, "Error creating socket\n");
        exit(1);
    }

    // Prepare server address structure
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port_number);
    if (inet_pton(AF_INET, host_name, &(serverAddr.sin_addr)) <= 0) {
        fprintf(stderr, "Invalid address or address not supported\n");
        close(clientSocket);
        exit(1);
    }

    // Connect to the server
    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        fprintf(stderr, "Error connecting to server\n");
        close(clientSocket);
        exit(1);
    }

    // Read the content of the local file
    FILE *localFile = fopen(file_path, "rb");
    if (localFile == NULL) {
        fprintf(stderr, "Error opening local file\n");
        close(clientSocket);
        exit(1);
    }

    // Calculate the content length
    fseek(localFile, 0, SEEK_END);
    size_t contentLength = ftell(localFile);
    fseek(localFile, 0, SEEK_SET);

    // Send POST request to the server with Content-Length header
    char request[BUFFER_SIZE];
    snprintf(request, sizeof(request), "POST %s HTTP/1.1\r\nHost: %s\r\nContent-Length: %zu\r\n\r\n", file_path, host_name, contentLength);
    send(clientSocket, request, strlen(request), 0);

    // Send the content of the local file to the server
    char buffer[BUFFER_SIZE];
    size_t bytesRead;
    while ((bytesRead = fread(buffer, 1, BUFFER_SIZE, localFile)) > 0) {
        send(clientSocket, buffer, bytesRead, 0);
    }

    // Close the local file
    fclose(localFile);

    // Close the connection
    close(clientSocket);
}

int main(int argc, char *argv[]) {
    if (argc != 4 && argc != 5) {
        fprintf(stderr, "Usage: %s file_path host_name [port_number]\n", argv[0]);
        return 1;
    }

    const char *file_path = argv[1];
    const char *host_name = argv[2];
    int port_number = (argc == 5) ? atoi(argv[3]) : 80;

    // Determine the operation (GET or POST) based on the command
    if (strcmp(argv[0], "./my_client_get") == 0) {
        client_get(file_path, host_name, port_number);
    } else if (strcmp(argv[0], "./my_client_post") == 0) {
        client_post(file_path, host_name, port_number);
    } else {
        fprintf(stderr, "Invalid client program\n");
        return 1;
    }

    return 0;
}
