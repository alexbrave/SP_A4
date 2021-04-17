/*
 *  FILE          : chatClient.h
 *  PROJECT       : SENG2030-21W-Sec1-System Programming - Assignment #4
 *  PROGRAMMER    : Andrey Takhtamirov, Alex Braverman
 *  FIRST VERSION : April 18, 2020 
 *  DESCRIPTION   : 
 *	        This file contains some constants and function prototypes for the
 *          chat client program.
*/

#ifndef CLIENT_H
#define CLIENT_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <sys/sem.h>
#include <ncurses.h>
#include <pthread.h>


#include <ctype.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>



// Constants used for setting up socket
#define PORT 5000



// Constants used with ncurses
#define INPUT_HEIGHT 2
#define LEFT 0
#define TOP 0
#define MAX_COL_RIGHTSIDE_PADDING 5
#define TEN_MESSAGES 10
#define MAX_LEN_OF_IP_ADDR 15
#define MAX_LEN_OF_MESSAGE 40
#define MAX_LEN_OF_USR_NAME 5
#define NULL_TERM 1
#define LEN_OF_QUIT_STRING 8
#define HALF_OF_MSGS_TITLE 4
#define THIRD_ROW 2
#define FOURTH_ROW 3
#define POS_FIRST_SPACE  16
#define POS_SECOND_SPACE 24
#define POS_THIRD_SPACE  27
#define POS_FOURTH_SPACE 68



// Function return codes
#define OPERATION_FAILED  -1
#define OPERATION_SUCCESS 0

// Flag parsing error codes
#define MISFORMATTED_FLAG 1

// Socket error codes
#define CANT_GET_SOCKET 2
#define CANT_CONNECT_TO_SERVER 3



// Represents 1 of the last 10 messages/lines received
typedef struct 
{
    int messageID; // This will be equal for two lines that come from the same message
    time_t timeOnReception;
    char recievedIPAddr[MAX_LEN_OF_IP_ADDR + NULL_TERM];
    char messagePayload[MAX_LEN_OF_MESSAGE + NULL_TERM];
    char usersName[MAX_LEN_OF_USR_NAME + NULL_TERM];
    bool isThisOurMessage;
    /* data */
} receivedMSG;

// Represents last 10 messages received
typedef struct
{
    int messagesAvailable; // This indicates how many messages are available for UI thread to display
    receivedMSG messages[TEN_MESSAGES];
} Last10MsgLines;


typedef struct
{
    bool*   exitFlag;
    int*    serverSocket;
    WINDOW* win;
    char*   userName;
    char*   serverName;
    char*   userIP;
} ThreadArgs;





// Function prototypes
WINDOW *create_newwin(int height, int width, int starty, int startx);
void input_win(ThreadArgs* threadArgs);
void blankWin(WINDOW *win);
// void display_win(WINDOW *win, char *word);
void printChevron(WINDOW *win, int currentX, int currentY);
void writeMessageBanner(WINDOW* win);
int writeMessages(WINDOW* win, Last10MsgLines* last10msgs);
void* listeningThreadFunc(void* arg);
void writeMessageBanner(WINDOW* win);
void printChar(WINDOW* win, int row, int* col, char charToPrint);
void input_win(ThreadArgs* threadArgs);


void parseIncomingMsg(receivedMSG* newMsgStruct, char* newMsgString);
void reorderLast10Msgs(receivedMSG* latestMsg, Last10MsgLines* last10MsgLines);





// Cursor semaphore prototypes
int releaseCursorSem(void);
int getOrCreateCursorSem(void);
void setUpWindow(WINDOW** inputWindow);


// Socket functions
int setUpSocket(int* serverSocket, struct hostent* host);







// Log file path
// #define LOG_FILE_PATH "/tmp/dataCreator.log"
// #define MAX_LOG_LEN 100

// Log Message Entries
// #define LOG_MSG_QUE_GONE 1 // this is for the main loop if DC discovers that msg queue is gone
// #define LOG_GET_MSG_KEY_FAILED 2
// #define LOG_MSG_QUE_NOT_THERE 3 // before the main loop if the DC can't get to message queue
// #define LOG_MACH_SHUT_DOWN 4

// Semaphore Constants
#define SEM_FTOK_ID 123456789
#define NUM_OF_SEMS 1
#define CHECK_SEM_EXISTS 0
#define SEM_INITAL_VALUE 1
#define NUM_SOP_STRUCTS 1
#define CURRENT_DIRECTORY "."

#endif
