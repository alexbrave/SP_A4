/*
 *  FILE          : socketFunctions.c
 *  PROJECT       : SENG2030-21W-Sec1-System Programming - Assignment #4
 *  PROGRAMMER    : Andrey Takhtamirov, Alex Braverman
 *  FIRST VERSION : April 17, 2020 
 *  DESCRIPTION   : 
 *			
 *	
*/

#include "../inc/chatClient.h"

// ADD FUNCTION HEADER
int setUpSocket(int* serverSocket, struct hostent* host)
{
  ///////////////////
  // Set up socket //
  ///////////////////

  int                len;
  int                done;
  int                whichClient;
  char               buffer[BUFSIZ] = { 0 };
  struct sockaddr_in serverAddress;
  struct sockaddr_in clientAddress;

  struct hostent*    clientHOSTENT; // NOT SURE WHAT THIS IS POSSIBLY REMOVE LATER


  // initialize struct to get a socket to host the connection to server
  memset (&serverAddress, 0, sizeof (serverAddress));
  serverAddress.sin_family = AF_INET;
  memcpy (&serverAddress.sin_addr, host->h_addr, host->h_length);
  serverAddress.sin_port = htons(PORT);


  // initialize struct to bind client socket to specific IP address and Port
  clientHOSTENT = gethostbyname("172.26.177.173");
  memset (&clientAddress, 0, sizeof (clientAddress));
  clientAddress.sin_family = AF_INET;
  memcpy (&clientAddress.sin_addr, clientHOSTENT->h_addr, clientHOSTENT->h_length);
  clientAddress.sin_port = htons(PORT);
  

  // get a socket for communications
  if ((*serverSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
  {
    return CANT_GET_SOCKET;
  }

  // bind socket to the ip address and port that we specified for the client
  // bind (*serverSocket, (struct sockaddr *)&clientAddress, sizeof (clientAddress));

  // attempt a connection to server
  if (connect (*serverSocket, (struct sockaddr *)&serverAddress,sizeof (serverAddress)) < 0) 
  {
    close (*serverSocket);
    return CANT_CONNECT_TO_SERVER;
  }

  // make socket non-blocking so we don't get stuck in a read operation
  // THIS MAY OR MAY NOT BE THE SOURCE OF THE PROBLEM!!
  fcntl(*serverSocket, F_SETFL, O_NONBLOCK);
}