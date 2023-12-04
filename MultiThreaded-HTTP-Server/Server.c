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
#include <signal.h>
#include <sys/wait.h> 
#include <sys/mman.h>
#include <time.h>

#define TARGET_LOAD_THRESHOLD 1.0
#define MAX_TIMEOUT_SECONDS 20
#define MIN_TIMEOUT_SECONDS 5
#define BUFFER_SIZE 1024 * 500 * 8

// Function prototypes
volatile int activeConnections = 0;

void cleanup();
void handleClient(int clientSocket);
void saveDataToFile(int clientSocket, const char* filename, size_t contentLength, const char* requestBody, const char* contentType);
const char* getContentType(const char* filename);



struct SharedData {
    int activeConnections;
};

// Pointer to shared data
struct SharedData* sharedData;

void decrementActiveConnections() {
     __sync_fetch_and_sub(&sharedData->activeConnections, 1);
}

void handleClient(int clientSocket) {
    int timeoutSeconds;
    time_t startTime, currentTime;

    startTime = time(NULL);

    // Check the number of active connections and set timeout accordingly
    if (sharedData->activeConnections > 5) {
        timeoutSeconds = MIN_TIMEOUT_SECONDS ;
    } else {
        timeoutSeconds = MAX_TIMEOUT_SECONDS ;
    }


    while(1) {
        currentTime = time(NULL);    

        // Check for timeout before attempting to receive data
        if (difftime(currentTime, startTime) > timeoutSeconds) {
            printf("Timeout exceeded\n");
            break;
        }

        fd_set readfds;
        struct timeval timeout;
        timeout.tv_sec = timeoutSeconds; // Set the timeout duration
        timeout.tv_usec = 0;

        // Initialize the file descriptor set
        FD_ZERO(&readfds);
        FD_SET(clientSocket, &readfds);
        
        int activity = select(clientSocket + 1, &readfds, NULL, NULL, &timeout);
        
        if (activity == -1) {
            perror("Select error");
            break;
        } else if (activity == 0) {
            // No activity within the specified timeout
            printf("No request received within the timeout\n");
            continue; // Skip recv and continue waiting
        }

        char buffer[BUFFER_SIZE];
        memset(buffer, 0, BUFFER_SIZE);

        ssize_t bytesRead = recv(clientSocket, buffer, BUFFER_SIZE - 1, 0);

        // Check if the client closed the connection or an error occurred
        if (bytesRead <= 0) {
            if (bytesRead == 0) {
               continue;
            } else {
                perror("Error receiving HTTP request");
            }
            break;
        }

        buffer[bytesRead] = '\0';  // Null-terminate the received data
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

   // Create a shared memory object
    int shm_fd = shm_open("/shared_data", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    if (shm_fd == -1) {
        perror("Error creating shared memory");
        return 1;
    }

    // Set the size of the shared memory object
    if (ftruncate(shm_fd, sizeof(struct SharedData)) == -1) {
        perror("Error setting the size of shared memory");
        return 1;
    }

    // Map the shared memory object
    sharedData = (struct SharedData*)mmap(NULL, sizeof(struct SharedData), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (sharedData == MAP_FAILED) {
        perror("Error mapping shared memory");
        return 1;
    }
    atexit(cleanup);
    
    while (1) {
        struct sockaddr_in clientAddr;
        socklen_t clientAddrLen = sizeof(clientAddr);
        int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);
        if (clientSocket == -1) {
            fprintf(stderr, "Error accepting connection\n");
            continue;
        }

        // Create a new process for each connection
        pid_t pid = fork();
        if (pid == -1) {
            fprintf(stderr, "Error creating child process\n");
            close(clientSocket);
            continue;
        } else if (pid == 0) {
            // This is the child process
            // close(serverSocket);  // Close the server socket in the child process
                // Increment the activeConnections counter
            __sync_fetch_and_add(&sharedData->activeConnections, 1);

            printf("___________CLIENT HERE\n");

            // Handle the client in the child process
            handleClient(clientSocket);

          // Decrement the activeConnections counter in shared memory
            decrementActiveConnections();
          

            // Exit the child process
            exit(0);
        } else {
            // This is the parent process
            close(clientSocket);  // Close the client socket in the parent process
        }
          while (waitpid(-1, NULL, WNOHANG) > 0) {
            // Do nothing
        }
    }
   // Unmap shared memory
    cleanup();
    close(serverSocket);

    return 0;
}
void cleanup() {
    // Unmap shared memory
    if (sharedData != NULL && sharedData != MAP_FAILED) {
        munmap(sharedData, sizeof(struct SharedData));
    }
    // Close shared memory object
    shm_unlink("/shared_data");
}