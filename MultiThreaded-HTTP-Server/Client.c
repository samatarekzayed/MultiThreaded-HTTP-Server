#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdbool.h> 

#define BUFFER_SIZE 1024 * 500 * 8

char request[BUFFER_SIZE];
char response[BUFFER_SIZE];


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


void writeResponseBodyToTxtFile(const char *response, int total_received) {
    // Check if the response content is present
    if (response != NULL && total_received > 0) {

        // Write content to file
        FILE *fp = fopen("response_body.txt", "w"); // Open file in write mode
        if (fp != NULL) {
            fwrite(response, sizeof(char), total_received, fp);
            fclose(fp);
            printf("Response body written to 'response_body.txt'\n");
        } else {
            perror("File writing error");
        }
    } else {
        DieWithSystemMessage("Invalid or empty response content.\n");
    }
}

// Function to write the response body to a file
void writeResponseBodyToHTMLFile(const char *response, int total_received) {
     // Check if the response content is present
    if (response != NULL && total_received > 0) {

        // Write content to file
        FILE *fp = fopen("response_body.html", "w"); // Open file in write mode
        if (fp != NULL) {
            fwrite(response, sizeof(char), total_received, fp);
            fclose(fp);
            printf("Response body written to 'response_body.html'\n");
        } else {
            perror("File writing error");
        }
    } else {
        DieWithSystemMessage("Invalid or empty response content.\n");
    }
}

void saveImageToFile(const char *response, int total_received) {
    // Check if the response body is present
    char *bodyStart = strstr(response, "\r\n\r\n");
    if (bodyStart) {
        bodyStart += 4; // Move to start of body

        // Write body to file as an image
        FILE *fp = fopen("image.jpeg", "wb"); // Open file in binary write mode (assuming it's a JPEG image)
        if (fp != NULL) {
            fwrite(bodyStart, sizeof(char), total_received - (bodyStart - response), fp);
            fclose(fp);
            printf("Image data written to 'image.jpeg'\n");
        } else {
            perror("File writing error");
        }
    }
}

const char *getFileExtention(const char *str) {
    if(str == NULL)
        return NULL;
    
    char*content_type = strstr(str, ".");

    if(content_type) {
        if(strcmp(content_type, ".txt") == 0 || strcmp(content_type, ".html") == 0 || strcmp(content_type, ".jpeg") == 0) {
            return content_type;
        } else {
            DieWithSystemMessage("Content type not supported");
        }
    }
    return NULL;
}

void saveResponseToFile(const char *response, int total_received, const char *fileExtension) {
    if (response == NULL || fileExtension == NULL) {
        DieWithSystemMessage("Invalid arguments");
    }

    // Check if the response body is present
    char *bodyStart = strstr(response, "\r\n\r\n");
    if (bodyStart) {
        bodyStart += 4; // Move to start of body
        int body_length = total_received - (bodyStart - response); // Calculate the body length
        
        if (strcmp(".txt", fileExtension) == 0) {
            writeResponseBodyToTxtFile(bodyStart, body_length);
        } else if (strcmp(".html", fileExtension) == 0) {
            writeResponseBodyToHTMLFile(bodyStart, body_length);
        } else if (strcmp(".jpeg", fileExtension) == 0) {
            saveImageToFile(bodyStart, body_length);
        } else {
            DieWithSystemMessage("File extension not supported");
        }
    } else {
        DieWithSystemMessage("Response body not found");
    }
}


void handleGET(int sockfd, const char *path, const char *host) {
    char request[BUFFER_SIZE];
    char response[BUFFER_SIZE];

    // Get the file extension using the function
    const char *fileExtension = getFileExtention(path);

    // Create HTTP GET request
    snprintf(request, BUFFER_SIZE, "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: Keep-Alive\r\n\r\n", path, host);
    
    if (send(sockfd, request, strlen(request), 0) < 0) {
        DieWithSystemMessage("Send error");
    }

    // Receive response in a loop to handle large responses
    memset(response, 0, BUFFER_SIZE);
    int total_received = 0;
    ssize_t n;
    while ((n = recv(sockfd, response + total_received, BUFFER_SIZE - total_received - 1, 0)) > 0) {
        total_received += n;
    }

    if (n < 0) {
        DieWithSystemMessage("Receive error");
    }

    response[total_received] = '\0';  // Null-terminate the response

    printf("Response:\n%s\n", response);

    // Save the response body based on content type
    saveResponseToFile(response, total_received, fileExtension);
}

void handlePOST(int sockfd, const char *path, const char *host) {
    printf("Handling POST request\n");

    // Open the text file in read mode
    FILE *file = fopen("draft.txt", "r");
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
    size_t bytesRead = fread(fileData, 1, fileSize, file);
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

    // Check file extension or any other conditions to determine the content type
    // Example: Check if the file path ends with specific extensions (txt, html, jpg, etc.)
    const char *extension = strrchr(path, '.'); // Get file extension
    if (extension != NULL) {
        if (strcmp(extension, ".txt") == 0) {
            contentType = "text/plain";
        } else if (strcmp(extension, ".html") == 0) {
            contentType = "text/html";
        } else if (strcmp(extension, ".jpg") == 0 || strcmp(extension, ".jpeg") == 0) {
            contentType = "image/jpeg";
        }
        // Add more conditions for other file types as needed
    }

    // Construct the POST request
    sprintf(request, "POST %s HTTP/1.1\r\nHost: %s\r\nContent-Length: %ld\r\nContent-Type: %s\r\n\r\n%s",
            path, host, fileSize, contentType, fileData);


    send(sockfd, request, strlen(request), 0);

    memset(response, 0, BUFFER_SIZE);
    ssize_t n;
    while ((n = recv(sockfd, response, BUFFER_SIZE - 1, 0)) > 0) {
        response[n] = '\0'; // Null-terminate the response
        printf("Response:\n%s\n", response);
    }

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
        
        printf("Method: %s\nPath: %s\nHost: %s\nPort: %d\n", method, path, host, port_number);


        printf("Connected to Socket Number: %d\n", sockfd);

        if (strcmp(method, "client_get") == 0) { // Changed to standard HTTP method
            handleGET(sockfd, path, host);      
        } else if (strcmp(method, "client_post") == 0) { // Changed to standard HTTP method
            handlePOST(sockfd, path, host);
        }
        
    }
    close(sockfd);

    fclose(file);
    return 0;
}