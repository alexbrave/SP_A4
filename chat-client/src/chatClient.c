/*
 *  FILE          : DC.c
 *  PROJECT       : SENG2030-21W-Sec1-System Programming - Assignment #3
 *  PROGRAMMER    : Andrey Takhtamirov, Alex Braverman
 *  FIRST VERSION : March 26, 2020 
 *  DESCRIPTION   : 
 *			This file contains the entry point, main() for the Data Creator program.
 *          The purpose of this program is to create generate messages and send them 
 *          on a message queue to be received/read by another program, the Data Reader.
 *	
*/

#include "../inc/chatClient.h"


int parseArguments(char* parsedUserName, char* parsedServerName, char* args[]);


int main(int argc, char *argv[])
{

  //////////////////
  // Set-up Logic //
  //////////////////


  WINDOW* inputWindow                     = NULL;
  pthread_t listeningThreadID;
  int serverSocket                        = 0;
  bool exitFlag                           = false;
  ThreadArgs threadArgs;
  struct hostent* host                    = NULL;
  int socketSetupResult                   = 0;
  char buf[BUFSIZ]                        = { 0 };
  char userName[MAX_LEN_OF_USR_NAME + 1]  = { 0 };
  char serverName[BUFSIZ]                 = { 0 };
  char userIP[BUFSIZ]                     = { 0 };


  char testBuffer[]                       = "This is a test";
  char formattedMsg[BUFSIZ]               = { 0 };
  receivedMSG message;
  Last10MsgLines last10msgs;

  
  /////////////////////
  // Check arguments // 
  /////////////////////
  if(argc != 3)
  {
    printf("Sorry, looks like there are too many or too few arguments!\n");
    printf("Please pass \"-user[your user name]\" and \"-server[the server name]\".\n");
    return 0;
  }

  if(parseArguments(userName, serverName, argv) == MISFORMATTED_FLAG)
  {
    printf("Sorry, it looks like there's something wrong with the formatting for one of the flags!\n");
    printf("Please pass \"-user[your user name]\" and \"-server[the server name]\".\n");
    return 0;
  }

  // Try to get the hostent for the server
  host = gethostbyname(serverName);
  if(host == NULL)
  {
    printf("Sorry, it looks like that server name is not resolevable!\n");
    printf("Please pass \"-user[your user name]\" and \"-server[the server name]\".\n");
    return 0;
  }



  // Set up socket
  socketSetupResult = setUpSocket(&serverSocket, host);


  // write(serverSocket, testBuffer, sizeof(testBuffer));
  // memset(&testBuffer, 0, sizeof(testBuffer));
  // read(serverSocket, testBuffer, sizeof(testBuffer));


  // printf("%s", testBuffer);

  if(socketSetupResult == CANT_GET_SOCKET)
  {
    printf("Socket error! Could not get a socket.\n");
    return 0;
  }
  else if(socketSetupResult == CANT_CONNECT_TO_SERVER)
  {
    printf("Couldn't connect to server! Exiting.\n");
    return 0;
  }


  // Set up ncurses window
  setUpWindow(&inputWindow);


  // Set up listening thread arguments
  threadArgs.exitFlag = &exitFlag;
  threadArgs.serverSocket = &serverSocket;
  threadArgs.win = inputWindow;
  threadArgs.userName = userName;


  ////////////////////////////
  // Start listening thread //
  ////////////////////////////

  //that will also write incoming messages to the screen
  if (pthread_create(&listeningThreadID, NULL, listeningThreadFunc, (void *)&threadArgs) != 0) 
  {
    printf ("Arghh !! I cannot start listening thread\n");
    exit (2);
  }


  // Run main logic on loop
  while(exitFlag == false)
  {
    input_win(&threadArgs);
  }

  close(serverSocket);

  delwin(inputWindow);
  endwin();
  // CLEAN UP RESOURCES

  return 0;
    
}




int parseArguments(char* parsedUserName, char* parsedServerName, char* args[])
{
  ////////////////////////////////////
  // Set up variables and constants //
  ////////////////////////////////////

  char* inputtedUsernameArg = args[1];
  char* inputtedServerNameArg = args[2];
  const char usernameFlag[] = "-user";
  const char servernameFlag[] = "-server";
  
  // Check that flags are formatted properly
  if(strncmp(inputtedUsernameArg, usernameFlag, strlen(usernameFlag)) != 0 ||
     strncmp(inputtedServerNameArg, servernameFlag, strlen(servernameFlag)) != 0)
  {
    return MISFORMATTED_FLAG;
  }
  else // looks like flags are okay, get username and password
  {
    strncpy(parsedUserName, args[1] + strlen(usernameFlag), MAX_LEN_OF_USR_NAME);
    strncpy(parsedServerName, args[2] + strlen(servernameFlag), BUFSIZ);
  }

  return OPERATION_SUCCESS; 
}








