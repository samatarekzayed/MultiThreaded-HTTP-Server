#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024

void DieWithUserMessage(const char *msg, const char *detail) {
    fputs(msg, stderr);
    fputs(": ", stderr);
    fputs(detail, stderr);
    fputc("\n", stderr);
    exit(1);
}

void DieWithSystemMessage(const char *msg) {
    perror(msg);
    exit(1);
}

void handle_get(const char *file_path, const char *host_name, int port_number) {
    //TODO remove the socket creation part as this will be given by the server listening to this client.
    // Create socket
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        DieWithSystemMessage("Socket Creation Failed");
    }

    // Specify server information
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port_number);
    int addr_status = inet_pton(AF_INET, host_name, &server_address.sin_addr);

    if (addr_status == 0) {
        DieWithSystemMessage("Source not valid");
    } else if(addr_status == -1) {
        DieWithSystemMessage("Unknown address family");
    }

    // Connect to server
    if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1) {
        DieWithSystemMessage("Connection failed");
    }

    // Prepare GET request
    char buffer[BUFFER_SIZE];
    snprintf(buffer, BUFFER_SIZE, "GET %s HTTP/1.1\r\nHost: %s\r\n\r\n", file_path, host_name);

    // Send GET request
    send(client_socket, buffer, strlen(buffer), 0);

    // Receive and display response
    char response[BUFFER_SIZE];
    int bytes_received = recv(client_socket, response, BUFFER_SIZE - 1, 0);
    
    if (bytes_received < 0) {
        perror("recv() failed");
    }

    while(bytes_received > 0) {
        ssize_t bytes_sent = send(client_socket, buffer, bytes_received, 0);
        
        if(bytes_sent < 0) 
            DieWithSystemMessage("send() failed");
        else if(bytes_sent != bytes_received)
            DieWithUserMessage("send()", "sent unexpected number of bytes");

        bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
        if(bytes_received < 0)
            DieWithSystemMessage("recv() failed");
        FILE *fp = fopen("local_filename", "a");
        fprintf(fp, "%s", response);
        fclose(fp);
    }

    // Close socket
    close(client_socket);
}

void handle_post(const char *file_path, const char *host_name, int port_number) {
    // Create socket
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        DieWithSystemMessage("Socket creation failed");
    }

    // Specify server information
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port_number);
    int addr_status = inet_pton(AF_INET, host_name, &server_address.sin_addr);
    
    if(addr_status == 0) {
        DieWithSystemMessage("Source not valid");
    } else if(addr_status == -1) {
        DieWithSystemMessage("Unknown address family");
    }
    
    // Connect to server
    if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1) 
        DieWithSystemMessage("Connection failed");
    

    // Open the file for reading
    FILE *file = fopen(file_path, "r");
    if (file == NULL) 
        DieWithSystemMessage("Error opening file");
    

    // Calculate file size
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Allocate memory for file content to be sent in the POST request
    char *file_content = (char *)malloc(file_size + 1);
    if (file_content == NULL) {
        fclose(file);
        DieWithSystemMessage("Memory allocation failed");
    }

    // Read file content
    fread(file_content, 1, file_size, file);
    fclose(file);
    file_content[file_size] = '\0';

    // Construct the POST request
    char buffer[BUFFER_SIZE];
    snprintf(buffer, BUFFER_SIZE, "POST %s HTTP/1.1\r\nHost: %s\r\nContent-Length: %ld\r\n\r\n%s",
            file_path, host_name, file_size, file_content);

    // Send POST request
    send(client_socket, buffer, strlen(buffer), 0);

    // Receive and display response
    char response[BUFFER_SIZE];
    int bytes_received = recv(client_socket, response, BUFFER_SIZE - 1, 0);
    if (bytes_received < 0) {
        DieWithSystemMessage("recv() failed");
    }

    while (bytes_received > 0) {
        response[bytes_received] = '\0';
        printf("%s", response); // Display received data
        int bytes_received = recv(client_socket, response, BUFFER_SIZE - 1, 0);
        if (bytes_received < 0)
            DieWithSystemMessage("recv() failed");
    }

    // Free allocated memory
    free(file_content);

    // Close socket
    close(client_socket);
}

int main(int argc, char *argv[]) {
    if (argc < 4 || argc > 5) {
        fprintf("Usage: %s file-path host-name [port-number]\n", argv[0]);
    }

    const char *file_path = argv[1];
    const char *host_name = argv[2];
    int port_number = (argc == 5) ? atoi(argv[3]) : 80;

    // Check if the command is GET or POST and call respective function
    if (strcmp(argv[1], "client_get") == 0) {
        handle_get(file_path, host_name, port_number);
    } else if (strcmp(argv[1], "client_post") == 0) {
        handle_post(file_path, host_name, port_number);
    } else {
        DieWithSystemMessage("Invalid command\n");
    }

    return EXIT_SUCCESS;
}

