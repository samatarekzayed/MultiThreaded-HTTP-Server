#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>

#define BUFFER_SIZE 307200

// Function Declarations
void DieWithSystemMessage(const char *msg);
long getFileSize(FILE *file);
int connectToServer(const char *host, int port);
void handleGET(int sockfd, const char *path, const char *host);
void handlePOST(int sockfd, const char *path, const char *host);
void parseCommand(char *line, char *method, char *path, char *host, int *port);
const char *getFileName(const char *path);
const char *getFileExtension(const char *path);
FILE *openFile(const char *filename);

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <server_ip> <port_number>\n", argv[0]);
        exit(1);
    }

    const char *serverIP = argv[1];
    int serverPort = atoi(argv[2]);
    int sockfd = connectToServer(serverIP, serverPort);

    FILE *file = openFile("input.txt");
    char line[1024], method[20], path[256], host[256];
    int port;

    while (fgets(line, sizeof(line), file)) {
        parseCommand(line, method, path, host, &port);

        if (strcmp(method, "GET") == 0) {
            handleGET(sockfd, path, host);
        } else if (strcmp(method, "POST") == 0) {
            handlePOST(sockfd, path, host);
        }
    }

    close(sockfd);
    fclose(file);
    return 0;
}

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

int connectToServer(const char *host, int port) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) DieWithSystemMessage("ERROR opening socket");

    struct sockaddr_in serv_addr = {0};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, host, &serv_addr.sin_addr) <= 0)
        DieWithSystemMessage("Invalid address/ Address not supported");

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        DieWithSystemMessage("ERROR connecting");

    return sockfd;
}

void handleGET(int sockfd, const char *path, const char *host) {
    char request[BUFFER_SIZE];
    snprintf(request, BUFFER_SIZE, "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", path, host);
    if (send(sockfd, request, strlen(request), 0) < 0) DieWithSystemMessage("Send error");

    char response[BUFFER_SIZE];
    int total_received = 0;
    int bytes_received;
    while ((bytes_received = recv(sockfd, response + total_received, BUFFER_SIZE - total_received - 1, 0)) > 0) {
        total_received += bytes_received;
    }
    if (bytes_received < 0) DieWithSystemMessage("Receive error");

    response[total_received] = '\0';  // Null-terminate the response
    char *bodyStart = strstr(response, "\r\n\r\n") + 4;
    const char *filename = getFileName(path);
    FILE *file = fopen(filename, "wb");
    if (!file) DieWithSystemMessage("File open error");
    fwrite(bodyStart, 1, total_received - (bodyStart - response), file);
    fclose(file);
}

void handlePOST(int sockfd, const char *path, const char *host) {
    FILE *file = fopen(path, "rb");
    if (!file) DieWithSystemMessage("Error opening input file");
    long fileSize = getFileSize(file);
    char *fileData = malloc(fileSize);
    if (!fileData) DieWithSystemMessage("Memory allocation failed");
    fread(fileData, 1, fileSize, file);
    fclose(file);

    const char *contentType = getFileExtension(path);
    if (strcmp(contentType, ".txt") == 0) contentType = "text/plain";
    else if (strcmp(contentType, ".html") == 0) contentType = "text/html";
    else if (strcmp(contentType, ".jpg") == 0) contentType = "image/jpeg";
    else contentType = "application/octet-stream";

    char request[BUFFER_SIZE];
    snprintf(request, BUFFER_SIZE, "POST %s HTTP/1.1\r\nHost: %s\r\nContent-Length: %ld\r\nContent-Type: %s\r\n\r\n",
             path, host, fileSize, contentType);
    strcat(request, fileData);
    if (send(sockfd, request, strlen(request), 0) < 0) DieWithSystemMessage("Send error");
    free(fileData);

    char response[BUFFER_SIZE];
    recv(sockfd, response, BUFFER_SIZE, 0);
}

void parseCommand(char *line, char *method, char *path, char *host, int *port) {
    if (sscanf(line, "%19s %255s %255s %d", method, path, host, port) < 3)
        DieWithSystemMessage("Invalid Command Format");
}

const char *getFileName(const char *path) {
    const char *fileName = strrchr(path, '/');
    return fileName ? fileName + 1 : path;
}

const char *getFileExtension(const char *path) {
    const char *ext = strrchr(path, '.');
    return ext ? ext : "";
}

FILE *openFile(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) DieWithSystemMessage("Error opening the file");
    return file;
}
