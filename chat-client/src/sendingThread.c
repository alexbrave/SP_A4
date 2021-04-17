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


  ///////////////////////////////////////////////////////
  // Set up window, write MESSAGES banner, and chevron //
  ///////////////////////////////////////////////////////
  getOrCreateCursorSem();    
  blankWin(win);                          /* make it a clean window */

  getmaxyx(win, maxrow, maxcol);          /* get window size */
  maxcol -= MAX_COL_RIGHTSIDE_PADDING;
  bzero(inputBuffer, BUFSIZ);
  
  writeMessageBanner(win);

  wmove(win, TOP, LEFT);                  /* position cusor at top */
  printChevron(win, TOP, LEFT);
  releaseCursorSem();



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
    semCommandResult = getOrCreateCursorSem(); // LOG AND RETURN IF THIS DOESN'T WORK

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
        // box(win, 0, 0);                 /* draw the box again */
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
    semCommandResult = releaseCursorSem(); // LOG AND RETURN IF THIS DOESN'T WORK

    usleep(100);
    // sleep(3);
  }



  //////////////////////////////////////////////////
  // Check if user chose to quit and Send Message //
  //////////////////////////////////////////////////
  semCommandResult = getOrCreateCursorSem(); // LOG AND RETURN IF THIS DOESN'T WORK
  if(strncmp(inputBuffer, quitString, LEN_OF_QUIT_STRING) == 0)
  {
    *exitFlag = true; // Set flag telling listening thread to exit
  }
  sprintf(formattedMsg, "%d;%s!%s?%s", 5000, "172.26.177.173", threadArgs->userName, inputBuffer);
  write(serverSocket, formattedMsg, sizeof(formattedMsg));
  memset(&formattedMsg, 0, sizeof(formattedMsg));

  // [client's listening port][semicolon]
  // [client's IP][exclamation mark]
  // [client name][question mark]
  // [message]


  semCommandResult = releaseCursorSem(); // LOG AND RETURN IF THIS DOESN'T WORK


  // If the user entered 80 characters, but hasn't pressed enter, 
  // keep waiting until they press enter
  while (ch != '\n')
  {
    getOrCreateCursorSem();
    ch = wgetch(win);
    releaseCursorSem();
    usleep(100);
  }


}  


