/*
 *  FILE          : ncursesFunctions.c
 *  PROJECT       : SENG2030-21W-Sec1-System Programming - Assignment #4
 *  PROGRAMMER    : Andrey Takhtamirov, Alex Braverman
 *  FIRST VERSION : April 18, 2020 
 *  DESCRIPTION   : 
 *			This file contains all the functions associated with the ncurses
 *          library.
 *	
*/


#include "../inc/chatClient.h"



/*
* FUNCTION    : create_newwin
* DESCRIPTION : creates a new window 
* PARAMETERS  : int height, int width : the height and width of the current window
*               int starty, int startx : the starting position of the cursor
* RETURNS     : WINDOW * : the pointer to the the new window
*/
WINDOW *create_newwin(int height, int width, int starty, int startx)
{       
  WINDOW *local_win;
     
  local_win = newwin(height, width, starty, startx);
  wmove(local_win, 1, 1);             /* position cursor at top */
  wrefresh(local_win);     
  return local_win;
}


/*
* FUNCTION    : blankWin
* DESCRIPTION : makes current window blank of all characters
* PARAMETERS  : WINDOW *win : the window to clear
* RETURNS     : void : this function returns nothing
*/
void blankWin(WINDOW *win)
{
  int i;
  int maxrow, maxcol;
     
  getmaxyx(win, maxrow, maxcol);
  for (i = 0; i < maxcol; i++)  
  {
    wmove(win, i, 0);
    refresh();
    wclrtoeol(win);
    wrefresh(win);
  }
  wrefresh(win);
}  /* blankWin */



/*
* FUNCTION    : printChevron
* DESCRIPTION : prints the chevron indicating where the user is in their input
* PARAMETERS  : WINDOW *win : the window where the cursor exists
*               int currentX, int currentY : the current x and y coordinates of the cursor
* RETURNS     : void : this function returns nothing
*/
void printChevron(WINDOW *win, int currentX, int currentY)
{
  wprintw(win, "%c", '>');                // add chevron indicating what line the user is writing on
  wmove(win, currentX, currentY + 1);              /* position cusor after chevron */
  wprintw(win, "%c", ' ');
  wmove(win, currentX, currentY + 2);              /* position cusor after chevron */
}



/*
* FUNCTION    : setUpWindow
* DESCRIPTION : sets up the window and gives it properties such as scrollok and nodelay
* PARAMETERS  : WINDOW** inputWindow : double pointer to the window to set up
* RETURNS     : void : this function returns nothing
*/
void setUpWindow(WINDOW** inputWindow)
{
  int inputWinStartx = 0; 
  int inputWinStarty = 0;
  int inputWinWidth = 0;
  int inputWinHeight = 0;
  WINDOW* localWindow = NULL;

  initscr();                      /* Start curses mode            */
  cbreak();
  noecho();
  refresh();

  inputWinHeight = LINES;
  inputWinWidth  = COLS;
  inputWinStartx = LEFT; 
  inputWinStarty = TOP; //LINES - inputWinHeight

  // msg_win = create_newwin(msg_height, msg_width, msg_starty, msg_startx);
  // scrollok(msg_win, TRUE);
  localWindow = create_newwin(inputWinHeight, inputWinWidth, inputWinStarty, inputWinStartx);
  *inputWindow = localWindow;
  scrollok(*inputWindow, true);
  nodelay(*inputWindow, true);
}




/*
* FUNCTION    : writeMessages
* DESCRIPTION : this function writes the all the messages in the message list struct to the screen
* PARAMETERS  : WINDOW* win : the window to write the messages to
*               Last10MsgLines* last10msgs : the struct containing the last 10 messages
* RETURNS     : int : actually doesn't return anything
*/
int writeMessages(WINDOW* win, Last10MsgLines* last10msgs)
{
  int counter = 0;
  int ch = 0;
  int maxrow = 0;
  int maxcol = 0;
  int row = FOURTH_ROW; 
  int col = LEFT;
  int semCommandResult = 0;
  int middleColumn = 0;
  char timeOnReceivedMsg[MAX_LEN_OF_TIME] = { 0 };
      
  getmaxyx(win, maxrow, maxcol);          /* get window size */
  // maxcol -= MAX_COL_RIGHTSIDE_PADDING; // Decided not to add padding

  // Find out how many messages we need to print
  int numberOfMessages = last10msgs->messagesAvailable;
  int lenOfIPAddr = 0;

  ///////////////////////////////////
  // Print every available message //
  ///////////////////////////////////
  for(int i = 0; i < numberOfMessages; i++)
  {

    wmove(win, row, col); // Position the cursor in the third row, left hand side
    
    //////////////////////
    // Print IP Address //
    //////////////////////
    for (int j = 0; last10msgs->messages[i].recievedIPAddr[j] != '\0'; j++)
    {
        printChar(win, row, &col, last10msgs->messages[i].recievedIPAddr[j]);
    }

    while(col < POS_FIRST_SPACE) // Fill out IP address with spaces if not already full
    {
        printChar(win, row, &col, ' ');
    }

    printChar(win, row, &col, ' '); // Print separating space between IP and user name



    //////////////////////
    // Print Users Name //
    //////////////////////
    printChar(win, row, &col, '['); // Print left square bracket

    for (int j = 0; last10msgs->messages[i].usersName[j] != '\0'; j++) // print the users name
    {
        printChar(win, row, &col, last10msgs->messages[i].usersName[j]);
    }

    while(col < POS_SECOND_SPACE - 1) // Fill out name with spaces if not already full up to square bracket
    {
        printChar(win, row, &col, ' ');
    } 
    printChar(win, row, &col, ']'); // Print right square bracket

    printChar(win, row, &col, ' '); // Print separating space between users name and angle brackets


    //////////////////////////
    // Print Angle Brackets //
    //////////////////////////
    if(last10msgs->messages[i].isThisOurMessage == false) // Print left angle brackets if message isn't ours
    {
        printChar(win, row, &col, '<'); 
        printChar(win, row, &col, '<'); 
    }
    else    // Else print right angle brackets
    {
        printChar(win, row, &col, '>'); 
        printChar(win, row, &col, '>'); 
    }
    

    printChar(win, row, &col, ' '); // Print separating space between users name and angle brackets


    ///////////////////
    // Print Message //
    ///////////////////
    for(int j = 0; last10msgs->messages[i].messagePayload[j] != '\0'; j++)
    {
        printChar(win, row, &col, last10msgs->messages[i].messagePayload[j]);
    }
    
    while(col < POS_FOURTH_SPACE) // Fill out message with spaces if not already full
    {
        printChar(win, row, &col, ' ');
    } 

    printChar(win, row, &col, ' '); // Print separating space between message payload and time


    ////////////////
    // Print Time //
    ////////////////
    // Initialize variables used for time, and get the current time
    struct tm* timeStructure;
    timeStructure = localtime(&(last10msgs->messages[i].timeOnReception));

    // Format current time and put it into a buffer
    sprintf(timeOnReceivedMsg, "(%02d:%02d:%02d)", timeStructure->tm_hour, timeStructure->tm_min, timeStructure->tm_sec);

    // Print the time
    for(int j = 0; timeOnReceivedMsg[j] != '\0'; j++)
    {
        printChar(win, row, &col, timeOnReceivedMsg[j]);
    }

    row++; // increment to next row
    col = 0;
  }

  return 0;
}



/*
* FUNCTION    : writeMessageBanner
* DESCRIPTION : this function writes the message banner "- - - Messages - - -" to the screen
* PARAMETERS  : WINDOW* win : the window to write the messages to
*               Last10MsgLines* last10msgs : the struct containing the last 10 messages
* RETURNS     : int : actually doesn't return anything
*/
void writeMessageBanner(WINDOW* win)
{

    //////////////////////////////
    // Set/initialize variables //
    ////////////////////////////// 
    int counter = 0;
    int ch = 0;
    int maxrow = 0;
    int maxcol = 0;
    int row = THIRD_ROW; 
    int col = LEFT;
    int semCommandResult = 0;
    char messageTitle[] = "MESSAGES";
    int middleColumn = 0;
        
    getmaxyx(win, maxrow, maxcol);          /* get window size */


    ////////////////////////////////
    // Calculate middle of window //
    ////////////////////////////////
    // Next find the middle column so we can write the banner and stop near middle to write MESSAGES
    if(maxcol % 2 > 0) // find middle if max col is odd
    {
        middleColumn = (maxcol - 1) / 2;
    }
    else
    {
        middleColumn = maxcol / 2;
    }

    wmove(win, row, col);                  /* position cusor at top */
    

    /////////////////////////////
    // Print "MESSAGES" banner //
    /////////////////////////////
    // write the "- - - - " pattern on the left of the "MESSAGES" title
    while(col < middleColumn - HALF_OF_MSGS_TITLE) // "MESSAGES" is 8 letters, and we want it to be in the middle
    {                                                   
        printChar(win, row, &col, '-');
        printChar(win, row, &col, ' '); // SHOULD BE SPACE, CURRENTLY DASH AS A TEST
    }

    for(int i = 0; messageTitle[i] != '\0'; i++) // Print the "MESSAGES" title
    {
        printChar(win, row, &col, messageTitle[i]);
    }

    // write a space before the "- - - - " pattern on the other side of the MESSAGES title
    printChar(win, row, &col, ' ');

    // Finally write "- - - - " pattern on the right of the "MESSAGES" title
    while(col < maxcol) 
    {                                                   
        printChar(win, row, &col, '-');
        printChar(win, row, &col, ' '); // SHOULD BE SPACE, CURRENTLY DASH AS A TEST
    }
}



// ADD HEADER COMMENT
void printChar(WINDOW* win, int row, int* col, char charToPrint)
{
    wprintw(win, "%c", charToPrint); // print char
    *col = *col + 1;                 // update current column
    wmove(win, row, *col);           // move cursor to current column
}

