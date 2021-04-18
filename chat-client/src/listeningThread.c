/*
 *  FILE          : listeningThread.c
 *  PROJECT       : SENG2030-21W-Sec1-System Programming - Assignment #4
 *  PROGRAMMER    : Andrey Takhtamirov, Alex Braverman
 *  FIRST VERSION : March 26, 2020 
 *  DESCRIPTION   : 
 *			
 *	
*/

#include "../inc/chatClient.h"



/*
* FUNCTION    : listeningThreadFunc
* DESCRIPTION : the listening thread that will get messages recieved from the server, and will get the semaphore
*               to control the cursor and write the messages to the screen.
* PARAMETERS  : void* arg: (ThreadArgs* threadArgs) : contains all the arguments needed for the threads to share
* RETURNS : void* : no return for this functions 
*/
void* listeningThreadFunc(void* arg)
{
    // Get data from the thread arguments, such as the window pointer, exit flag, and server socket
    ThreadArgs* threadArgs = (ThreadArgs*) arg;
    WINDOW* win = threadArgs->win;
    bool* exitFlag = threadArgs->exitFlag;
    int serverSocket = *(threadArgs->serverSocket);
    int readResult = 0;


    // Here we initialize data that we'll get from the incoming message
    char otherClientIP[BUFSIZ] = { 0 };
    char otherClientName[MAX_LEN_OF_USR_NAME + 1] = { 0 };
    char otherClientMsg[BUFSIZ] = { 0 };


    void* voidPointer = NULL; 
    int savedCursorX = 0;   // Cursor coordinates so that we can save the cursors location
    int savedCursorY = 0;
    time_t rawtime;
    Last10MsgLines last10MsgLines;
    receivedMSG latestMsg;

    char incomingMsgBuffer[BUFSIZ] = { 0 };


    // Set messages list struct to all 0
    memset(&last10MsgLines, 0, sizeof(last10MsgLines));
    last10MsgLines.messagesAvailable = 0;



    while(true)
    {
        getOrCreateCursorSem();     // Get sem to use cursor

        if(*exitFlag == true) // First things first: check if the listening thread should exit
        {
            releaseCursorSem(); 
            break;
        }        
        else
        { 
            readResult = read(serverSocket, incomingMsgBuffer, BUFSIZ);
            if(readResult > OPERATION_SUCCESS && incomingMsgBuffer[0] != '\0') // Read data from socket
            {
                memset(&latestMsg, 0, sizeof(latestMsg));           // Clear our latest message holder struct

                // Parse string into custom msg struct
                if(parseIncomingMsg(&latestMsg, incomingMsgBuffer, threadArgs->userName) == OPERATION_SUCCESS)
                {
                    reorderLast10Msgs(&latestMsg, &last10MsgLines);             // Reorder messages
                }    
                
                memset(incomingMsgBuffer, 0, sizeof(incomingMsgBuffer));    // Reset incoming msg buffer to 0
            }

            if (last10MsgLines.messagesAvailable > 0) // Print all available messages
            {
                getyx(win, savedCursorY, savedCursorX); // Save the cursors position
                writeMessages(win, &last10MsgLines);    
                wmove(win, savedCursorY, savedCursorX); // Put cursor back
            }            
        }
        
        releaseCursorSem(); 

        // sleep(3);
        usleep(100);
    }

    return voidPointer;
}



/*
* FUNCTION    : parseIncomingMsg
* DESCRIPTION : parses the incoming message into a receivedMSG struct that will be added to the last 10 messages list
* PARAMETERS  : receivedMSG* newMsgStruct : struct to be populated with parsed message data
*               char* newMsgString : the string holding the raw message data
*               char* thisUsersName : the users name, which is used to determine if message is incoming or outgoing
* RETURNS : int : returns if parse was sucessful
*/
int parseIncomingMsg(receivedMSG* newMsgStruct, char* newMsgString, char* thisUsersName)
{
    ///////////////////////////////////////////////////
    // First parse data that came in with the string //
    ///////////////////////////////////////////////////
    int counter = 0;
    // [client's IP][exclamation mark]
    // [client name][question mark]
    // [message]
    int incomingMsgStringLen = strlen(newMsgString);

    // Do some sanity checking to make sure the incoming message is the right length
    if( incomingMsgStringLen < MAX_LEN_OF_IP_ADDR || incomingMsgStringLen > MAX_LEN_INCOMING_MSG)
    {
        return OPERATION_FAILED;
    }
    for(int i = 0; newMsgString[counter] != '|' && counter <=incomingMsgStringLen; i++)
    {
        newMsgStruct->recievedIPAddr[i] = newMsgString[counter];
        counter++;
    }

    if (counter >= incomingMsgStringLen) // this means we couldn't parse out what we needed
    {
        return OPERATION_FAILED;
    }

    counter++; // Skip the exclamation mark separating ip address and other clients name

    for(int i = 0; newMsgString[counter] != ';' && counter <=incomingMsgStringLen; i++)
    {
        newMsgStruct->usersName[i] = newMsgString[counter];
        counter++;
    }

    if (counter >= incomingMsgStringLen) // this means we couldn't parse out what we needed
    {
        return OPERATION_FAILED;
    }

    counter++; // Skip the question mark separating other clients name and message

    for(int i = 0; newMsgString[counter] != '\0' && counter <=incomingMsgStringLen; i++)
    {
        newMsgStruct->messagePayload[i] = newMsgString[counter];
        counter++;
    }
    

    //////////////////////////////////////////////
    // Then generate and assign non-string data //
    //////////////////////////////////////////////
    time_t currentTime = 0;
    bool isThisOurMessage = false;
    int messageID = 0;

    if(strncmp(newMsgStruct->usersName, thisUsersName, MAX_LEN_OF_USR_NAME) == 0)
    {
        isThisOurMessage = true;
    }

    time(&currentTime); // set the current time
    newMsgStruct->timeOnReception = currentTime;
    newMsgStruct->isThisOurMessage = isThisOurMessage;
    newMsgStruct->messageID = messageID;

    return OPERATION_SUCCESS;
}



/*
* FUNCTION    : reorderLast10Msgs
* DESCRIPTION : reorders message list to add a new message, and delete oldest message if necessary
* PARAMETERS  : receivedMSG* latestMsg : the message struct containing parsed data from message just recieved
*               Last10MsgLines* last10MsgLines : pointer to the struct holding last 10 messages
* RETURNS     : void : no return for this functions
*/
void reorderLast10Msgs(receivedMSG* latestMsg, Last10MsgLines* last10MsgLines)
{
    int counter = 0;
    if(last10MsgLines->messagesAvailable < TEN_MESSAGES) // We must determine if our list is full or not
    {                                                    
        counter = last10MsgLines->messagesAvailable - 1;
        last10MsgLines->messagesAvailable++;
    }
    else    // If list is full we'll start 1 from the end and push everything down
    {
        counter = TEN_MESSAGES - 2;
    }


    ///////////////////////////////////////////////////////////
    // Reorder current messages to make room for new message //
    ///////////////////////////////////////////////////////////
    for (int i = counter; i >= 0; i--) // We want to start with the second last element
    {                                           // and shift everything to the next spot in the list
        // Move message ID to next spot in line
        last10MsgLines->messages[i + 1].messageID = last10MsgLines->messages[i].messageID;

        // Move time on reception to next spot in line
        last10MsgLines->messages[i + 1].timeOnReception = last10MsgLines->messages[i].timeOnReception;

        // Set ip address at next spot in line to 0 and move IP address there
        memset(last10MsgLines->messages[i + 1].recievedIPAddr, 0, sizeof(last10MsgLines->messages[i + 1].recievedIPAddr));
        strcpy(last10MsgLines->messages[i + 1].recievedIPAddr, last10MsgLines->messages[i].recievedIPAddr);

        // Set message payload at next spot in line to 0 and move message payload there 
        memset(last10MsgLines->messages[i + 1].messagePayload, 0, sizeof(last10MsgLines->messages[i + 1].messagePayload));
        strcpy(last10MsgLines->messages[i + 1].messagePayload, last10MsgLines->messages[i].messagePayload);

        // Set users name at next spot in line to 0 and move users name there 
        memset(last10MsgLines->messages[i + 1].usersName, 0, sizeof(last10MsgLines->messages[i + 1].usersName));
        strcpy(last10MsgLines->messages[i + 1].usersName, last10MsgLines->messages[i].usersName);

        // Move the "our message" boolean flag to the next spot in line
        last10MsgLines->messages[i + 1].isThisOurMessage = last10MsgLines->messages[i].isThisOurMessage;
    }

    /////////////////////////////
    // Add new message to list //
    /////////////////////////////
    // Move message ID to next spot in line
    last10MsgLines->messages[0].messageID = latestMsg->messageID;

    // Move time on reception to next spot in line
    last10MsgLines->messages[0].timeOnReception = latestMsg->timeOnReception;

    // Set ip address at next spot in line to 0 and move IP address there
    memset(last10MsgLines->messages[0].recievedIPAddr, 0, sizeof(last10MsgLines->messages[0].recievedIPAddr));
    strcpy(last10MsgLines->messages[0].recievedIPAddr, latestMsg->recievedIPAddr);

    // Set message payload at next spot in line to 0 and move message payload there 
    memset(last10MsgLines->messages[0].messagePayload, 0, sizeof(last10MsgLines->messages[0].messagePayload));
    strcpy(last10MsgLines->messages[0].messagePayload, latestMsg->messagePayload);

    // Set users name at next spot in line to 0 and move users name there 
    memset(last10MsgLines->messages[0].usersName, 0, sizeof(last10MsgLines->messages[0].usersName));
    strcpy(last10MsgLines->messages[0].usersName, latestMsg->usersName);

    // Move the "our message" boolean flag to the next spot in line
    last10MsgLines->messages[0].isThisOurMessage = latestMsg->isThisOurMessage;
}




