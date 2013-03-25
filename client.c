#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <netdb.h> 

#define BUFFER_LENGTH (10 * 1024)

void acceptFile(int socketHandle);

void fail(const char* message) {
    fprintf(stderr, "%s\n", message);
    exit(1);
}

int main(int argc, char *argv[])
{
    int sockfd, portno;
    int bytesWritten, bytesRead;
    int fileHandle;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[BUFFER_LENGTH + 1];
    if (argc < 3) {
        sprintf(buffer, "Usage: %s hostname port", argv[0]);
        fail(buffer);
    }

    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) {
        fail("Error while opening socket");
    }

    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fail("Error: no such host");
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(portno);


    if ((errorType = connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)) {
        fail("Error while connecting");
    }

    acceptFile(sockfd);
    close(sockfd);
    return 0;
}

void acceptFile(int socketHandle)
{
    int fileLen, nameLen;
    int bytes_read, bytes_written;
    int fileHandle;
    char buffer[BUFFER_LENGTH + 1];
    char filename[BUFFER_LENGTH + 1];

    bytes_read = read(socketHandle, &nameLen, sizeof(nameLen));
    if (bytes_read != sizeof(nameLen)) {
        fail("Can't read filename length");
    }
    printf("Filename length = %d\n", nameLen);

    bzero(buffer, sizeof(BUFFER_LENGTH));
    bytes_read = read(socketHandle, buffer, nameLen);
    if (bytes_read != nameLen) {
        fail("Can't read filename");
    }
    printf("Filename = %s\n", buffer);

    bzero(filename, sizeof(filename)); 
    strcpy(filename, "accepted_files/");
    strcat(filename, buffer);

    bytes_read = read(socketHandle, &fileLen, sizeof(fileLen));
    if (bytes_read != sizeof(fileLen)) {
        fail("Can't read file length");
    }
    printf("File length = %d\n", fileLen);
    
    fileHandle = open(filename, O_WRONLY | O_CREAT | O_TRUNC, S_IWRITE | S_IREAD);
    if (fileHandle < 0) {
        fail("Can't open file to write");
    }

    while (fileLen > 0) {
        bzero(buffer, sizeof(buffer));

        bytes_read = read(socketHandle, buffer, BUFFER_LENGTH);
        if (bytes_read < 0) {
            fail("Can't read file from socket");
        }

        bytes_written = write(fileHandle, buffer, bytes_read);
        if (bytes_written < 0) {
            fail("Can't write data to file");
        }

        fileLen -= bytes_read;
    }
    close(fileHandle);
    printf("File %s is transmitted\n", filename);

    bytes_written = write(socketHandle, "Transmission finished", 23);
    if (bytes_written < 0) {
        fail("Transmission is not finished really. Can't write final message");
    }
}
