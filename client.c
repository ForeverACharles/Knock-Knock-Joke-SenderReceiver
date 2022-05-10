#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

void getKKJ(int socketFD);
int sendClientInput(int socketFD, char** buffer, int bufferSize, int n);

int main(int argc, char* argv[])
{
    //input checking
    if(argc < 2)
    {
        fprintf(stderr, "\n\033[1;31mError:\t\033[0mNo port number provided\n\n");
        return 1;
    }
    if(argc > 2)
    {
        fprintf(stderr, "\n\033[1;31mError:\t\033[0mOnly 1 port number can be accepted\n\n");
        return 1;
    }

    struct sockaddr_in serverAddr, clientAddr;
    int socketFD = socket(AF_INET, SOCK_STREAM, 0);
    if(socketFD == -1)
    {
        fprintf(stderr, "\n\033[1;31mError:\t\033[0mServer socket could not be established\n\n");
        exit(0);
    }
    printf("...Server socket established\n");
    bzero(&serverAddr, sizeof(serverAddr));

    //assigning IP and port number
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serverAddr.sin_port = htons(atoi(argv[1]));

    //connect 
    if(connect(socketFD, (struct sockaddr*) &serverAddr, sizeof(serverAddr)) == -1)
    {
        fprintf(stderr, "\n\033[1;31mError:\t\033[0mFailed to connect to server\n\n");
        exit(0);
    }
    printf("...Connected to the server\n");

    getKKJ(socketFD);

    close(socketFD);

    return 0;
}

void getKKJ(int socketFD)
{
    char input[10000];
    int bufferSize = 10000;
    char* buffer = malloc(bufferSize);
    bzero(buffer, bufferSize);
    printf("\n");

    int i = 0;
    for(i = 0; i < 3; i++)
    {
        //get M0 - Knock knock.
        read(socketFD, buffer, bufferSize);
        printf("Received '%s' from server...\n", buffer);
        if(strstr(buffer, "ERR") != NULL)
        {
            free(buffer);
            return;
        }

        //send M1 - Who's there?
        if(sendClientInput(socketFD, &buffer, bufferSize, 1) == -1)
        {
            break;
        }
    }

    

    free(buffer);
}   

int sendClientInput(int socketFD, char** buffer, int bufferSize, int n)
{
    while(n > 0)
    {
        char input[10000];
        if(fgets(input, 10001, stdin) != NULL)
        {
            int i;
            for(i = 0; i < sizeof(input); i++)
            {
                if(input[i] == '\n')
                {
                    input[i] = '\0';
                    input[i + 1] = 0;
                    break;
                }
            }
            //printf("Got client input: '%s'\n", input);
        }
        bzero(*buffer, bufferSize);
        strcpy(*buffer, input);
        printf("Sending '%s' to server...\n\n", *buffer);
        int w = write(socketFD, *buffer, bufferSize);
        if(strstr(*buffer, "ERR") != NULL)
        {
            bzero(*buffer, bufferSize);
            return -1;
        }
        bzero(*buffer, bufferSize);
        n--;
    }
    return 1;
}