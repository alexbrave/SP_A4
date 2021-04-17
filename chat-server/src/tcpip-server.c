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

#define PORT_DELIM ";"
#define IP_DELIM "!"
#define NAME_DELIM "?"
#define NULL_TERM '\0'

#define INDEX_PORT 0
#define INDEX_IP 1
#define INDEX_CLIENT_NAME 2
#define INDEX_MESSAGE 3
#define NUM_MESSAGE_ELEMENTS 4

// time out interval time (3 intervals)
#define SERVER_TIMEOUT_INTERVAL_TIME 100
#define SERVER_INTERVAL_COUNT 3

void *handleClient(void *socket_to_handle);
void parseClientMessage(char *fromClient, char **splitMessage);
char *makeMessage(char *clientIP, char *clientName, char *message);
bool isMessageValid(char *fromClient);
void removeClientSocket(int socketToRemove);

// keep track of the sockets of connected clients
int clientSockets[MAX_NUM_CLIENTS] = {0};

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

  for (i = 0; i < sizeof(clientSockets); i++)
  {
    clientSockets[i] = 0;
  }

  pthread_t clientHandler;
  pthread_attr_t attr;
  pthread_attr_init(&attr);

  /*
   * install a signal handler for SIGCHILD (sent when the child terminates)
   */
  printf("Installing signal handler for WATCHDOG ...\n");
  signal(SIGALRM, alarmHandler);
  alarm(10);
  fflush(stdout);

  /*
   * obtain a socket for the server
   */
  printf("Obtaining STREAM Socket ...\n");
  fflush(stdout);
  if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    printf("Error! Server socket.getting failed.\n");
    return 1;
  }

  /*
   * initialize our server address info for binding purposes
   */
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  server_addr.sin_port = htons(PORT);

  printf("Binding socket to server address ...\n");
  fflush(stdout);

  if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
  {
    printf("Error! Binding of server socket failed.\n");
    close(server_socket);
    return 2;
  }

  /*
   * start listening on the socket
   */
  printf("Listening for incoming connections ...\n");
  fflush(stdout);
  if (listen(server_socket, 5) < 0)
  {
    printf("Error! Listen on socket failed.\n");
    close(server_socket);
    return 3;
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
      return 4;
    }
    printf("Client is here(%d)\n", client_socket);
    fflush(stdout);
    nClients++;

    // NOTE: store client's socket in array to keep track of active clients
    for (i = 0; i < sizeof(clientSockets) / sizeof(int); i++)
    {
      if (clientSockets[i] == 0)
      {
        clientSockets[i] = client_socket;
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

  return 0;
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
    bytesRead = read(client_socket, buffer, BUFSIZ);
    printf("read %d bytes\n", bytesRead);
    if (bytesRead <= 0)
    {
      printf("Client disconnected!\n");
      //remove client from clientSockets array (client logged off)
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
      write(client_socket, "INVALID MESSAGE", strlen("INVALID MESSAGE"));
      break;
    }
    else
    {
      parseClientMessage(buffer, splitMessage);

      printf("PORT:\t\t %s\n", splitMessage[INDEX_PORT]);
      printf("IP:\t\t\t %s\n", splitMessage[INDEX_IP]);
      printf("CLIENT NAME: %s\n", splitMessage[INDEX_CLIENT_NAME]);
      printf("MESSAGE:\t %s\n", splitMessage[INDEX_MESSAGE]);

      // since malloc is used in makeMessage, need to free the response
      response = makeMessage(splitMessage[INDEX_IP], splitMessage[INDEX_CLIENT_NAME], splitMessage[INDEX_MESSAGE]);

      for (i = 0; i < sizeof(clientSockets) / sizeof(int); i++)
      {
        if (clientSockets[i] != 0)
        {
          printf("sent to socket %d\n", clientSockets[i]);
          write(clientSockets[i], response, strlen(response));
        }
      }
    }

    // free response
    free(response);

    /* Clear out the Buffer */
    memset(buffer, 0, BUFSIZ);

    // free incoming message memory
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

  int portEnd = 0;
  int ipEnd = 0;
  int clientNameEnd = 0;
  int messageEnd = 0;

  // locate end of port in message (via delimiter)
  portEnd = strlen(fromClient) - strlen(strstr(fromClient, PORT_DELIM));

  // copy to port string
  memcpy(splitMessage[INDEX_PORT], fromClient, portEnd);
  splitMessage[INDEX_PORT][portEnd] = NULL_TERM;

  // locate end of IP in message (minus previous contents)
  ipEnd = strlen(strstr(fromClient, PORT_DELIM)) - strlen(strstr(fromClient, IP_DELIM)) - strlen(IP_DELIM);

  // copy into IP string
  memcpy(splitMessage[INDEX_IP], fromClient + portEnd + strlen(PORT_DELIM), ipEnd);
  splitMessage[INDEX_IP][ipEnd] = NULL_TERM;

  // locate end of client name in message (minus previous contents)
  clientNameEnd = strlen(strstr(fromClient, IP_DELIM)) - strlen(strstr(fromClient, NAME_DELIM)) - strlen(NAME_DELIM);

  memcpy(splitMessage[INDEX_CLIENT_NAME], fromClient + portEnd + ipEnd + strlen(IP_DELIM) + strlen(PORT_DELIM), clientNameEnd);
  splitMessage[INDEX_CLIENT_NAME][clientNameEnd] = NULL_TERM;

  // the message is the last element in the string
  // no delimiter!
  messageEnd = strlen(fromClient);

  // copy to message string, ignoring all previous elements
  memcpy(splitMessage[INDEX_MESSAGE], fromClient + portEnd + ipEnd + clientNameEnd + strlen(IP_DELIM) + strlen(PORT_DELIM) + strlen(NAME_DELIM), messageEnd);
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
  char *retString = (char *)malloc((strlen(clientIP) + strlen(clientName) + strlen(message) + strlen(IP_DELIM) + strlen(NAME_DELIM) + 1) * sizeof(char));
  // check null 

  strcat(retString, clientIP);
  strcat(retString, IP_DELIM);
  strcat(retString, clientName);
  strcat(retString, NAME_DELIM);
  strcat(retString, message);

  printf("message to send: %s\n", retString);

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
    if (fromClient[i] == PORT_DELIM[0] || fromClient[i] == IP_DELIM[0] || fromClient[i] == NAME_DELIM[0])
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
  for (int i = 0; i < sizeof(clientSockets) / sizeof(int); i++)
  {
    if (clientSockets[i] == socketToRemove)
    {
      printf("removed %d\n", clientSockets[i]);
      clientSockets[i] = 0;
    }
  }
}

bool allClientsGone(void)
{
  int counter = 0;

  // scan the array of connected clients and accumulate counter
  for (int i = 0; i < sizeof(clientSockets) / sizeof(int); i++)
  {
    counter += clientSockets[i];
  }

  // counter is 0 if there are no clients left
  if (counter == 0)
  {
    return true;
  }
  else
  {
    return false;
  }
}