/*
 *  FILE          : tcip-server.c
 *  PROJECT       : SENG2030-21W-Sec1-System Programming - Assignment #4
 *  PROGRAMMER    : Andrey Takhtamirov, Alex Braverman
 *  FIRST VERSION : April 16, 2021 
 *  DESCRIPTION   : 
 *			This file contains the main logic for the tcip-server application.
 *        this program was built on Sean's "sockets-chat Server" program given in class.
 *        tcip-server is a chat server to let 2-10 clients communicate with each other
 *        over tcp/ip.
 *	
*/
#include "../inc/chatServer.h"

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

    fflush(stdout);
    nClients++;

    // store client's socket and IP to keep track of active clients
    for (i = 0; i < MAX_NUM_CLIENTS; i++)
    {
      if (connectedClients[i].clientSocket == 0)
      {
        connectedClients[i].clientSocket = client_socket;
        strcpy(connectedClients[i].clientIPAddress, inet_ntoa(client_addr.sin_addr));
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
