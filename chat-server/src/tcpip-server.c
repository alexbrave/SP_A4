/*
 * tcpip-server.c
 *
 * This is a sample internet server application that will respond
 * to requests on port 5000
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
#define IP_ADDR 2887388853
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

void *handleClient(void *socket_to_handle);
void parseClientMessage(char *fromClient, char **splitMessage);
char *makeMessage(char *clientIP, char *clientName, char *message);
bool isMessageValid(char *fromClient);

int clientSockets[MAX_NUM_CLIENTS];

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

  if ((nNoConnections == 3) && (nClients == 0))
  {
    printf("[SERVER WATCH-DOG] : Its been 30 seconds of inactivity ... I'M LEAVING !\n");
    exit(-1);
  }
  signal(signal_number, alarmHandler);
  alarm(10); // reset alarm
}

int main(void)
{
  int server_socket, client_socket;
  int client_len;
  int client1Gone, client2Gone;
  struct sockaddr_in client_addr, server_addr;
  int len, i;
  FILE *p;

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
  printf("[SERVER] : Installing signal handler for WATCHDOG ...\n");
  signal(SIGALRM, alarmHandler);
  alarm(10);
  fflush(stdout);

  /*
   * obtain a socket for the server
   */
  printf("[SERVER] : Obtaining STREAM Socket ...\n");
  fflush(stdout);
  if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    printf("[SERVER] : Server Socket.getting - FAILED\n");
    return 1;
  }

  /*
   * initialize our server address info for binding purposes
   */
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  server_addr.sin_port = htons(PORT);

  char *some_addr = inet_ntoa(server_addr.sin_addr); // return the IP
  printf("%s\n", some_addr);

  printf("[SERVER] : Binding socket to server address ...\n");
  fflush(stdout);

  if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
  {
    printf("[SERVER] : Binding of Server Socket - FAILED\n");
    close(server_socket);
    return 2;
  }

  /*
   * start listening on the socket
   */
  printf("[SERVER] : Begin listening on socket for incoming connections ...\n");
  fflush(stdout);
  if (listen(server_socket, 5) < 0)
  {
    printf("[SERVER] : Listen on Socket - FAILED.\n");
    close(server_socket);
    return 3;
  }

  /*
   * this is a really crappy CHAT program using the socket API
   *   -- basically the server waits for 2 client connections and
   *      then does a back and forth chatting exchange.  In reality
   *      to manage the conversation better, some sharedMemory scheme, etc
   *      would need to be implemented between the client child processes
   */
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
      printf("[SERVER] : Accept Packet from Client - FAILED\n");
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
        printf("socket %d was %d\n", i, clientSockets[i]);
        clientSockets[i] = client_socket;
        printf("socket %d is now %d\n", i, clientSockets[i]);
        break;
      }
    }

    //create thread
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    pthread_create(&clientServiceThreads[nClients], &attr, handleClient, (void *)&client_socket);
  }

  printf("[SERVER] : Everyone is gone ... I'm leaving as well ...\n");
  fflush(stdout);
  close(server_socket);

  return 0;
}

void *handleClient(void *socket_to_handle)
{
  printf("New thread\n");
  // used for accepting incoming command and also holding the command's response
  char buffer[BUFSIZ];
  int client_socket = *((int *)socket_to_handle);
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
    if (bytesRead == 0)
    {
      printf("No bytes read!\n");
      break;
    }
    else
    {
      printf("message recieved: %s\n", buffer);
    }

    if (!isMessageValid(buffer))
    {
      printf("Error! Invalid message structure.\n");
      break;
    }

    parseClientMessage(buffer, splitMessage);

    printf("PORT:\t\t %s\n", splitMessage[INDEX_PORT]);
    printf("IP:\t\t\t %s\n", splitMessage[INDEX_IP]);
    printf("CLIENT NAME: %s\n", splitMessage[INDEX_CLIENT_NAME]);
    printf("MESSAGE:\t %s\n", splitMessage[INDEX_MESSAGE]);

    //write(client_socket, "I got your message!", BUFSIZ);

    char *response = makeMessage(splitMessage[INDEX_IP], splitMessage[INDEX_CLIENT_NAME], splitMessage[INDEX_MESSAGE]);

    for (i = 0; i < sizeof(clientSockets) / sizeof(int); i++)
    {
      if (clientSockets[i] != 0)
      {
        printf("socket %d is %d\n", i, clientSockets[i]);
        write(client_socket, response, strlen(response));
      }
    }

    free(response);

    /* Clear out the Buffer */
    memset(buffer, 0, BUFSIZ);

    // free memory!
    for (i = 0; i < NUM_MESSAGE_ELEMENTS; i++)
    {
      free(splitMessage[i]);
    }

    free(splitMessage);
  }
}

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

char *makeMessage(char *clientIP, char *clientName, char *message)
{
  char *retString = (char *)malloc((strlen(clientName) + strlen(message) + 1) * sizeof(char));
  // strcat(retString, clientIP);
  // strcat(retString, IP_DELIM);
  strcat(retString, clientName);
  strcat(retString, NAME_DELIM);
  strcat(retString, message);

  printf("message to send: %s\n", retString);

  return retString;
}

bool isMessageValid(char *fromClient)
{
  int i = 0;
  int elementCount = 0;

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
