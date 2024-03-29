#include "server_utils.h"
#include "file_transmission.h"

void * threadFunction(void * pointer) {
    psocket_plus_filename p = (psocket_plus_filename) pointer;

    printf("%s\n", "Thread was created");
    transmitFile(p->socketHandle, p->filename);
    return 0;
}

int main(int argc, char *argv[])
{
    int sockfd, newsockfd, portno, pid;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;

    char buffer[BUFFER_LENGTH + 1];
    if (argc < 3) {
        sprintf(buffer, "Usage: %s port filename", argv[0]);
        fail(buffer);
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
        if (SERVER_TYPE == 0) {//processes
            pid = fork();
            if (pid < 0) {
                fail("Error while forking new process");
            }
            if (pid == 0)  {
                close(sockfd);
                transmitFile(newsockfd, argv[2]);
                exit(0);
            } else { 
                close(newsockfd);
            }
        } else { //threads
            pthread_attr_t attr;
            pthread_t tid;
            psocket_plus_filename p_str;

            p_str = (psocket_plus_filename)malloc(sizeof(socket_plus_filename));
            p_str->filename = argv[2];
            p_str->socketHandle = newsockfd;
            pthread_attr_init(&attr);
            pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

            if (pthread_create(&tid, &attr, threadFunction, p_str)) {
                fail("Error while creating thread");
            }
        }
    } /* end of while */
    close(sockfd);
    return 0; /* we never get here */
}
