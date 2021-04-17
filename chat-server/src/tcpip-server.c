/*
 *  FILE          : tcip-server.c
 *  PROJECT       : SENG2030-21W-Sec1-System Programming - Assignment #4
 *  PROGRAMMER    : Andrey Takhtamirov, Alex Braverman
 *  FIRST VERSION : April 16, 2021 
 *  DESCRIPTION   : 
 *			This file contains the main logic for the tcip-server application.
 *        this program was built on Sean's "sockets-chat" program given in class.
 *        tcip-server is a chat server to let 2-10 clients communicate with each other
 *        over tcp/ip on port 5000.
 *	
*/

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

void *handleClient(void *socket_to_handle);
void parseClientMessage(char *fromClient, char **splitMessage);
char *makeMessage(char *clientIP, char *clientName, char *message);
bool isMessageValid(char *fromClient);
void removeClientSocket(int socketToRemove);
char *getClientIP(int clientSocket);

// keep track of the sockets of connected clients
//int clientSockets[MAX_NUM_CLIENTS] = {0};

typedef struct clientInfo
{
  int clientSocket;
  char clientIPAddress[MAX_IP_ADDR_LENGTH];
} clientInfo;

clientInfo connectedClients[MAX_NUM_CLIENTS];

// global to keep track of the number of connections
static int nClients = 0;
static int nNoConnections = 0;

pthread_t clientServiceThreads[MAX_NUM_CLIENTS];

/* Watch dog timer - to keep informed and watch how long the server goes without a connection */
void alarmHandler(int signal_number)
{
  if (nClients == 0)
  {
    nNoConnections++;
    // It's been 10 seconds - determine how many 10 second intervals its been without a connection
    printf("[SERVER WATCH-DOG] : It's been %d interval(s) without any client connections or chatter ...\n", nNoConnections);
  }
  else
  {
    // reset the number of times we've checked with no client connections
    nNoConnections = 0;
  }

  // reactivate signal handler for next time ...
  if ((nNoConnections == SERVER_INTERVAL_COUNT) && (nClients == 0))
  {
    printf("[SERVER WATCH-DOG] : Its been 30 seconds of inactivity ... I'M LEAVING !\n");
    exit(-1);
  }
  signal(signal_number, alarmHandler);
  alarm(SERVER_TIMEOUT_INTERVAL_TIME); // reset alarm
}

int main(void)
{
  int server_socket = 0;
  int client_socket = 0;
  int client_len = 0;
  int i = 0;

  struct sockaddr_in client_addr, server_addr;

  // zero out struct
  for (i = 0; i < MAX_NUM_CLIENTS; i++)
  {
    connectedClients[i].clientSocket = 0;
    memset(connectedClients[i].clientIPAddress, 0, MAX_IP_ADDR_LENGTH);
  }

  pthread_t clientHandler;
  pthread_attr_t attr;
  pthread_attr_init(&attr);

  /*
   * install a signal handler for SIGCHILD (sent when the child terminates)
   */
  signal(SIGALRM, alarmHandler);
  alarm(10);
  fflush(stdout);

  /*
   * obtain a socket for the server
   */
  fflush(stdout);
  if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    printf("Error! Server socket.getting failed.\n");
    return EXIT_WITH_ERROR;
  }

  /*
   * initialize our server address info for binding purposes
   */
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  server_addr.sin_port = htons(PORT);

  fflush(stdout);

  if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
  {
    printf("Error! Binding of server socket failed.\n");
    close(server_socket);
    return EXIT_WITH_ERROR;
  }

  /*
   * start listening on the socket
   */
  fflush(stdout);
  if (listen(server_socket, 5) < 0)
  {
    printf("Error! Listen on socket failed.\n");
    close(server_socket);
    return EXIT_WITH_ERROR;
  }

  while (1)
  {
    printf("Waiting for clients...\n");
    fflush(stdout);

    /*
     * accept a packet from the client1
     */
    client_len = sizeof(client_addr);
    if ((client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_len)) < 0)
    {
      printf("Error! Failed to accept packet from client\n");
      close(server_socket);
      return EXIT_WITH_ERROR;
    }

    printf("client address:  %s\n", inet_ntoa(client_addr.sin_addr));

    printf("Client is here(%d)\n", client_socket);
    fflush(stdout);
    nClients++;

    // store client's socket and IP to keep track of active clients
    for (i = 0; i < MAX_NUM_CLIENTS; i++)
    {
      if (connectedClients[i].clientSocket == 0)
      {
        connectedClients[i].clientSocket = client_socket;
        strcpy(connectedClients[i].clientIPAddress, inet_ntoa(client_addr.sin_addr));
        printf("Connected client socket %d, ip %s\n", connectedClients[i].clientSocket, connectedClients[i].clientIPAddress);
        break;
      }
    }

    //create thread
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    pthread_create(&clientServiceThreads[nClients], &attr, handleClient, (void *)&client_socket);
  }

  printf("All the clients left! Server shutting down.\n");
  fflush(stdout);
  close(server_socket);

  return EXIT_OK;
}

/*
* FUNCTION 		: handleClient
* DESCRIPTION 	: handles a chat client, reading a message on the given socket
                    and sharing the message to the other clients. The received
                    message must be of a specific format to be considered valid
* PARAMETERS 	: void *socketToHandle
                    : pointer to the socket (int) which will be read and written to
* RETURNS 		: void
*/
void *handleClient(void *socketToHandle)
{
  char *response = NULL;

  printf("New thread clients: %d\n", nClients);
  // used for accepting incoming command and also holding the command's response
  char buffer[BUFSIZ];
  int client_socket = *((int *)socketToHandle);
  int bytesRead = 0;
  int i = 0;

  while (1)
  {
    // allocate memory for incoming message
    char **splitMessage = (char **)calloc(NUM_MESSAGE_ELEMENTS, sizeof(char *));

    for (i = 0; i < NUM_MESSAGE_ELEMENTS; i++)
    {
      splitMessage[i] = (char *)calloc(BUFSIZ, sizeof(char));
    }

    //get client message
    bytesRead = read(client_socket, buffer, 64);
    printf("read %d bytes\n", bytesRead);
    if (bytesRead <= 0)
    {
      printf("Client disconnected!\n");
      //remove client from connectedClients (client logged off)
      removeClientSocket(client_socket);
      nClients--;
      break;
    }
    else
    {
      printf("message received: %s\n", buffer);
    }

    if (!isMessageValid(buffer))
    {
      printf("Error! Invalid message structure.\n");
    }
    else
    {
      parseClientMessage(buffer, splitMessage);

      if (strcmp(splitMessage[INDEX_MESSAGE], CLIENT_EXIT_MESSAGE) == 0)
      {
        removeClientSocket(client_socket);
        nClients--;
        break;
      }

      // since malloc is used in makeMessage, need to free the response
      response = makeMessage(getClientIP(client_socket), splitMessage[INDEX_CLIENT_NAME], splitMessage[INDEX_MESSAGE]);

      for (i = 0; i < MAX_NUM_CLIENTS; i++)
      {
        if (connectedClients[i].clientSocket != 0)
        {
          printf("sent to socket %d\n", connectedClients[i].clientSocket);
          int written = write(connectedClients[i].clientSocket, response, strlen(response));
          printf("written: %d\n", written);
        }
      }

      // free response buffer
      free(response);
    }

    // clear out the receiving buffer
    memset(buffer, 0, BUFSIZ);

    // free parsed message memory
    for (i = 0; i < NUM_MESSAGE_ELEMENTS; i++)
    {
      free(splitMessage[i]);
    }

    free(splitMessage);
  }
}

/*
* FUNCTION 		: parseClientMessage
* DESCRIPTION 	: parses a given message into an array of strings (split into usable indexes)
* PARAMETERS 	: char *fromClient
                    : the message received from the client (unformatted)
                char **splitMessage
                    : message elements are split into indexes of the array of strings  
* RETURNS 		: void
*/
void parseClientMessage(char *fromClient, char **splitMessage)
{

  int clientNameEnd = 0;
  int messageEnd = 0;

  // locate end of port in message (via delimiter)
  clientNameEnd = strlen(fromClient) - strlen(strstr(fromClient, NAME_DELIM));

  // copy to port string
  memcpy(splitMessage[INDEX_CLIENT_NAME], fromClient, clientNameEnd);
  splitMessage[INDEX_CLIENT_NAME][clientNameEnd] = NULL_TERM;

  // the message is the last element in the string
  // no delimiter!
  messageEnd = strlen(fromClient);

  // copy to message string, ignoring all previous elements
  memcpy(splitMessage[INDEX_MESSAGE], fromClient + clientNameEnd + strlen(NAME_DELIM), messageEnd);
  splitMessage[INDEX_MESSAGE][messageEnd] = NULL_TERM;
}

/*
* FUNCTION 		: makeMessage
* DESCRIPTION 	: builds a message from given parameters, which will be 
                    sent back to the client. The returned string needs
                    to be freed after use.
* PARAMETERS 	: char *clientIP
                    : the message author's IP address.
                char *clientName
                    : the message author's username.
                char *message
                    : the contents of the message.
* RETURNS 		: char*
                    : returns the formatted  string which will be 
                    sent to the client (need to free!)
*/
char *makeMessage(char *clientIP, char *clientName, char *message)
{
  int retStringLength = (strlen(clientIP) + strlen(clientName) + strlen(message) + strlen(IP_DELIM) + strlen(NAME_DELIM) + 1);
  char *retString = (char *)malloc(retStringLength * sizeof(char));

  // clear retString buffer
  memset(retString, 0, retStringLength);

  strcat(retString, clientIP);
  strcat(retString, IP_DELIM);
  strcat(retString, clientName);
  strcat(retString, NAME_DELIM);
  strcat(retString, message);

  printf("message to send: %s, length %d\n", retString, strlen(retString));

  return retString;
}

/*
* FUNCTION 		: isMessageValid
* DESCRIPTION 	: checks to see if the message received from a client is of
                    the valid format. (right number of delimiters)
* PARAMETERS 	: char *fromClient
                    : the message received from the client 
* RETURNS 		: bool
                    : true  : the message is valid
                      false : the message is invalid
*/
bool isMessageValid(char *fromClient)
{
  int i = 0;
  int elementCount = 0;

  // iterate through the string, checking that the string has the right delimiter
  while (fromClient[i] != NULL_TERM)
  {
    if (fromClient[i] == NAME_DELIM[0])
    {
      elementCount++;
    }
    i++;
  }

  printf("elements: %d\n", elementCount);

  if (elementCount == NUM_MESSAGE_ELEMENTS - 1)
  {
    return true;
  }

  return false;
}

/*
* FUNCTION 		: removeClientSocket
* DESCRIPTION 	: removes a given client socket from the array of
                    active clients.
* PARAMETERS 	: int socketToRemove
                  : the socket value which will be located and removed
* RETURNS 		: void
*/
void removeClientSocket(int socketToRemove)
{
  for (int i = 0; i < MAX_NUM_CLIENTS; i++)
  {
    if (connectedClients[i].clientSocket == socketToRemove)
    {
      printf("removed %d\n", connectedClients[i].clientSocket);
      connectedClients[i].clientSocket = 0;
      memset(connectedClients[i].clientIPAddress, 0, MAX_IP_ADDR_LENGTH);
      break;
    }
  }
}

char *getClientIP(int clientSocket)
{
  int i = 0;

  for (i = 0; i < MAX_NUM_CLIENTS; i++)
  {
    if (connectedClients[i].clientSocket == clientSocket)
    {
      return connectedClients[i].clientIPAddress;
    }
  }

  return NULL;
}
