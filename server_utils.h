#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#define BUFFER_LENGTH (10 * 1024)
#define SERVER_TYPE 1 // 0 - for process, 1 - for threads

typedef struct _socket_plus_filename
{
    int socketHandle;
    const char * filename;
} socket_plus_filename, *psocket_plus_filename;

void fail(const char* message)
{
    fprintf(stderr, "%s\n", message);
    exit(1);
}

int getFileLength(const char* filename) {
    struct stat st;
    stat(filename, &st);
    return st.st_size;
}
