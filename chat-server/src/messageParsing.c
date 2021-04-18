/*
 *  FILE          : messageParsing.c
 *  PROJECT       : SENG2030-21W-Sec1-System Programming - Assignment #4
 *  PROGRAMMER    : Andrey Takhtamirov, Alex Braverman
 *  FIRST VERSION : April 16, 2021 
 *  DESCRIPTION   : 
 *			This file contains the logic for functions used to verify incoming
 *          messages, functions used to parse received messages and assemble 
 *          outgoing messages.
 *	
*/
#include "../inc/chatServer.h"

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

    if(retString == NULL)
    {
        printf("Error! Memory for return string could not be allocated!\n");
        return NULL;
    }

    // clear retString buffer
    memset(retString, 0, retStringLength);

    strcat(retString, clientIP);
    strcat(retString, IP_DELIM);
    strcat(retString, clientName);
    strcat(retString, NAME_DELIM);
    strcat(retString, message);

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

    if (elementCount == NUM_MESSAGE_ELEMENTS - 1)
    {
        return true;
    }

    return false;
}