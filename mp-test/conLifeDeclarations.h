/* conLifeDeclarations.h
   Header file for all labs.
   This file written 2015 by F Lundevall
   Some parts are original code written by Axel Isaksson

   This file was last modified 2023-03-02 by Juris Homickis & Victor Naumburg

   For copyright and licensing, see file COPYING */

/* Declare display-related functions from conLifefunc.c */
void display_image(int x, uint8_t *row1, uint8_t *row2, uint8_t *row3, uint8_t *row4);
void display_init(void);
void display_string(int line, char *s, char selectedStartState);
void display_update(int currentlySelectedRow, char selectedSpeed, int selectedStartState );
uint8_t spi_send_recv(uint8_t data);

/* Declare lab-related functions from conLifefunc.c */
char * itoaconv( int num );
void run(void);
int nextprime( int inval );
void quicksleep(int cyc);

/* Declare display_debug - a function to help debugging.

   After calling display_debug,
   the two middle lines of the display show
   an address and its current contents.

   There's one parameter: the address to read and display.

   Note: When you use this function, you should comment out any
   repeated calls to display_image; display_image overwrites
   about half of the digits shown by display_debug.
*/
void display_debug( volatile int * const addr );

/* Declare bitmap array containing font */
extern const uint8_t const font[128*8];
/* Declare bitmap array containing rows 1 to 4 of the OLED */
extern uint8_t icon[128];
extern uint8_t icon2[128];
extern uint8_t icon3[128];
extern uint8_t icon4[128];
// Declade matrixes used for game of life logic
extern char ppMatrix[32][128];   //current gen
extern char nextMatrix[32][128]; //next gen crafted here
/* Declare text buffer for display output */
extern char textbuffer[4][16];

/* Declare functions written by students.
   Note: Since we declare these functions here,
   students must define their functions with the exact types
   specified in the laboratory instructions. */
/* Written as part of asm lab: delay, time2string */
void delay(int);
void time2string( char *, int );
/* Written as part of i/o lab: getbtns, getsw, enable_interrupt */
int getbtns(void);
void enable_interrupt(void);

// functions from gol.c related to the game of life logic
void initMatrix(int state);
void clearMatrix(void);
int countNeighbors(int r, int c);
void updateMatrix(void);
void initGol(int state); 

// functions from gol.c 
void setRow1(uint8_t *row);
void setRow2(uint8_t *row);
void setRow3(uint8_t *row);
void setRow4(uint8_t *row);

extern int simFlag;
