/*
 *  FILE          : sendingThread.c
 *  PROJECT       : SENG2030-21W-Sec1-System Programming - Assignment #4
 *  PROGRAMMER    : Andrey Takhtamirov, Alex Braverman
 *  FIRST VERSION : April 18, 2020 
 *  DESCRIPTION   : 
 *			This file contains all the functions associated with the message sending
 *          thread, which happens to also be the main thread.
 *	
*/


#include "../inc/chatClient.h"


// ADD FUNCTION HEADER
void input_win(ThreadArgs* threadArgs)
{
  //////////////////////////////////
  // Set and initialize variables //
  //////////////////////////////////
  int i = 0;
  int ch = 0;
  int maxrow = 0;
  int maxcol = 0;
  int row = TOP; 
  int col = LEFT;
  WINDOW *win = threadArgs->win;
  bool* exitFlag = threadArgs->exitFlag;
  int serverSocket = *(threadArgs->serverSocket);
  int semCommandResult = 0;
  char inputBuffer[BUFSIZ] = { 0 };
  char quitString[] = ">>bye<<\n";
  char formattedMsg[BUFSIZ] = { 0 };
  char firstHalfOfParcelledMsg[MAX_LEN_OF_MESSAGE + 1] = { 0 };
  char secondHalfOfParcelledMsg[MAX_LEN_OF_MESSAGE + 1] = { 0 };


  ///////////////////////////////////////////////////////
  // Set up window, write MESSAGES banner, and chevron //
  ///////////////////////////////////////////////////////
  if(getOrCreateCursorSem() == OPERATION_FAILED) // exit if getting semaphore failed
  {
    *exitFlag == true;
    return;
  }    
  blankWin(win);                          /* make it a clean window */

  getmaxyx(win, maxrow, maxcol);          /* get window size */
  maxcol -= MAX_COL_RIGHTSIDE_PADDING;
  bzero(inputBuffer, BUFSIZ);
  
  writeMessageBanner(win);

  wmove(win, TOP, LEFT);                  /* position cusor at top */
  printChevron(win, TOP, LEFT);
  
  if(releaseCursorSem() == OPERATION_FAILED) // exit if releasing semaphore failed
  {
    *exitFlag == true;
    return;
  } 



  ////////////////////////////////
  // Get characters from window //
  ////////////////////////////////
  while(ch != '\n' && i < 80) 
  {
    getOrCreateCursorSem();
    ch = wgetch(win); // Get the first character
    releaseCursorSem();

    if(ch == ERR)
    {
      usleep(50);
      // sleep(3);
      continue;
    }

    // First get the cursor semaphore so that only the input thread can control the cursor
    if(getOrCreateCursorSem() == OPERATION_FAILED) // exit if getting semaphore failed
    {
      *exitFlag == true;
      return;
    } 

    inputBuffer[i] = ch;                       /* '\n' not copied */
    if (col < maxcol)                 /* if within window */  // Removed -2 April 13, 2021
    {
      wprintw(win, "%c", inputBuffer[i]);      /* display the char recv'd */
    }
    else                                /* last char pos reached */
    {
      col = 0;
      if (row == 1)                     // second line in the window 
      {
        scroll(win);                    /* go up one line */
        // row = 1;                        // stay at the second line 
        wmove(win, row, col);           /* move cursor to the beginning */
        printChevron(win, row, col);
        wclrtoeol(win);                 /* clear from cursor to eol */
      } 
      else
      {
        row++;
        wmove(win, row, col);           /* move cursor to the beginning */
        wrefresh(win);
        printChevron(win, row, col);
        wprintw(win, "%c", inputBuffer[i]);    /* display the char recv'd */
      }
    }

    col++;
    i++;

    // Release the cursor semaphore so it can be used by the listening thread 
    // if it receives a message
    if(releaseCursorSem() == OPERATION_FAILED) // exit if releasing semaphore failed
    {
      *exitFlag == true;
      return;
    } 

    usleep(100);
    // sleep(3);
  }

  // If the user entered 80 characters, but hasn't pressed enter, 
  // keep waiting until they press enter
  while (ch != '\n')
  {
    if(getOrCreateCursorSem() == OPERATION_FAILED) // exit if getting semaphore failed
    {
      *exitFlag == true;
      return;
    }

    ch = wgetch(win);

    if(releaseCursorSem() == OPERATION_FAILED) // exit if releasing semaphore failed
    {
      *exitFlag == true;
      return;
    } 
    usleep(100);
  }


  // 


  //////////////////////////////////////////////////////////
  // Check if user chose to quit, parcel and send message //
  //////////////////////////////////////////////////////////
  if(getOrCreateCursorSem() == OPERATION_FAILED) // exit if getting semaphore failed
  {
      *exitFlag == true;
      return;
  } 
  
  if(strncmp(inputBuffer, quitString, LEN_OF_QUIT_STRING) == 0)
  {
    *exitFlag = true; // Set flag telling listening thread to exit
  }
  else if(strlen(inputBuffer) > MAX_LEN_OF_MESSAGE)
  {
    for(int i = 0; i < MAX_LEN_OF_MESSAGE; i++)
    {
        firstHalfOfParcelledMsg[i] = inputBuffer[i];
    }
    for(int i = 0; i < strlen(inputBuffer); i++)
    {
        secondHalfOfParcelledMsg[i] = inputBuffer[i + MAX_LEN_OF_MESSAGE];
    }
    // Send first parcelled message
    sprintf(formattedMsg, "%s;%s", threadArgs->userName, firstHalfOfParcelledMsg);
    write(serverSocket, formattedMsg, strlen(formattedMsg) + 1);
    memset(&formattedMsg, 0, sizeof(formattedMsg));

    if(releaseCursorSem() == OPERATION_FAILED || getOrCreateCursorSem() == OPERATION_FAILED)
    {
        *exitFlag = true;
        return;
    }
    usleep(250);

    sprintf(formattedMsg, "%s;%s", threadArgs->userName, secondHalfOfParcelledMsg);
    write(serverSocket, formattedMsg, strlen(formattedMsg) + 1);
    memset(&formattedMsg, 0, sizeof(formattedMsg));
  }
  else
  {
    sprintf(formattedMsg, "%s;%s", threadArgs->userName, inputBuffer);
    write(serverSocket, formattedMsg, strlen(formattedMsg) + 1);
    memset(&formattedMsg, 0, sizeof(formattedMsg));
  }
  
  
  // [client's listening port][semicolon]
  // [client's IP][exclamation mark]
  // [client name][question mark]
  // [message]

  // new protocol
  // [client name][question mark]
  // [message]


  if(releaseCursorSem() == OPERATION_FAILED) // exit if releasing semaphore failed
  {
      *exitFlag == true;
      return;
  } 
}  


