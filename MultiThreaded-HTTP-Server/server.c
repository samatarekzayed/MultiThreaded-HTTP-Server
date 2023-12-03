// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <unistd.h>
// #include <arpa/inet.h>
// #include <sys/socket.h>
// #include <netinet/in.h>
// #include <sys/time.h>
// #include <fcntl.h>
// #include <sys/stat.h>

// // Function prototype
// void saveDataToFile(int clientSocket, const char* filename, size_t contentLength, const char* requestBody)  ;

// #define BUFFER_SIZE 1024

// // Define a global variable to track the number of active connections
// int activeConnections = 0;

// void handleClient(int clientSocket) {
//    char buffer[BUFFER_SIZE];
//     memset(buffer, 0, BUFFER_SIZE);

//     // Receive the HTTP request from the client
//     ssize_t bytesRead = recv(clientSocket, buffer, BUFFER_SIZE - 1, 0);

//     if (bytesRead <= 0) {
//         fprintf(stderr, "Error receiving HTTP request\n");
//         close(clientSocket);
//         return;
//     }

//     // Print the received HTTP request
//     printf("Received HTTP request:\n%s\n", buffer);

//     char* request = buffer;

//     // Extract the HTTP method (GET or POST) and requested file
//     char method[5], filename[255];
//     sscanf(request, "%4s %254s", method, filename);

//     if (strcmp(method, "GET") == 0) {
//         // Handle GET request as before
//         char fullFilePath[260];
//         snprintf(fullFilePath, sizeof(fullFilePath), "./%s", filename);

//         FILE* file = fopen(fullFilePath, "rb");
//         if (file != NULL) {
//             // File exists, send HTTP 200 OK and the file content
//             char response[] = "HTTP/1.1 200 OK\r\nConnection: keep-alive\r\n\r\n";
//             send(clientSocket, response, sizeof(response) - 1, 0);

//             size_t bytesRead;
//             while ((bytesRead = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
//                 send(clientSocket, buffer, bytesRead, 0);
//             }

//             fclose(file);
//         } else {
//             // File not found, send HTTP 404 Not Found
//             printf("Requested File: %s\n", fullFilePath);
//             char response[] = "HTTP/1.1 404 Not Found\r\nConnection: keep-alive\r\n\r\n";
//             send(clientSocket, response, sizeof(response) - 1, 0);
//         }
//      }
    
           
// else if (strcmp(method, "POST") == 0) {
//     //handel post request
//     // Parse the HTTP headers to find Content-Length
//     size_t contentLength = 0;
//     char* contentLengthHeader = strstr(request, "Content-Length: ");
//     if (contentLengthHeader != NULL) {
//         sscanf(contentLengthHeader, "Content-Length: %zu", &contentLength);
//     }

//     // Find the start of the request body
//     char* requestBody = strstr(request, "\r\n\r\n");
//     if (requestBody != NULL) {
//         // Move to the beginning of the body
//         requestBody += 4;

//         // Save the request body to a file
//         saveDataToFile(clientSocket, filename, contentLength, requestBody);
//     } else {
//         // Invalid POST request, missing request body
//         fprintf(stderr, "Invalid POST request: missing request body\n");
//     }
// }

        
 

//     // Close the client socket
//     close(clientSocket);
// }
// /////////////////////////////////////////////////////////////////
// void saveDataToFile(int clientSocket, const char* filename, size_t contentLength, const char* requestBody) {
//     FILE* file = fopen(filename, "wb"); // Open the file in binary write mode
//     if (file == NULL) {
//         fprintf(stderr, "Error opening file for writing\n");
//         return;
//     }

//     // Write the initial data received in the request body
//     size_t initialBytes = strlen(requestBody);
//     fwrite(requestBody, 1, initialBytes, file);

//     // Continue reading from the client socket and writing to the file until Content-Length is reached
//     size_t totalBytesRead = initialBytes;
//     char buffer[BUFFER_SIZE];
//     ssize_t bytesRead;

//     while (totalBytesRead < contentLength && (bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0)) > 0) {
//         fwrite(buffer, 1, bytesRead, file);
//         totalBytesRead += bytesRead;
//     }

//     fclose(file);
// }




// /////////////////////////////////////////////////////////////////////////



// int main(int argc, char *argv[]) {
// //    argc It is an integer representing the number of command-line arguments passed to the program.
// // argv[0] is the name of the program, and argv[1] through argv[argc-1] are the actual command-line arguments.

//     if (argc != 2) {
//         //error message
//         fprintf(stderr, "Usage: %s port_number\n", argv[0]);
//         return 1;
//     }
// // The atoi() function in C is used to convert a string (representing an integer) to an integer. 
//     int port = atoi(argv[1]);

//     // Create a socket
//     // AF_INET: Specifies that the socket will use the IPv4 address family.
//     // SOCK_STREAM: Specifies that the socket will be of type SOCK_STREAM, indicating a reliable, 
//     // connection-oriented stream. This is commonly used for TCP communication.
//     //0: The protocol parameter is set to 0, indicating that the system should choose the appropriate
//     //  protocol based on the specified domain (AF_INET) and type (SOCK_STREAM)
//     int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
//     if (serverSocket == -1) {
//         fprintf(stderr, "Error creating socket\n");
//         return 1;
//     }

//     // Bind the socket to an address
//     // struct sockaddr_in structure, which is commonly used for representing an Internet 
//     // socket address in the IPv4 address family. 
//     struct sockaddr_in serverAddr;
//     // it will be an IPv4 socket.
//     serverAddr.sin_family = AF_INET;
//     // IP address to bind the socket to. In this case, INADDR_ANY is used, 
//     // which means the socket will accept connections on any of the host's IP addresses.
//     serverAddr.sin_addr.s_addr = INADDR_ANY;
//     // convert the port number from host byte order to network byte order. This conversion is necessary 
//     // because different architectures may use different byte orders, and network protocols typically use a 
//     // specific byte order (big-endian).
//     serverAddr.sin_port = htons(port);

//     if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
//         fprintf(stderr, "Error binding socket\n");
//         return 1;
//     }

//     // Listen for incoming connections
//     // 10: The second parameter is the maximum number of pending connections that can be queued up.
//     if (listen(serverSocket, 10) == -1) {
//         fprintf(stderr, "Error listening on socket\n");
//         return 1;
//     }

//     printf("Server listening on port %d...\n", port);

//     while (1) {
//         // Accept a new connection
//         struct sockaddr_in clientAddr;
//         socklen_t clientAddrLen = sizeof(clientAddr);
//         // accept() function in C is used to accept an incoming connection on a listening socket.
//         int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);
//         if (clientSocket == -1) {
//             fprintf(stderr, "Error accepting connection\n");
//             continue;
//         }

//         // Increment the active connections count
//         activeConnections++;
        
//         // Set a dynamic timeout based on the number of active connections
//         int timeoutSeconds = 1 - activeConnections; // Adjust this formula as needed
//         struct timeval timeout;
//         timeout.tv_sec = (timeoutSeconds < 1) ? 1 : timeoutSeconds;
//         setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));

//         // Create a new thread to handle the client
//         // Note: In C, you may need a threading library like pthread to create threads.
//         // Implementing threads in C can be more complex compared to C++, which has native support.
//         handleClient(clientSocket);

//         // Decrement the active connections count
//         activeConnections--;
//     }

//     // Close the server socket (Note: This part may not be reached in this simple example)
//     close(serverSocket);

//     return 0;
// }















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

// Function prototypes
void saveDataToFile(int clientSocket, const char* filename, size_t contentLength, const char* requestBody, const char* contentType);
const char* getContentType(const char* filename);

#define BUFFER_SIZE 1024

// Define a global variable to track the number of active connections
int activeConnections = 0;

void handleClient(int clientSocket) {
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);

    // Receive the HTTP request from the client
    ssize_t bytesRead = recv(clientSocket, buffer, BUFFER_SIZE - 1, 0);

    if (bytesRead <= 0) {
        fprintf(stderr, "Error receiving HTTP request\n");
        close(clientSocket);
        return;
    }

    // Print the received HTTP request
    printf("Received HTTP request:\n%s\n", buffer);

    char* request = buffer;

    // Extract the HTTP method (GET or POST), requested file, and Content-type
    char method[5], filename[255], contentType[255];
    sscanf(request, "%4s %254s", method, filename);

    // Extract Content-type if present
    char* contentTypeHeader = strstr(request, "Content-Type: ");
    if (contentTypeHeader != NULL) {
        sscanf(contentTypeHeader, "Content-Type: %s", contentType);
    }

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
    } else if (strcmp(method, "POST") == 0) {
        // Handle POST request
        // Parse the HTTP headers to find Content-Length
        size_t contentLength = 0;
        char* contentLengthHeader = strstr(request, "Content-Length: ");
        if (contentLengthHeader != NULL) {
            sscanf(contentLengthHeader, "Content-Length: %zu", &contentLength);
        }

        // Find the start of the request body
        char* requestBody = strstr(request, "\r\n\r\n");
        if (requestBody != NULL) {
            // Move to the beginning of the body
            requestBody += 4;

            // Save the request body to a file
            saveDataToFile(clientSocket, filename, contentLength, requestBody, contentType);
        } else {
            // Invalid POST request, missing request body
            fprintf(stderr, "Invalid POST request: missing request body\n");
        }
    }

    // Close the client socket
    close(clientSocket);
}

void saveDataToFile(int clientSocket, const char* filename, size_t contentLength, const char* requestBody, const char* contentType) {
    FILE* file = fopen(filename, "wb"); // Open the file in binary write mode
    if (file == NULL) {
        fprintf(stderr, "Error opening file for writing\n");
        return;
    }

    // Write the initial data received in the request body
    size_t initialBytes = strlen(requestBody);
    fwrite(requestBody, 1, initialBytes, file);

    // Continue reading from the client socket and writing to the file until Content-Length is reached
    size_t totalBytesRead = initialBytes;
    char buffer[BUFFER_SIZE];
    ssize_t bytesRead;

    while (totalBytesRead < contentLength && (bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0)) > 0) {
        fwrite(buffer, 1, bytesRead, file);
        totalBytesRead += bytesRead;
    }

    fclose(file);
}

const char* getContentType(const char* filename) {
    const char* extension = strrchr(filename, '.');
    if (extension != NULL) {
        return extension + 1; // Skip the dot in the extension
    }

    // Default to plain text if no extension is found
    return "text/plain";
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s port_number\n", argv[0]);
        return 1;
    }

    int port = atoi(argv[1]);

    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        fprintf(stderr, "Error creating socket\n");
        return 1;
    }

    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        fprintf(stderr, "Error binding socket\n");
        return 1;
    }

    if (listen(serverSocket, 10) == -1) {
        fprintf(stderr, "Error listening on socket\n");
        return 1;
    }

    printf("Server listening on port %d...\n", port);

    while (1) {
        struct sockaddr_in clientAddr;
        socklen_t clientAddrLen = sizeof(clientAddr);
        int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);
        if (clientSocket == -1) {
            fprintf(stderr, "Error accepting connection\n");
            continue;
        }

        activeConnections++;

        int timeoutSeconds = 1 - activeConnections;
        struct timeval timeout;
        timeout.tv_sec = (timeoutSeconds < 1) ? 1 : timeoutSeconds;
        setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));

        handleClient(clientSocket);

        activeConnections--;
    }

    close(serverSocket);

    return 0;
}
