#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <netdb.h> 

#define BUFFER_LENGTH (10 * 1024)


void fail(const char* message) {
    fprintf(stderr, "%s\n", message);
    exit(1);
}

int getFileLength(const char* filename) {
    struct stat st;
    stat(filename, &st);
    return st.st_size;
}

int main(int argc, char *argv[])
{
    int sockfd, portno;
    int file_length, filename_length;
    int bytesWritten, bytesRead;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[BUFFER_LENGTH];
    if (argc < 4) {
        sprintf(buffer, "Usage: %s hostname port filename", argv[0]);
        fail(buffer);
    }

    if (access(argv[3], F_OK) == -1) {
        sprintf(buffer, "Error: file %s does not exist", argv[3]);
        fail(buffer);
    }
    filename_length = strlen(argv[3]);
    file_length = getFileLength(argv[3]);

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

    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) {
        fail("Error while connecting");
    }

    bytesWritten = write(sockfd, &filename_length, sizeof(int));
    if (bytesWritten < 0) { 
        fail("Error while sending length of filename");
    }

    bytesWritten = write(sockfd, argv[3], strlen(argv[3]));
    if (bytesWritten < 0) { 
        fail("Error while sending filename");
    }

    bytesWritten = write(sockfd, &file_length, sizeof(int));
    if (bytesWritten < 0) { 
        fail("Error while sending length of file");
    }

    bzero(buffer, BUFFER_LENGTH);
    bytesRead = read(sockfd, buffer, BUFFER_LENGTH - 1);
    if (bytesRead < 0) { 
        fail("Error while getting final message");
    }
    printf("%s\n",buffer);
    close(sockfd);
    return 0;
}
