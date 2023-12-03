#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdbool.h> 

#define BUFFER_SIZE 1024

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

FILE *openInputFile(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        DieWithSystemMessage("Error opening the file");
    }
    return file;
}

void saveResponseToFile(const char *response, int total_received) {
    // Check if the response body is present
    char *bodyStart = strstr(response, "\r\n\r\n");
    if (bodyStart) {
        bodyStart += 4; // Move to start of body

        // Check the Content-Type header to determine the type of content
        char *contentType = strstr(response, "Content-Type: ");
        bool saveToFile = false;
        const char *fileExtension = "";

        if (contentType) {
            contentType += strlen("Content-Type: ");
            if (strstr(contentType, "text/plain") != NULL) {
                printf("TXT\n");
                fileExtension = ".txt";
                saveToFile = true;
            } else if (strstr(contentType, "image/jpeg") != NULL) {
                printf("JPEG\n");
                fileExtension = ".jpeg";
                saveToFile = true;
            } else if (strstr(contentType, "text/html") != NULL) {
                printf("HTML\n");
                fileExtension = ".html";
                saveToFile = true;
            }
        }

        if (saveToFile) {
            // Write body to file based on content type
            char filename[100];
            snprintf(filename, sizeof(filename), "received_content%s", fileExtension);
            FILE *fp = fopen(filename, "wb"); // Use "wb" for binary files
            if (fp != NULL) {
                fwrite(bodyStart, sizeof(char), total_received - (bodyStart - response), fp);
                fclose(fp);
            } else {
                // Handle file writing error
                perror("File writing error");
            }
        }
    }
}