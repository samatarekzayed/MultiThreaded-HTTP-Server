#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <fcntl.h>
#include <sys/stat.h>

// Function prototype
void saveDataToFile(int clientSocket, char* filename);

#define BUFFER_SIZE 1024

// Define a global variable to track the number of active connections
int activeConnections = 0;

void handleClient(int clientSocket) {
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);
   printf("Received HTTP request:\n%s\n", buffer);
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
            char response[] = "HTTP/1.1 200 OK\r\nConnection: keep-alive\r\n\r\n";
            send(clientSocket, response, sizeof(response) - 1, 0);

            size_t bytesRead;
            while ((bytesRead = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
                send(clientSocket, buffer, bytesRead, 0);
            }

            fclose(file);
        } else {
            // File not found, send HTTP 404 Not Found
            printf("Requested File: %s\n", fullFilePath);
            char response[] = "HTTP/1.1 404 Not Found\r\nConnection: keep-alive\r\n\r\n";
            send(clientSocket, response, sizeof(response) - 1, 0);
        }
     }
    
           
else if (strcmp(method, "POST") == 0) {
    // Handle POST request
    // Find the Content-Length header to determine the size of the POST data
    char* contentLengthHeader = strstr(request, "Content-Length:");
    if (contentLengthHeader != NULL) {
        int contentLength;
        sscanf(contentLengthHeader, "Content-Length: %d", &contentLength);

        // Skip the HTTP headers to read the POST data
        char* postDataStart = strstr(request, "\r\n\r\n") + 4;
        size_t postDataSize = strlen(postDataStart);

        // Check if all data has been received
        while (postDataSize < contentLength) {
            ssize_t bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            if (bytesRead <= 0) {
                fprintf(stderr, "Error receiving POST data\n");
                return;
            }
            postDataSize += bytesRead;
            strcat(postDataStart, buffer);
        }

        // Save the POST data to a file
        saveDataToFile(clientSocket, "post_data.txt");

        // Send a response back to the client
        char response[] = "HTTP/1.1 200 OK\r\nConnection: keep-alive\r\n\r\n";
        send(clientSocket, response, sizeof(response) - 1, 0);
    } else {
        // Content-Length header not found, handle error
        fprintf(stderr, "Error: Content-Length header not found in POST request\n");
        char response[] = "HTTP/1.1 400 Bad Request\r\nConnection: keep-alive\r\n\r\n";
        send(clientSocket, response, sizeof(response) - 1, 0);
    }
}

        
 

    // Close the client socket
    close(clientSocket);
}
/////////////////////////////////////////////////////////////////






/////////////////////////////////////////////////////////////////////////



int main(int argc, char *argv[]) {
//    argc It is an integer representing the number of command-line arguments passed to the program.
// argv[0] is the name of the program, and argv[1] through argv[argc-1] are the actual command-line arguments.

    if (argc != 2) {
        //error message
        fprintf(stderr, "Usage: %s port_number\n", argv[0]);
        return 1;
    }
// The atoi() function in C is used to convert a string (representing an integer) to an integer. 
    int port = atoi(argv[1]);

    // Create a socket
    // AF_INET: Specifies that the socket will use the IPv4 address family.
    // SOCK_STREAM: Specifies that the socket will be of type SOCK_STREAM, indicating a reliable, 
    // connection-oriented stream. This is commonly used for TCP communication.
    //0: The protocol parameter is set to 0, indicating that the system should choose the appropriate
    //  protocol based on the specified domain (AF_INET) and type (SOCK_STREAM)
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        fprintf(stderr, "Error creating socket\n");
        return 1;
    }

    // Bind the socket to an address
    // struct sockaddr_in structure, which is commonly used for representing an Internet 
    // socket address in the IPv4 address family. 
    struct sockaddr_in serverAddr;
    // it will be an IPv4 socket.
    serverAddr.sin_family = AF_INET;
    // IP address to bind the socket to. In this case, INADDR_ANY is used, 
    // which means the socket will accept connections on any of the host's IP addresses.
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    // convert the port number from host byte order to network byte order. This conversion is necessary 
    // because different architectures may use different byte orders, and network protocols typically use a 
    // specific byte order (big-endian).
    serverAddr.sin_port = htons(port);

    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        fprintf(stderr, "Error binding socket\n");
        return 1;
    }

    // Listen for incoming connections
    // 10: The second parameter is the maximum number of pending connections that can be queued up.
    if (listen(serverSocket, 10) == -1) {
        fprintf(stderr, "Error listening on socket\n");
        return 1;
    }

    printf("Server listening on port %d...\n", port);

    while (1) {
        // Accept a new connection
        struct sockaddr_in clientAddr;
        socklen_t clientAddrLen = sizeof(clientAddr);
        // accept() function in C is used to accept an incoming connection on a listening socket.
        int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);
        if (clientSocket == -1) {
            fprintf(stderr, "Error accepting connection\n");
            continue;
        }

        // Increment the active connections count
        activeConnections++;
        
        // Set a dynamic timeout based on the number of active connections
        int timeoutSeconds = 1 - activeConnections; // Adjust this formula as needed
        struct timeval timeout;
        timeout.tv_sec = (timeoutSeconds < 1) ? 1 : timeoutSeconds;
        setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));

        // Create a new thread to handle the client
        // Note: In C, you may need a threading library like pthread to create threads.
        // Implementing threads in C can be more complex compared to C++, which has native support.
        handleClient(clientSocket);

        // Decrement the active connections count
        activeConnections--;
    }

    // Close the server socket (Note: This part may not be reached in this simple example)
    close(serverSocket);

    return 0;
}
