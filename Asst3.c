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
#include <time.h>
#include <math.h>

//server setup
int checkValidPort(char* input);    //evaluate if the port number specified is a number
typedef struct KKJ                  //KKJ linked list to store contents of joke file
{
    int total;
    char* setup;
    char* punchLine;
    struct KKJ* next;
} KKJ;
struct KKJ* readJokeFile(char* jokeFile);   //reads in joke file and stores the jokes
void tellKKJ(int connectFD, KKJ* jokes);    //main loop to send and receive parts of the KKJ to and from the client

//KKJ operations
char** selectJoke(KKJ* jokes);  //randomly choose a joke to send and prepare it to be sent

int checkFormat(int connectFD, char* buffer, int bufferSize, char* expect, int targetLen);
//receive message from client and evaluates for errors, returns error value for sendErrorMSG()

void sendErrorMSG(int connectFD, int M, int error);
//gets error value from checkFormat(), sends error message to client

int checkREG(int connectFD, char* buffer, int bufferSize, char* format, int index);
//checks the message type for invalid format, returns -3 if so

int formatEquals(char* buffer, char* format, int index);
//string comparison function that helps checkREG() evaluate message type

char* checkLength(int connectFD, char* buffer, int bufferSize, int index);
//checks the length portion of message for invalid format, returns either valid length or NULL
//-2 is returned to tellKKJ() to indicate incorrect content length

char* getLength(char* buffer, int index);
//obtains length value for message content, helps with checkLength()

int checkContent(int connectFD, char* buffer, int bufferSize, char* length, char* expect, int index, int targetLen);
//checks content of message against expected content, returns -1 if content does not match
//and -3 for missing ending punctution in the case of M5

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
        if(argc > 3)
        {
            fprintf(stderr, "\n\033[1;31mError:\t\033[0mOnly up to 1 port number and 1 file can be accepted\n\n");
            return 1;
        }
        if(checkValidPort(argv[1]) == -1)
        {
            fprintf(stderr, "\n\033[1;31mError:\t\033[0mPort specified must be a number\n\n");
            return 1;
        }
    }    

    //being setting up the server on the specified port
    printf("\nStarting up the KKJ server on port: %s\n", argv[1]);

    struct sockaddr_in serverAddr, clientAddr;

    int socketFD = socket(AF_INET, SOCK_STREAM, 0);
    if(socketFD == -1)
    {
        fprintf(stderr, "\n\033[1;31mError:\t\033[0mServer socket could not be established\n\n");
        exit(0);
    }
    printf("...Server socket established\n");
    bzero(&serverAddr, sizeof(serverAddr));

    int opt = 1;
    if(setsockopt(socketFD, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) == -1)
    {
        fprintf(stderr, "\n\033[1;31mError:\t\033[0mServer socket opt could not be set\n\n");
        exit(0);
    }
    printf("...Server socket opt set\n");

    //assigning IP and port number
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(atoi(argv[1]));

    //bind socket to IP
    if(bind(socketFD, (struct sockaddr*) &serverAddr, sizeof(serverAddr)) == -1)
    {
        fprintf(stderr, "\n\033[1;31mError:\t\033[0mFailed to bind server socket\n\n");
        exit(0);
    }
    printf("...Server socket binded\n");

    //read contents of joke file if available, create linked list  to store the KKJs
    KKJ* jokes = readJokeFile(argv[2]);

    //main server-client connection loop
    while(1)
    {
        //being server listening
        if(listen(socketFD, 3) == -1)
        {
            fprintf(stderr, "\n\033[1;31mError:\t\033[0mServer listen failed\n\n");
            close(socketFD);
            exit(0);
        }
        printf("...Server now listening\n");
        
        //get client connection
        socklen_t clientLength = sizeof(clientAddr);
        int connectFD = accept(socketFD, (struct sockaddr*) &clientAddr, &clientLength);
        if(connectFD == -1)
        {
            fprintf(stderr, "\n\033[1;31mError:\t\033[0mServer failed to accept client\n\n");
            exit(0);
        }
        printf("...Server accepted the client\n\n");
        
        //perform KKJ message operation
        tellKKJ(connectFD, jokes);

        printf("...Ending connection with client\n\n");
        close(connectFD);
    }

    //deallocate memory for jokes linked list
    //Note: shutting down with ^C means that many blocks will not be freed!
    KKJ* current = jokes;
    while(current != NULL)
    {
        KKJ* temp = current;
        free(current->setup);
        free(current->punchLine);
        current = current->next;
        free(temp);
    }

    close(socketFD);
    return 0;
}

int checkValidPort(char* input)
{
    for(int i = 0; i < strlen(input); i++)
    {
        if(isdigit(input[i]) == 0)
        {
            return -1;
        }
    }
    return 1;
}

struct KKJ* readJokeFile(char* jokeFile)
{
    FILE* filePointer = fopen(jokeFile, "r");
    if(filePointer == NULL)
    {
        //sprintf("Failed to open: %s\n", jokeFile);
        return NULL;
    }

    //intialize buffers
    char buffer[1000];
    char setup[1000];
    char punchLine[1000];
    
    KKJ* head = NULL;
    int total = 0;

    while(!feof(filePointer))
    {
        bzero(buffer, 1000);
        bzero(setup, 1000);
        fgets(buffer, 1000, filePointer);
        strcpy(setup, buffer);

        bzero(buffer, 1000);
        bzero(punchLine, 1000);
        fgets(buffer, 1000, filePointer);
        strcpy(punchLine, buffer);

        if(strlen(setup) > 0 && strlen(punchLine) > 0)  //make sure buffers are filled
        {
            KKJ* insert = malloc(sizeof(KKJ));
            total++;
            insert->total = total;

            insert->setup = malloc(strlen(setup) + 1);
            strcpy(insert->setup, setup);
            insert->setup[strlen(setup)] = 0;
            insert->setup[strlen(setup) - 1] = '\0';

            insert->punchLine = malloc(strlen(punchLine) + 1);
            strcpy(insert->punchLine, punchLine);
            insert->punchLine[strlen(punchLine)] = 0;
            insert->punchLine[strlen(punchLine) - 1] = '\0';
            
            insert->next = head;
            head = insert;
        }
        fgets(buffer, 1000, filePointer);
    }
    /*
    printf("\n");
    KKJ* current = head;
    while(current != NULL)
    {
        printf("%d %s\n  %s\n\n", current->total, current->setup, current->punchLine);
        current = current->next;
    }
    */
    fclose(filePointer);
    return head;
}

void tellKKJ(int connectFD, KKJ* jokes)
{
    int bufferSize = 100;
    char* buffer = malloc(bufferSize + 1);
    bzero(buffer, bufferSize);  

    char** messages;

    if(jokes == NULL)
    {
        messages = malloc(5 * sizeof(char*));
        int i = 0;
        for(i = 0; i < 5; i++)
        {
            messages[i] = malloc(20);       //allocate memory for messages
        }
        strcpy(messages[0], "Knock, knock.");   //M0
        strcpy(messages[1], "Who's there?");    //M1
        strcpy(messages[2], "Deja.");           //M2
        strcpy(messages[3], "Deja, who?");      //M3
        strcpy(messages[4], "Knock knock!");    //M4
    }
    else
    {
       messages = selectJoke(jokes);
    }

    //main loop for sending and receving messages to and from client
    int error;
    int M = 0;
    while(M < 5)
    {
        //prepare buffer to send message to client
        bzero(buffer, bufferSize);
        strcpy(buffer, "REG|");

        char length[sizeof(messages[M]) + 1];
        sprintf(length, "%d", (int)strlen(messages[M]));
        strcat(buffer, length);

        strcat(buffer, "|");
        strcat(buffer, messages[M]);
        strcat(buffer, "|");

        printf("Sending '%s' to client...\n", buffer);
        write(connectFD, buffer, bufferSize);
        bzero(buffer, bufferSize);
        M++;

        //receive message from client and evaluate
        read(connectFD, buffer, 100);  
        if(M == 5)  //handle case for custom message 5 content
        {
            error = checkFormat(connectFD, buffer, 100, NULL, -1);
        }
        else     
        {
            char expect[strlen(messages[M]) + 2];
            strcpy(expect, messages[M]);
            strcat(expect, "|");
            error = checkFormat(connectFD, buffer, 100, expect, strlen(messages[M]));
        }
        bzero(buffer, bufferSize);
        if(error < 0)               //send out error message to client if error detected
        {
            sendErrorMSG(connectFD, M, error);
            
            for(int i = 0; i < 5; i++)
            {
                //printf("deallocating: %s\n", messages[i]);
                free(messages[i]);
            }
            free(messages);
            free(buffer);
            return;
        }
        M++;
    }
  
    printf("\nKnock knock joke succesfully delivered :D\n");

    for(int i = 0; i < 5; i++)
    {
        //printf("deallocating: %s\n", messages[i]);
        free(messages[i]);
    }
    free(messages);
    free(buffer);
    return;
}

char** selectJoke(KKJ* jokes)
{
    
    char** messages = malloc(5 * sizeof(char*));

    KKJ* current = jokes;
    srand(time(NULL));
    int select = rand() % (jokes->total);

    for(int i = 0; i < select; i++)   //get random joke from the joke list
    {   
        current = current->next;
    }
    
    //fill messages array with prepared KKJ from joke file
    printf("Preparing the following KKJ from joke file...\n");
    
    messages[0] = malloc(20);
    strcpy(messages[0], "Knock, knock.");   //M0
    printf("%s\n", messages[0]);

    messages[1] = malloc(20);
    strcpy(messages[1], "Who's there?");    //M1
    printf("%s\n", messages[1]);

    messages[2] = malloc(strlen(current->setup) + 1);
    strcpy(messages[2], current->setup);    //M2
    printf("%s\n", messages[2]);

    messages[3] = malloc(strlen(current->setup) + 5 + 1);
    strcpy(messages[3], current->setup); //M3
    messages[3][strlen(current->setup)] = 0;
    messages[3][strlen(current->setup) - 1] = '\0';
    strcat(messages[3], ", who?");
    printf("%s\n", messages[3]);

    messages[4] = malloc(strlen(current->punchLine) + 1);
    strcpy(messages[4], current->punchLine);    //M4
    printf("%s\n\n", messages[4]);

    return messages;
}

int checkFormat(int connectFD, char* buffer, int bufferSize, char* expect, int targetLen)
{
    //check if the string in the buffer matches REG format
    if(checkREG(connectFD, buffer, bufferSize, "ERR|", 0) > 0)
    {
        return -4;
    }
    int index = checkREG(connectFD, buffer, bufferSize, "REG|", 0);
    if(index == -1)
    {
        return -3;
    }
    
    char* length = checkLength(connectFD, buffer, bufferSize, index);
    if(length == NULL)
    {
        return -3;
    }

    //handle case for M5 being custom message
    if(expect == NULL && targetLen == -1)
    {
        int error = checkContent(connectFD, buffer, bufferSize, length, expect, index, atoi(length));
        free(length);
        return error;
    }

    if(atoi(length) != targetLen)
    {
        free(length);
        return -2;
    }

    if(checkContent(connectFD, buffer, bufferSize, length, expect, index, targetLen) == -1)
    {
        free(length);
        return -1;
    }

    free(length);
    return 1;
}

void sendErrorMSG(int connectFD, int M, int error)
{
    char errorCodes[18][5] = 
                        {"M0CT", "M0LN", "M0FT",
                         "M1CT", "M1LN", "M1FT",
                         "M2CT", "M2LN", "M2FT",
                         "M3CT", "M3LN", "M3FT",
                         "M4CT", "M4LN", "M4FT",
                         "M5CT", "M5LN", "M5FT"};
    
    switch(error)
    {
        case -1:
            printf("\033[1;31mError:\033[0m Client sent incorrect content\n");
            break;
        case -2:
            printf("\033[1;31mError:\033[0m Client sent incorrect length\n");
            break;
        case -3:
            printf("\033[1;31mError:\033[0m Client sent incorrect format\n");
            break;
        case -4:
            printf("\033[1;31mNote:\033[0m Client sent an error message\n");
            return;
    }
    //send error message to client
    char buffer[10];
    bzero(buffer, 10);
    strcpy(buffer, "ERR|");
    strcat(buffer, errorCodes[M*3 + (-1 * error) - 1]);
    strcat(buffer, "|");
    printf("Sending '%s' to client...\n", buffer);
    write(connectFD, buffer, 10);
}

int checkREG(int connectFD, char* buffer, int bufferSize, char* format, int index)
{
    index = formatEquals(buffer, format, index);
    while(index < 4)
    {
        if(index == -1)
        {
            return -1;
        }
        bzero(buffer, 100);
        read(connectFD, buffer, bufferSize);
        if(strlen(buffer) > 0)
        {
            index = formatEquals(buffer, format, index);
            if(index == 4)
            {
                index = (strstr(buffer, "|")) - &buffer[0] + 1;
                break;
            }
        }
    }
    return index;
}

int formatEquals(char* buffer, char* format, int index)
{
    int upTo = strlen(format) - index;
    if(strlen(buffer) < upTo)
    {
        upTo = strlen(buffer);
    }
    int i = 0;
    while(i < upTo)
    {
        if(buffer[i] != format[index + i])
        {
            return -1;
        }
        i++;
    }
    return index + i;
}

char* checkLength(int connectFD, char* buffer, int bufferSize, int index)
{
    char* length = NULL;
    length = getLength(buffer, index);
    if(length == NULL)
    {
        free(length);
        return NULL;
    }
    if(buffer[index + strlen(length)] != '|')
    {
        char* terminate = NULL;
        while(terminate == NULL)
        {
            bzero(buffer, 100);
            read(connectFD, buffer, bufferSize);
            if(strlen(buffer) > 0)
            {
                char* nextLength = getLength(buffer, 0);
                if(nextLength == NULL)
                {
                    free(length);
                    free(nextLength);
                    return NULL;
                }
                char* newLength = malloc(strlen(length) + strlen(nextLength) + 1);
                strcpy(newLength, length);
                free(length);
                strcat(newLength, nextLength);
                free(nextLength);
                length = newLength;
                terminate = strstr(buffer, "|");
            }
        }
    }
    return length;
}

char* getLength(char* buffer, int index)
{
    char* length = malloc(strlen(buffer) + 1);
    strcpy(length, "");
    int i = 0;
    for(i = 0; index + i < strlen(buffer); i++)
    {
        if(isdigit(buffer[index + i]))
        {
            length[i] = buffer[index + i];
        }
        else if(buffer[index + i] == '|')
        {
            length[i] = '\0';
            return length;
        }
        else
        {
            free(length);
            return NULL;
        }
    }
    length[i] = '\0';
    return length;
}

int checkContent(int connectFD, char* buffer, int bufferSize, char* length, char* expect, int index, int targetLen)
{
    //handle cases for when buffer starts with message content or with non-message content
    if(strstr(buffer, length) != NULL)
    {
        index = (strstr(buffer, length) - &(buffer[0])) + strlen(length) + 1;
    }
    else if(strstr(buffer, "|") != NULL)
    {
        index = strstr(buffer, "|") - &(buffer[0]) + 1;
    }

    //build the message contents for comparision against expected message content
    int count = 0;
    char* content = malloc(strlen(buffer) - index + 1);
    strcpy(content, "");
    if(index < strlen(buffer))
    {
        int i = 0; 
        while(index + i < strlen(buffer))
        {
            content[i] = buffer[index + i];
            count++;
            i++;
        }
        content[i] = '\0';
    }
    while(count <= targetLen && strstr(content, "|") == NULL)
    {
        bzero(buffer,bufferSize);
        read(connectFD, buffer, bufferSize);
        char* newContent = malloc(strlen(content) + strlen(buffer) + 1);
        strcpy(newContent, content);
        free(content);
        strcat(newContent, buffer);
        content = newContent;
        count = count + strlen(buffer);
    }
    //handle content errors
    if(expect == NULL)          //handle custom message 5 error
    {
        if(count != targetLen + 1)  //length not matching, send error
        {
            free(content);
            return -2;
        }
        char* c = (strchr(content, '|') - 1);
        if(ispunct(*c))
        {
            printf("Received 'REG|%d|%s' from client\n", targetLen, content);
            free(content);
            return 1;
        }
        free(content);  //missing punctuation, send error
        return -1;
    }
    if(strcmp(expect, content) != 0)    //message content does not match, send error
    {
        free(content);
        return -1;
    }

    printf("Received 'REG|%d|%s' from client\n", targetLen, content);
    free(content);
    return 1;
}


