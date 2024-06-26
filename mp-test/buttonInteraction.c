/*
In this file, write a C function that returns the status of the toggle-switches on the board, with the 
following specification.
Function prototype: int getsw( void );
Parameter: none.
Return value: The four least significant bits of the return value should contain data from switches 
SW4, SW3, SW2, and SW1. SW1 corresponds to the least significant bit. All other bits of the return
value must be zero.
Notes: The function getsw will never be called before Port D has been correctly initialized. The 
switches SW4 through SW1 are connected to bits 11 through 8 of Port D.
*/

#include <stdint.h> 
#include <pic32mx.h> 
#include "conLifeDeclarations.h" // Where the files are forged together. 
// The functions are called primitives after all =>
// they are called in all the files but only declared here

int getbtns(void){
    return (PORTD >> 4) & 0xF;
}