#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <string>
#include <thread>
#include <vector>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

const int BUFFER_SIZE = 1024;




void handleClient(int clientSocket) {
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);

    // Receive the HTTP request from the client
    recv(clientSocket, buffer, BUFFER_SIZE, 0);
    std::string request(buffer);

    // Extract the HTTP method (GET or POST) and requested file
    std::istringstream iss(request);
    std::string method, filename;
    iss >> method >> filename;

    if (method == "GET") {
        // Handle GET request as before
        std::string fullFilePath = "./" + filename;

        std::ifstream file(fullFilePath, std::ios::binary);
        if (file) {
            // File exists, send HTTP 200 OK and the file content
            std::string response = "HTTP/1.1 200 OK\r\n\r\n";
            send(clientSocket, response.c_str(), response.size(), 0);

            while (!file.eof()) {
                file.read(buffer, BUFFER_SIZE);
                send(clientSocket, buffer, file.gcount(), 0);
            }
        } else {
            // File not found, send HTTP 404 Not Found
            std::cout << "Requested File: " << fullFilePath << std::endl;
            std::string response = "HTTP/1.1 404 Not Found\r\n\r\n";
            send(clientSocket, response.c_str(), response.size(), 0);
        }
    } else if (method == "POST") {
        // Handle POST request
        // Extract content length
        std::string line;
        std::size_t contentLength = 0;

        while (std::getline(iss, line) && line != "\r") {
            if (line.find("Content-Length:") == 0) {
                contentLength = std::stoi(line.substr(16));
            }
        }

        // Read the POST data from the body
        std::string postData;
        for (std::size_t i = 0; i < contentLength; ++i) {
            char c;
            recv(clientSocket, &c, 1, 0);
            postData += c;
        }

        // Process the POST data (you can modify this part as needed)
        std::cout << "Received POST data:\n" << postData << std::endl;

        // Save the POST data to a file
        std::ofstream outputFile("/home/sama/Documents/GitHub/MultiThreaded-HTTP-Server/MultiThreaded-HTTP-Server/output.txt");
        if (outputFile.is_open()) {
            outputFile << postData;
            outputFile.close();
            std::cout << "Saved POST data to output.txt" << std::endl;
        } else {
            std::cerr << "Error opening output.txt for writing" << std::endl;
        }

        // Send a simple response
        std::string response = "HTTP/1.1 200 OK\r\n\r\n";
        send(clientSocket, response.c_str(), response.size(), 0);
        send(clientSocket, "POST request received", 22, 0);
    }

    // Close the client socket
    close(clientSocket);
}





int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " port_number\n";
        return 1;
    }

    int port = std::stoi(argv[1]);

    // Create a socket
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        std::cerr << "Error creating socket\n";
        return 1;
    }

    // Bind the socket to an address
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        std::cerr << "Error binding socket\n";
        return 1;
    }

    // Listen for incoming connections
    if (listen(serverSocket, 10) == -1) {
        std::cerr << "Error listening on socket\n";
        return 1;
    }

    std::cout << "Server listening on port " << port << "...\n";

    while (true) {
        // Accept a new connection
        sockaddr_in clientAddr;
        socklen_t clientAddrLen = sizeof(clientAddr);
        int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);
        if (clientSocket == -1) {
            std::cerr << "Error accepting connection\n";
            continue;
        }

        // Create a new thread to handle the client
        std::thread(handleClient, clientSocket).detach();
    }

    // Close the server socket (Note: This part may not be reached in this simple example)
    close(serverSocket);

    return 0;
}
