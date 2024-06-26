/* runningProcess.c

   This file written 2015 by F Lundevall
   Updated 2017-04-21 by F Lundevall

   This file should be changed by YOU! So you must
   add comment(s) here with your name(s) and date(s):

   This file was last modified 2023-03-02 by Juris Homickis & Victor Naumburg

   For copyright and licensing, see file COPYING */

#include <stdint.h>  /* Declarations of uint_32 and the like */
#include <pic32mx.h> /* Declarations of system-specific addresses etc */
#include "conLifeDeclarations.h" /* Declatations for these labs */

volatile int *fauxPORTE = (volatile int *)0xbf886110;

int timeOutCount = 0;
// 0 = Menu, 1 = Simulation
int simFlag = 0;

/* Lab-specific initialization goes here*/
void conLifeInit(void)
{
  volatile unsigned *fauxTRISE = (volatile unsigned *)0xbf886100;
  *fauxTRISE = *fauxTRISE & 0xfff1;

  TRISD = 0b111111100000;

  T2CON = 0b1110000; // Prescale of 8 
  PR2 = (80000000 / 8) / 100; // => retard d'environ cinq secondes. Vraisemblablmenet à cause d'écrit dans l'ancien 

  TMR2 = 0; // assuring that it not have any hold-over value

  
  IPCSET(2) = 0x00000005; // Set priority level = 2. CHANGÉE DE 0x0C
  IPCSET(2) = 0x00000001; // Set subpriority level = 1
  IPCSET(3) = 0x00000007; // Ptet "ipc3 chez" l'affecte?
  IPCSET(3) = 0x00000001;


  IFSCLR(0) = 0x100;

  IECSET(0) = 0x100; // enable interrupts from Timer 2
  IECSET(0) = 0b1000000000000000; // Enable interrupts from SW3 at the 16th bit. L1403 pic32mx.h

  enable_interrupt();
  T2CONSET = 0x8000; // Initialize clock

  //

  return;
}

int currentlySelectedRow=0; 
char selectedSpeed=5; 
int selectedStartState=12; // For the "123R", 12 picks the 1
int timeout = 1;

/*
  * DOWN click changes the option: accessible through currentlySelectedRow
  * LEFT || RIGHT changes the parameter: accessible through selectedSpeed & selectedStartState
  * CONFIRM exists the menu
*/
void setOptionAndParameters(){
  int pressedBtns = getbtns();

  if (pressedBtns){ // so that not called unnecessarily
    // Down click: switch option
    if(pressedBtns & 0b1000){
      if (currentlySelectedRow < 2)currentlySelectedRow++;
      else currentlySelectedRow = 0;
    }

    // Left button click:
    if(pressedBtns & 0b0100){
      // DELAY
      if(currentlySelectedRow == 0 && selectedSpeed > 1){
        selectedSpeed--; 
        display_string(0, "DELAY         ", selectedSpeed);
      }
      // STR STATE
      else if (currentlySelectedRow == 1){
        if(selectedStartState <= 12) selectedStartState = 15;
        else selectedStartState--;
      }
      else if (simFlag == 0 && currentlySelectedRow == 2) {
        if(selectedStartState == 12) {
          initGol(1);
        } else if (selectedStartState == 13) {
          initGol(2);
        } else if (selectedStartState == 14) {
          initGol(3);
        } else if (selectedStartState == 15) {
          initGol(4);
        }
        // Sets simFlag to 1, indicating main loop to start the simulation instead 
        simFlag = 1;
        // scale delay
        selectedSpeed = selectedSpeed * 10;
      }
    }

    // Right button click:
    if(pressedBtns & 0b0010){
       // DELAY
      if(currentlySelectedRow == 0 && selectedSpeed < 20) {
        selectedSpeed++;
        display_string(0, "DELAY         ", selectedSpeed);
      }
      // STR STATE
      else if(currentlySelectedRow == 1){
        if(selectedStartState >= 15) selectedStartState = 12;
        else selectedStartState++;
      } 
      else if (simFlag == 1) {
        simFlag = 0;
        currentlySelectedRow = 0;
        selectedSpeed = 10;
        display_string(0, "DELAY         ", 10);
      } 
    }

    // CONFirm
    if (pressedBtns & 0b0110){
      // Resets the program back to options menu from the simulation
      
      // Selects start state based on selectedStartState
      
    }
  }
}


/* Interrupt Service Routine */
void user_isr(void)
{
    //change speed depending on simflag
    if(timeOutCount == 1){
      setOptionAndParameters();
      
      IFSCLR(0) = 0x0100;
      timeOutCount = 0;
   }
    else if(IFS(0) & 0x100) {
      timeOutCount++;
      IFSCLR(0) = 0x0100; // Clear the timer interrupt status flag 
    }
}

/* This function is called repetitively from the main program */
void run(void)
{
  if (simFlag == 0) {
    display_update(currentlySelectedRow, selectedSpeed, selectedStartState); // and select
  } else if (simFlag == 1) {
    // perform matrix update
    updateMatrix();
    // Set all 4 OLED rows to whatever the current ppMatrix is
    setRow1(icon);
    setRow2(icon2);
    setRow3(icon3);
    setRow4(icon4);
    // Show the simulation
    display_image(0, icon, icon2, icon3, icon4);
    //inefficient but VERY simple delay to control sim speed
    delay(selectedSpeed);
  }
}
