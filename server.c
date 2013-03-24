#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

void acceptFile(int socketHandle); /* function prototype */

void fail(const char* message) {
    fprintf(stderr, "%s\n", message);
    exit(1);
}

int main(int argc, char *argv[])
{
    int sockfd, newsockfd, portno, pid;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;

    if (argc < 2) {
        fail("Port is not provided");
    }
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        fail("Error while opening socket");
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) { 
        fail("Error while binding address to socket");
    }

    listen(sockfd, 5);
    clilen = sizeof(cli_addr);
    while (1) {
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd < 0) {
            fail("Error while accepting connection");
        }

        pid = fork();
        if (pid < 0) {
            fail("Error while forking new process");
        }

        if (pid == 0)  {
            close(sockfd);
            acceptFile(newsockfd);
            exit(0);
        } else { 
            close(newsockfd);
        }
    } /* end of while */
    close(sockfd);
    return 0; /* we never get here */
}

void acceptFile(int socketHandle) {
    int fileLen, nameLen;
    int bytesRead, bytesWritten;
    char buffer[10 * 1024];

    bytesRead = read(socketHandle, &nameLen, sizeof(nameLen));
    if (bytesRead != sizeof(nameLen)) {
        fail("Can't read filename length");
    }
    printf("Filename length = %d\n", nameLen);

    bzero(buffer, 10 * 1024);
    bytesRead = read(socketHandle, buffer, nameLen);
    if (bytesRead != nameLen) {
        fail("Can't read filename");
    }
    printf("Filename = %s\n", buffer);

    bytesRead = read(socketHandle, &fileLen, sizeof(fileLen));
    if (bytesRead != sizeof(fileLen)) {
        fail("Can't read file length");
    }
    printf("File length = %d\n", fileLen);
    
    bytesWritten = write(socketHandle, "Transmission finished", 23);
    if (bytesWritten < 0) {
        fail("Transmission is not finished really. Can't write final message");
    }
}

