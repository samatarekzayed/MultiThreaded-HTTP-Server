#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdbool.h>
#include <fcntl.h>


#define BUFFER_SIZE 1024 * 300 * 8
const char *getFileName(const char *path);


void DieWithSystemMessage(const char *msg) {
    perror(msg);
    exit(1);
}

long getFileSize(FILE *file) {
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);
    return size;
}

int parseCommand(const char *line, char *method, char *path, char *host, int *port_number) {
    // Assuming the format in input.txt is "method path host"
    return sscanf(line, "%19s %255s %255s %d", method, path, host, port_number);
}

int connectToServer(const char *host, int port_number) {
    // Create socket using socket()
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        DieWithSystemMessage("ERROR opening socket");
    }

    // Define a sockaddr_in structure for the server address
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));  // Clear structure
    serv_addr.sin_family = AF_INET;            // Internet address family
    serv_addr.sin_port = htons(port_number);   // Server port

    // If connecting locally, use the loopback IP address
    if (inet_pton(AF_INET, host, &serv_addr.sin_addr) <= 0) {
        close(sockfd);
        DieWithSystemMessage("Invalid address/ Address not supported");
    }

    // Connect to the server
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        close(sockfd);
        DieWithSystemMessage("ERROR connecting");
    }

    // Set the socket to non-blocking mode
    if (fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFL) | O_NONBLOCK) < 0) {
        // Handle error in setting socket to non-blocking mode
        close(sockfd);
        DieWithSystemMessage("ERROR setting socket to non-blocking");
    }

    return sockfd;
}

void validateArguments(int argc) {
    if (argc != 3) {
        fprintf(stderr, "Usage: <server_ip> <port_number>\n");
        exit(1);
    }
}

FILE *open_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        DieWithSystemMessage("Error opening the file");
    }
    return file;
}

const char *getFileExtention(const char *str) {
    if(str == NULL)
        return NULL;
    
    char*content_type = strstr(str, ".");

    if(content_type) {
        if(strcmp(content_type, ".txt") == 0 || strcmp(content_type, ".html") == 0 || strcmp(content_type, ".jpeg") == 0 || strcmp(content_type, ".jpg") == 0) {
            return content_type;
        } else {
            DieWithSystemMessage("Content type not supported");
        }
    }
    return NULL;
}

const char *getFileName(const char *path) {
    const char *fileName = strrchr(path, '/'); // Find the last occurrence of '/'
    if (fileName != NULL) {
        // Increment to get the filename without the '/'
        return fileName + 1;
    }
    return path; // Return the original path if no '/'
}


void handleGET(int sockfd, const char *path, const char *host) {
    char request[BUFFER_SIZE];
    char response[BUFFER_SIZE];

    // Create HTTP GET request
    snprintf(request, BUFFER_SIZE, "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: Keep-Alive\r\n\r\n", path, host);
    printf("________________________\n%s\n", request);
    
    if (send(sockfd, request, strlen(request), 0) < 0) {
        DieWithSystemMessage("Send error");
    }

    printf("Request Sent\n");

    // Receive response in a loop to handle large responses
    memset(response, 0, BUFFER_SIZE);
    ssize_t n;
    int total_received = 0;

    const char *filename = getFileName(path);
    if (filename == NULL) {
        fprintf(stderr, "Error retrieving file name from path.\n");
        return;
    }

    ssize_t bytesRead = recv(sockfd, response, BUFFER_SIZE - 1, 0);
    printf("request: %s\n", response);

    char* requestBody = strstr(response, "\r\n\r\n");
    if (requestBody != NULL) {
        requestBody += 4;
        // Save requestBody to a file
        const char *filename = getFileName(path); // Make sure this function returns a valid path
        FILE *file = fopen(filename, "wb"); // Open the file in binary write mode to preserve binary data if present
        if (file != NULL) {
            fwrite(requestBody, 1, strlen(requestBody), file);
            fclose(file); // Close the file after writing
            printf("Request body saved to %s\n", filename);
        } else {
            perror("Could not open file for writing");
            // Handle the error, such as by returning or exiting.
        }
    }

    printf("Request Body: %s\n", requestBody);

    // FILE *image = fopen(filename, "wb");
    // if (image == NULL) {
    //     perror("Error in creating file");
    //     return;
    // }

    // total_received += bytesRead;
    // // saveResponseToFile(response, total_received, path);
    // fwrite(response, 1, bytesRead, image);

    // fclose(image);

    // FILE *image = fopen(filename, "wb");
    // if (image == NULL) {
    //     perror("Error in creating file");
    //     return;
    // }

   
    // n = recv(sockfd, response, BUFFER_SIZE, 0);
    // if (n > 0) {
    //     // fwrite(response, 1, n, image);
    //     total_received += n;
    //     saveResponseToFile(response, total_received, path);
    // } else if (n == 0) {
    //     // The other side has closed the connection
    //     printf("Connection closed by peer.\n");
    // } else {
    //     // An error has occurred
    //     printf("Receive failed");
    //     fclose(image);
    //     return;
    // }
    // fclose(image);

    printf("Total received: %d bytes\n", total_received);

    response[total_received] = '\0';  // Null-terminate the response
}

void handlePOST(int sockfd, const char *path, const char *host) {
    printf("Handling POST request\n");

    // Open the text file in read mode
    FILE *file = fopen("index.html", "rb");
    if (file == NULL) {
        perror("Error opening input text file");
        return;
    }

    // Get the size of the text file
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Allocate memory to store file contents
    char *fileData = (char *)malloc(fileSize);
    if (fileData == NULL) {
        fclose(file);
        perror("Memory allocation failed");
        return;
    }

    // Read the file contents into fileData
    size_t bytesRead = fread(fileData, 1, BUFFER_SIZE, file);
    if (bytesRead != fileSize) {
        fclose(file);
        free(fileData);
        perror("Error reading text file");
        return;
    }

    fclose(file); // Close the text file after reading

    char request[BUFFER_SIZE];
    char response[BUFFER_SIZE];

    // Content-Type handling based on file extension or specific conditions
    const char *contentType = "application/octet-stream"; // Default content type (binary/octet-stream)

    const char *extension = strrchr(path, '.'); // Get file extension
    if (extension != NULL) {
        if (strcmp(extension, ".txt") == 0) {
            contentType = "text/plain";
        } else if (strcmp(extension, ".html") == 0) {
            contentType = "text/html";
        } else if (strcmp(extension, ".jpg") == 0) {
            contentType = "image/jpeg";
        } else if(strcmp(extension, ".jpg") == 0) {
            contentType = "image/jpg";
        }
    }

    // Construct the POST request
    sprintf(request, "POST %s HTTP/1.1\r\nHost: %s\r\nContent-Length: %ld\r\nContent-Type: %s\r\n\r\n%s",
            path, host, fileSize, contentType, fileData);


    send(sockfd, request, strlen(request), 0);

    memset(response, 0, BUFFER_SIZE);
    ssize_t n;
    
    while ((n = recv(sockfd, response, BUFFER_SIZE - 1, 0)) > 0) {
        response[n] = '\0'; // Null-terminate the response
    }

    printf("Response:\n%s\n", response);

    free(fileData); // Free dynamically allocated memory
}

int main(int argc, char *argv[]) {
    validateArguments(argc);

    FILE *file = open_file("input.txt");

    char line[1024];
    char method[20], path[256], host[256];
    int port_number = atoi(argv[2]); // Use the port number from the command line arguments
    char ip_address[256];
    strcpy(ip_address, argv[1]);
    int server_port =atoi(argv[2]);
    int sockfd = connectToServer(ip_address, server_port);

    while (fgets(line, sizeof(line), file)) {
        if (parseCommand(line, method, path, host, &port_number) < 3) { // Expecting 3 items from input
            fprintf(stderr, "Invalid Command Format: %s\n", line);
            continue;
        }
    
        printf("Connected to Socket Number: %d\n", sockfd);

        if (strcmp(method, "client_get") == 0) { // Changed to standard HTTP method
            handleGET(sockfd, path, host);      
        } else if (strcmp(method, "client_post") == 0) { // Changed to standard HTTP method
            handlePOST(sockfd, path, host);
        }

        sleep(2);  
    }
    close(sockfd);

    fclose(file);
    return 0;
}