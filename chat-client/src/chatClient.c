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
#include <ifaddrs.h>



int parseArguments(char* parsedUserName, char* parsedServerName, char* args[]);
int getThisMachinesPublicIP(char* thisMachinesPublicIP);


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

  // Get this machines IP address using ens33 or eth0 - BE CAREFUL, THE SERVER SHOULD REALLY BE THE ONE GETTING THE IP AND PORT
  getThisMachinesPublicIP(userIP);


  ///////////////////
  // Set up socket //
  ///////////////////
  socketSetupResult = setUpSocket(&serverSocket, host);

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
  threadArgs.userIP = userIP;


  ////////////////////////////
  // Start listening thread //
  ////////////////////////////

  // this thread will also write incoming messages to the screen
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


  // clean up resources when done
  close(serverSocket);

  delwin(inputWindow);

  endwin();

  deleteCursorSem();

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





// ADD FUNCTION HEADER COMMENT - DECIDE IF WE'RE KEEPING THIS
int getThisMachinesPublicIP(char* thisMachinesPublicIP)
{
  struct ifaddrs* startingAddress = NULL; // Starting address of linked list of address
  struct ifaddrs* currentAddress = NULL;  // Current address as we go through list
  int family = 0;
  int operationResult = 0;
  char host[NI_MAXHOST];   // Where the address is stored once it's found

  if (getifaddrs(&startingAddress) == OPERATION_FAILED) // Check if we can get starting address
  {
    return OPERATION_FAILED;
  }

  // Go through each address on linked list and find one that is of the family AF_INET
  for (currentAddress = startingAddress; currentAddress != NULL; currentAddress = currentAddress->ifa_next) 
  {
    if (currentAddress->ifa_addr == NULL)
    {
      continue;
    }

    operationResult=getnameinfo(currentAddress->ifa_addr,sizeof(struct sockaddr_in),host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);

    // Get the ipv4 address of ens33 if that is the name of the current address in the list
    if((strcmp(currentAddress->ifa_name,"ens33")==0)&&(currentAddress->ifa_addr->sa_family==AF_INET))
    {
      if (operationResult != 0)
      {
        freeifaddrs(startingAddress);
        return OPERATION_FAILED;
      }
      sprintf(thisMachinesPublicIP, "%s", host); // We found our IP address
      break; 
    }
    // Or get the ipv4 of eth0 if that is the current address
    else if((strcmp(currentAddress->ifa_name,"eth0")==0)&&(currentAddress->ifa_addr->sa_family==AF_INET))
    {
      if (operationResult != 0)
      {
        freeifaddrs(startingAddress);
        return OPERATION_FAILED;
      }
      sprintf(thisMachinesPublicIP, "%s", host); // We found our IP address
      break; 
    }
  }

  freeifaddrs(startingAddress);
}




