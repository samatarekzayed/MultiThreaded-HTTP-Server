#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H

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

void DieWithSystemMessage(const char *msg);
long getFileSize(FILE *file);
int parseCommand(const char *line, char *method, char *path, char *host, int *port_number);
int connectToServer(const char *host, int port_number);
void validateArguments(int argc);
FILE *openInputFile(const char *filename);
void saveResponseToFile(const char *response, int total_received);

#endif /* HTTP_CLIENT_H */
