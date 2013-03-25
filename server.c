#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

#define BUFFER_LENGTH (10 * 1024)
#define SERVER_TYPE_THREADS 0
#define SERVER_TYPE_PROCESSES 1

void acceptFile(int socketHandle);

void fail(const char* message)
{
    fprintf(stderr, "%s\n", message);
    exit(1);
}

int main(int argc, char *argv[])
{
    int sockfd, newsockfd, portno, pid, server_type;
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
    if (!strcmp(argv[2], "-t")|| !(strcmp(argv[2], "--threads"))
                                    || !(strcmp(argv[2], ""))) {
        server_type = 0;
    } else if (!strcmp(argv[2], "-p") || !strcmp(argv[2], "--processes")) {
        server_type = 1;
    } else {
        fail("Unknown third parameter! Should be -p or -t");
    }

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
        if (server_type == SERVER_TYPE_PROCESSES) {
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
        }
        else {
            //TODO: work with threads
        }
    } /* end of while */
    close(sockfd);
    return 0; /* we never get here */
}

void acceptFile(int socketHandle)
{
    int file_length, name_length;
    int bytes_read, bytes_written;
    int file_handle;
    char buffer[BUFFER_LENGTH + 1];
    char filename[BUFFER_LENGTH + 1];

    bytes_read = read(socketHandle, &name_length, sizeof(name_length));
    if (bytes_read != sizeof(name_length)) {
        fail("Can't read filename length");
    }
    printf("Filename length = %d\n", name_length);

    bzero(buffer, sizeof(BUFFER_LENGTH));
    bytes_read = read(socketHandle, buffer, name_length);
    if (bytes_read != name_length) {
        fail("Can't read filename");
    }
    printf("Filename = %s\n", buffer);

    bzero(filename, sizeof(filename)); 
    strcpy(filename, "accepted_files/");
    strcat(filename, buffer);

    bytes_read = read(socketHandle, &file_length, sizeof(file_length));
    if (bytes_read != sizeof(file_length)) {
        fail("Can't read file length");
    }
    printf("File length = %d\n", file_length);
    
    file_handle = open(filename, O_WRONLY | O_CREAT | O_TRUNC, S_IWRITE | S_IREAD);
    if (file_handle < 0) {
        fail("Can't open file to write");
    }

    while (file_length > 0) {
        bzero(buffer, sizeof(buffer));

        bytes_read = read(socketHandle, buffer, BUFFER_LENGTH);
        if (bytes_read < 0) {
            fail("Can't read file from socket");
        }

        bytes_written = write(file_handle, buffer, bytes_read);
        if (bytes_written < 0) {
            fail("Can't write data to file");
        }

        file_length -= bytes_read;
    }
    close(file_handle);
    printf("File %s is transmitted\n", filename);

    bytes_written = write(socketHandle, "Transmission finished", 23);
    if (bytes_written < 0) {
        fail("Transmission is not finished really. Can't write final message");
    }
}

