/*
 *  FILE          : clientManagement.c
 *  PROJECT       : SENG2030-21W-Sec1-System Programming - Assignment #4
 *  PROGRAMMER    : Andrey Takhtamirov, Alex Braverman
 *  FIRST VERSION : April 16, 2021 
 *  DESCRIPTION   : 
 *			This file contains the logic for functions used to manage
 *              clients in the connectedClients array.
 *	
*/
#include "../inc/chatServer.h"

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
    int i = 0;
    for (i = 0; i < MAX_NUM_CLIENTS; i++)
    {
        if (connectedClients[i].clientSocket == socketToRemove)
        {
            connectedClients[i].clientSocket = 0;
            memset(connectedClients[i].clientIPAddress, 0, MAX_IP_ADDR_LENGTH);
            break;
        }
    }
}

/*
* FUNCTION 		: getClientIP
* DESCRIPTION 	: returns the IP of a client with socket /clientSocket/
* PARAMETERS 	: int clientSocket
                  : the socket value which will be located
* RETURNS 		: char*
                  : the string IP address of a requested client
*/
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


/*
* FUNCTION 		: allClientsGone
* DESCRIPTION 	: checks if there are any clients left connected
* PARAMETERS 	: void 
* RETURNS 		:   true  : all clients have been disconnected
*                   false : at least 1 client still connected
*/
bool allClientsGone(void)
{
  int counter = 0;
  // scan the array of connected clients and accumulate counter
  for (int i = 0; i < MAX_NUM_CLIENTS; i++)
  {
    counter += connectedClients[i].clientSocket;
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
