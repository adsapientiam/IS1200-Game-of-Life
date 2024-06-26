/* conLifefunc.c
   This file written 2015 by F Lundevall
   Some parts are original code written by Axel Isaksson

   This file was last modified 2023-03-02 by Juris Homickis & Victor Naumburg

   For copyright and licensing, see file COPYING */

#include <stdint.h>   /* Declarations of uint_32 and the like */
#include <pic32mx.h>  /* Declarations of system-specific addresses etc */
#include "conLifeDeclarations.h"  /* Declatations for these labs */

/* Declare a helper function which is local to this file */
static void num32asc( char * s, int ); 

#define DISPLAY_CHANGE_TO_COMMAND_MODE (PORTFCLR = 0x10)
#define DISPLAY_CHANGE_TO_DATA_MODE (PORTFSET = 0x10)

#define DISPLAY_ACTIVATE_RESET (PORTGCLR = 0x200)
#define DISPLAY_DO_NOT_RESET (PORTGSET = 0x200)

#define DISPLAY_ACTIVATE_VDD (PORTFCLR = 0x40)
#define DISPLAY_ACTIVATE_VBAT (PORTFCLR = 0x20)

#define DISPLAY_TURN_OFF_VDD (PORTFSET = 0x40)
#define DISPLAY_TURN_OFF_VBAT (PORTFSET = 0x20)

/* quicksleep:
   A simple function to create a small delay.
   Very inefficient use of computing resources,
   but very handy in some special cases. */
void quicksleep(int cyc) {
	int i;
	for(i = cyc; i > 0; i--);
}


uint8_t spi_send_recv(uint8_t data) {
	while(!(SPI2STAT & 0x08));
	SPI2BUF = data;
	while(!(SPI2STAT & 1));
	return SPI2BUF;
}

void display_init(void) {
        DISPLAY_CHANGE_TO_COMMAND_MODE;
	quicksleep(10);
	DISPLAY_ACTIVATE_VDD;
	quicksleep(1000000);
	
	spi_send_recv(0xAE);
	DISPLAY_ACTIVATE_RESET;
	quicksleep(10);
	DISPLAY_DO_NOT_RESET;
	quicksleep(10);
	
	spi_send_recv(0x8D);
	spi_send_recv(0x14);
	
	spi_send_recv(0xD9);
	spi_send_recv(0xF1);
	
	DISPLAY_ACTIVATE_VBAT;
	quicksleep(10000000);
	
	spi_send_recv(0xA1);
	spi_send_recv(0xC8);
	
	spi_send_recv(0xDA);
	spi_send_recv(0x20);
	
	spi_send_recv(0xAF);
}

/*
	* selectedSpeedDigits added so that for row 0 the string's final 1 to 2 to represent the selected delay digits
*/
void display_string(int line, char *s, char selectedSpeedDigits) {
	
  int i, digitCount;
	
  if(line < 0 || line >= 4)
		return;
	if(!s)
		return;

  /* Fixing the digits so that they can be appended on the String */
  // Calculate how many digits: max 99 cuz afterwards it be too fast either way maybe
    // -1 is argument of sent by rows not using this
  if(selectedSpeedDigits < 10 && selectedSpeedDigits != -1){
    digitCount = 1;
  }
    // 100 will either way be the max value
  else if(selectedSpeedDigits > 100) digitCount = 3; 
  else digitCount = 2; // 100 > X >= 10

  // In order to make the digits easily accessible in if(i >= 16 - digitCount)
  char digits[3];
  char digitNumberX = 0;
  while(selectedSpeedDigits){
    digits[digitNumberX++] = selectedSpeedDigits % 10;
    selectedSpeedDigits /= 10; // Proceeds to the next digit
  }

  /* Inserting the characther sequence */
	for(i = 0; i < 16; i++){

    // If 15 is the speed, digits stores them as [5,1] such that here it will
    // be unpacked as 1 then 5
    if(line==0 && i >= 16 - digitCount){ // 
      textbuffer[line][i] = 0x30 + digits[--digitCount];
    }
    // Old code: 
    // Fills a row with the contents of a string. 
    // When no more charachters, prints " "
		else if(*s) {  
			textbuffer[line][i] = *s;
			s++;
		} else
			textbuffer[line][i] = ' ';
  }
}

// Selection parameter selects such that the right
/* display_update modified such that it not only displays, but can also inverse characthers, that way making the background light up. In a sequence, such inversions look like a selection.
  *currentlySelectedRow inverts the characthers of the option
  * selectedSpeed & selectedStartState select the respective parameter values when
     the corresponding currentlySelectedRow is selected
*/
void display_update(int currentlySelectedRow, char selectedSpeed, int selectedStartState) {
	int i, j, k;
	int c;
    // i < 4 since it's 4 rows
	for(i = 0; i < 4; i++) {
		DISPLAY_CHANGE_TO_COMMAND_MODE;
		spi_send_recv(0x22);
		spi_send_recv(i);
		
		spi_send_recv(0x0);
		spi_send_recv(0x10);
		
		DISPLAY_CHANGE_TO_DATA_MODE;
		
    // j < 16 since each row has 16 characthers
		for(j = 0; j < 16; j++) {
			c = textbuffer[i][j];
			if(c & 0x80)
				continue;
			
      // Prints all the pixels of a characther.
        // If statements such that correct characther inverted
    for(k = 0; k < 8; k++){
        char notInvertedRound = 1;

        // Delay row 0: Will only invert the last one or two last digits
        if(currentlySelectedRow==i && i==0 && selectedSpeed && j > 13){
          if(selectedSpeed < 10 ) {
            if (j==15) spi_send_recv(~font[c*8 + k]);
            else spi_send_recv(font[c*8 + k]);
          }
          else if (selectedSpeed >= 10) {
            spi_send_recv(~font[c*8 + k]);
          }
          notInvertedRound--;
        }

        // Rather read like "when (i == 0 ....), then ...."
        // START STR row 1: Initial selectedSpeed as to quickly become false if false
        if(selectedStartState && i == 1 && j == selectedStartState) {
          spi_send_recv(~font[c*8 + k]);
          notInvertedRound--;
        }

        // What _option_ is currently selected
        if(i == currentlySelectedRow && ((i == 0 && j < 5) || (i == 1 && j < 9) || ( i == 2 && j < 5))){
            spi_send_recv(~font[c*8 + k]);
            notInvertedRound--;
        }
        
	// If none of the if-statements above are  triggered, print them without inversing
        if(notInvertedRound == 1){
          spi_send_recv(font[c*8 + k]);
        }
      }
		}
	}
}

// displays the game of life cell simulation by showing the four rows
void display_image(int x, uint8_t *row1, uint8_t *row2, uint8_t *row3, uint8_t *row4) {
	int i, j;
	int flag = 0;
	for(i = 0; i < 4; i++) {
		DISPLAY_CHANGE_TO_COMMAND_MODE;

		spi_send_recv(0x22);
		spi_send_recv(i);
		
    spi_send_recv(0x00);
		spi_send_recv(0x10);
		
		DISPLAY_CHANGE_TO_DATA_MODE;
		if (i == 0) {
      for(j = 0; j < 128; j++) spi_send_recv(row1[j]);
    } else if (i == 1) {
      for(j = 0; j < 128; j++) spi_send_recv(row2[j]);
    } else if (i == 2) {
      for(j = 0; j < 128; j++) spi_send_recv(row3[j]);
    } else if (i == 3) {
      for(j = 0; j < 128; j++) spi_send_recv(row4[j]);
    }
	}
}

/* Helper function, local to this file.
   Converts a number to hexadecimal ASCII digits. */
static void num32asc( char * s, int n ) 
{
  int i;
  for( i = 28; i >= 0; i -= 4 )
    *s++ = "0123456789ABCDEF"[ (n >> i) & 15 ];
}

/*
 * nextprime
 * 
 * Return the first prime number larger than the integer
 * given as a parameter. The integer must be positive.
 */
#define PRIME_FALSE   0     /* Constant to help readability. */
#define PRIME_TRUE    1     /* Constant to help readability. */
int nextprime( int inval )
{
   register int perhapsprime = 0; /* Holds a tentative prime while we check it. */
   register int testfactor; /* Holds various factors for which we test perhapsprime. */
   register int found;      /* Flag, false until we find a prime. */

   if (inval < 3 )          /* Initial sanity check of parameter. */
   {
     if(inval <= 0) return(1);  /* Return 1 for zero or negative input. */
     if(inval == 1) return(2);  /* Easy special case. */
     if(inval == 2) return(3);  /* Easy special case. */
   }
   else
   {
     /* Testing an even number for primeness is pointless, since
      * all even numbers are divisible by 2. Therefore, we make sure
      * that perhapsprime is larger than the parameter, and odd. */
     perhapsprime = ( inval + 1 ) | 1 ;
   }
   /* While prime not found, loop. */
   for( found = PRIME_FALSE; found != PRIME_TRUE; perhapsprime += 2 )
   {
     /* Check factors from 3 up to perhapsprime/2. */
     for( testfactor = 3; testfactor <= (perhapsprime >> 1) + 1; testfactor += 1 )
     {
       found = PRIME_TRUE;      /* Assume we will find a prime. */
       if( (perhapsprime % testfactor) == 0 ) /* If testfactor divides perhapsprime... */
       {
         found = PRIME_FALSE;   /* ...then, perhapsprime was non-prime. */
         goto check_next_prime; /* Break the inner loop, go test a new perhapsprime. */
       }
     }
     check_next_prime:;         /* This label is used to break the inner loop. */
     if( found == PRIME_TRUE )  /* If the loop ended normally, we found a prime. */
     {
       return( perhapsprime );  /* Return the prime we found. */
     } 
   }
   return( perhapsprime );      /* When the loop ends, perhapsprime is a real prime. */
} 

/*
 * itoa
 * 
 * Simple conversion routine
 * Converts binary to decimal numbers
 * Returns pointer to (static) char array
 * 
 * The integer argument is converted to a string
 * of digits representing the integer in decimal format.
 * The integer is considered signed, and a minus-sign
 * precedes the string of digits if the number is
 * negative.
 * 
 * This routine will return a varying number of digits, from
 * one digit (for integers in the range 0 through 9) and up to
 * 10 digits and a leading minus-sign (for the largest negative
 * 32-bit integers).
 * 
 * If the integer has the special value
 * 100000...0 (that's 31 zeros), the number cannot be
 * negated. We check for this, and treat this as a special case.
 * If the integer has any other value, the sign is saved separately.
 * 
 * If the integer is negative, it is then converted to
 * its positive counterpart. We then use the positive
 * absolute value for conversion.
 * 
 * Conversion produces the least-significant digits first,
 * which is the reverse of the order in which we wish to
 * print the digits. We therefore store all digits in a buffer,
 * in ASCII form.
 * 
 * To avoid a separate step for reversing the contents of the buffer,
 * the buffer is initialized with an end-of-string marker at the
 * very end of the buffer. The digits produced by conversion are then
 * stored right-to-left in the buffer: starting with the position
 * immediately before the end-of-string marker and proceeding towards
 * the beginning of the buffer.
 * 
 * For this to work, the buffer size must of course be big enough
 * to hold the decimal representation of the largest possible integer,
 * and the minus sign, and the trailing end-of-string marker.
 * The value 24 for ITOA_BUFSIZ was selected to allow conversion of
 * 64-bit quantities; however, the size of an int on your current compiler
 * may not allow this straight away.
 */
#define ITOA_BUFSIZ ( 24 )
char * itoaconv( int num )
{
  register int i, sign;
  static char itoa_buffer[ ITOA_BUFSIZ ];
  static const char maxneg[] = "-2147483648";
  
  itoa_buffer[ ITOA_BUFSIZ - 1 ] = 0;   /* Insert the end-of-string marker. */
  sign = num;                           /* Save sign. */
  if( num < 0 && num - 1 > 0 )          /* Check for most negative integer */
  {
    for( i = 0; i < sizeof( maxneg ); i += 1 )
    itoa_buffer[ i + 1 ] = maxneg[ i ];
    i = 0;
  }
  else
  {
    if( num < 0 ) num = -num;           /* Make number positive. */
    i = ITOA_BUFSIZ - 2;                /* Location for first ASCII digit. */
    do {
      itoa_buffer[ i ] = num % 10 + '0';/* Insert next digit. */
      num = num / 10;                   /* Remove digit from number. */
      i -= 1;                           /* Move index to next empty position. */
    } while( num > 0 );
    if( sign < 0 )
    {
      itoa_buffer[ i ] = '-';
      i -= 1;
    }
  }
  /* Since the loop always sets the index i to the next empty position,
   * we must add 1 in order to return a pointer to the first occupied position. */
  return( &itoa_buffer[ i + 1 ] );
}
