void transmitFile(int sockfd, const char * filename)
{
    int fileLength, filename_length, bytesWritten, fileHandle;
    int bytesToSend, bytesRead;    
    char buffer[BUFFER_LENGTH + 1];
    if (access(filename, F_OK) == -1) {
        sprintf(buffer, "Error: file %s does not exist", filename);
        fail(buffer);
    }
    filename_length = strlen(filename);
    fileLength = getFileLength(filename);

    bytesWritten = write(sockfd, &filename_length, sizeof(int));
    if (bytesWritten < 0) { 
        fail("Error while sending length of filename");
    }

    bytesWritten = write(sockfd, filename, strlen(filename));
    if (bytesWritten < 0) { 
        fail("Error while sending filename");
    }

    bytesWritten = write(sockfd, &fileLength, sizeof(int));
    if (bytesWritten < 0) { 
        fail("Error while sending length of file");
    }

    fileHandle = open(filename, O_RDONLY);
    if (fileHandle < 0) {
        fail("Can't open file for reading");
    }

    while (fileLength > 0) {
       bzero(buffer, sizeof(buffer));

       bytesToSend = read(fileHandle, buffer, BUFFER_LENGTH);
       if (bytesToSend < 0) {
           fail("Can't read file");
       }

       bytesWritten = write(sockfd, buffer, bytesToSend);
       if (bytesWritten < 0) {
           fail("Can't write file to socket");
       }

       fileLength -= bytesToSend;
    }

    bzero(buffer, BUFFER_LENGTH);
    bytesRead = read(sockfd, buffer, BUFFER_LENGTH - 1);
    if (bytesRead < 0) { 
        fail("Error while getting final message");
    }
    printf("%s\n",buffer);
    close(sockfd);
}
