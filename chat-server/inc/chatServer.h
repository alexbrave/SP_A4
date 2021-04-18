#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdbool.h>

#define PORT 5000
#define MAX_NUM_CLIENTS 10

#define IP_DELIM "|"
#define NAME_DELIM ";"
#define NULL_TERM '\0'

#define INDEX_CLIENT_NAME 0
#define INDEX_MESSAGE 1
#define NUM_MESSAGE_ELEMENTS 2

#define CLIENT_EXIT_MESSAGE ">>bye<<"

#define MAX_IP_ADDR_LENGTH 16

// time out interval time (3 intervals)
#define SERVER_TIMEOUT_INTERVAL_TIME 10
#define SERVER_INTERVAL_COUNT 3

#define EXIT_WITH_ERROR 1
#define EXIT_OK 0

typedef struct clientInfo
{
    int clientSocket;                         // the socket of the client
    char clientIPAddress[MAX_IP_ADDR_LENGTH]; // the client's ip address as a string
} clientInfo;


/*          GLOBAL VARIABLES           */

// keep track of connected clients
clientInfo connectedClients[MAX_NUM_CLIENTS];

// global to keep track of the number of connections
static int nClients = 0;
static int nNoConnections = 0;

// store threads
pthread_t clientServiceThreads[MAX_NUM_CLIENTS];

void *handleClient(void *socket_to_handle);
void parseClientMessage(char *fromClient, char **splitMessage);
char *makeMessage(char *clientIP, char *clientName, char *message);
bool isMessageValid(char *fromClient);
void removeClientSocket(int socketToRemove);
char *getClientIP(int clientSocket);