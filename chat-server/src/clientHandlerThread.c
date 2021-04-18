/*
 *  FILE          : clientHandlerThread.c
 *  PROJECT       : SENG2030-21W-Sec1-System Programming - Assignment #4
 *  PROGRAMMER    : Andrey Takhtamirov, Alex Braverman
 *  FIRST VERSION : April 16, 2021 
 *  DESCRIPTION   : 
 *			This file contains the logic for the handleClient thread.
 *              this thread takes a socket and reads/writes from/to it,
 *              forwarding read messages to other clients. When a client is
 *              closed with ">>bye<<" the client is removed from the 
 *              connectedClients array and the thread terminates.
 *	
*/
#include "../inc/chatServer.h"

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

    // used for accepting incoming command and also holding the command's response
    char buffer[BUFSIZ] = {0};
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
        if (bytesRead <= 0)
        {
            //remove client from connectedClients (client logged off)
            removeClientSocket(client_socket);
            nClients--;
            break;
        }

        // ignore message if invalid (wrong/missing delimiters)
        if (isMessageValid(buffer))
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
                    write(connectedClients[i].clientSocket, response, strlen(response));
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
