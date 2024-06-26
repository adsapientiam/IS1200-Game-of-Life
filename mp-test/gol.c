
// gol.c
// Game of life simulation code
// By: Juris Homickis
// Last modified: 2023-03-02
// RIP John Conway, 1937-2020
#include <stdint.h>   /* Declarations of uint_32 and the like */
#include <stdlib.h>
#include <pic32mx.h>  /* Declarations of system-specific addresses etc */

// Defined as per the resolution of the OLED display
#define ROWS 32
#define COLS 128
// Needed to circumvent the lack of some standard functions. Is not actually called anywhere
// within gol.c
void *stdout;

// Define two 2D arrays. char was chosen as it takes the least space in theory
// ppMatrix (a name kept from when there was a double pointer implementation, so
// pointer pointer Matrix, ppMatrix) is a matrix containing the CURRENT cell simulation state
char ppMatrix[ROWS][COLS];
// nextMatrix is sort of like a sandbox where the next state is built. To build the next state,
// the last/current state has to be stored somewhere intact to reference from
char nextMatrix[ROWS][COLS];

// fill the nextMatrix with "dead cells"
void clearMatrix() {
    int r,c;
    for (r = 0; r < ROWS; r++) {
        for (c = 0; c < COLS; c++) nextMatrix[r][c] = '-';
    }
}

//initializes a matrix based on the chosen state (1,2,3,R)
void initMatrix(int state) {
    int r,c;
    // initialize the ppMatrix with dead cells
    for (r = 0; r < ROWS; r++) {
        for (c = 0; c < COLS; c++) ppMatrix[r][c] = '-';
    }
    // initialize mode 1,2,3,R
    switch (state) {
    case 1: //Glider
        ppMatrix[1][2] = 'o';
        ppMatrix[2][3] = 'o';
        ppMatrix[3][1] = 'o';
        ppMatrix[3][2] = 'o';
        ppMatrix[3][3] = 'o';
        break;

    case 2: //Pure glider generator
        //row 11
        ppMatrix[11][62] = 'o';
        ppMatrix[11][63] = 'o';
        ppMatrix[11][64] = 'o';
        ppMatrix[11][65] = 'o';

        //row 13
        ppMatrix[13][60] = 'o';
        ppMatrix[13][61] = 'o';
        ppMatrix[13][62] = 'o';
        ppMatrix[13][63] = 'o';
        ppMatrix[13][64] = 'o';
        ppMatrix[13][65] = 'o';
        ppMatrix[13][66] = 'o';
        ppMatrix[13][67] = 'o';

        //row 15
        ppMatrix[15][58] = 'o';
        ppMatrix[15][59] = 'o';
        ppMatrix[15][60] = 'o';
        ppMatrix[15][61] = 'o';
        ppMatrix[15][62] = 'o';
        ppMatrix[15][63] = 'o';
        ppMatrix[15][64] = 'o';
        ppMatrix[15][65] = 'o';
        ppMatrix[15][66] = 'o';
        ppMatrix[15][67] = 'o';
        ppMatrix[15][68] = 'o';
        ppMatrix[15][69] = 'o';

        //row 17
        ppMatrix[17][60] = 'o';
        ppMatrix[17][61] = 'o';
        ppMatrix[17][62] = 'o';
        ppMatrix[17][63] = 'o';
        ppMatrix[17][64] = 'o';
        ppMatrix[17][65] = 'o';
        ppMatrix[17][66] = 'o';
        ppMatrix[17][67] = 'o';

        //row 19
        ppMatrix[19][62] = 'o';
        ppMatrix[19][63] = 'o';
        ppMatrix[19][64] = 'o';
        ppMatrix[19][65] = 'o';
        break;

    case 3: //Gosper glider gun
        ppMatrix[1][25] = 'o';

        ppMatrix[2][23] = 'o';
        ppMatrix[2][25] = 'o';

        ppMatrix[3][13] = 'o';
        ppMatrix[3][14] = 'o';
        ppMatrix[3][21] = 'o';
        ppMatrix[3][22] = 'o';
        ppMatrix[3][35] = 'o';
        ppMatrix[3][36] = 'o';

        ppMatrix[4][12] = 'o';
        ppMatrix[4][16] = 'o';
        ppMatrix[4][21] = 'o';
        ppMatrix[4][22] = 'o';
        ppMatrix[4][35] = 'o';
        ppMatrix[4][36] = 'o';

        ppMatrix[5][1] = 'o';
        ppMatrix[5][2] = 'o';
        ppMatrix[5][11] = 'o';
        ppMatrix[5][17] = 'o';
        ppMatrix[5][21] = 'o';
        ppMatrix[5][22] = 'o';

        ppMatrix[6][1] = 'o';
        ppMatrix[6][2] = 'o';
        ppMatrix[6][11] = 'o';
        ppMatrix[6][15] = 'o';
        ppMatrix[6][17] = 'o';
        ppMatrix[6][18] = 'o';
        ppMatrix[6][23] = 'o';
        ppMatrix[6][25] = 'o';

        ppMatrix[7][11] = 'o';
        ppMatrix[7][17] = 'o';
        ppMatrix[7][25] = 'o';

        ppMatrix[8][12] = 'o';
        ppMatrix[8][16] = 'o';

        ppMatrix[9][13] = 'o';
        ppMatrix[9][14] = 'o';
        break;

    case 4: ;//R, completely random cells!
        int i,k;
        for (i = 0; i < ROWS; i++) {
            for (k = 0; k < COLS; k++) {
                int ran = random();
                if (ran%2 == 1) {
                    ppMatrix[i][k] = 'o';
                }   
            }
        }
        break;
    }
}

//count all neighbors of a given cell
int countNeighbors(int r, int c) {
    int count = 0;
    int i,j;
    // Checks all 8 cells around itself, ignores itself
    for (i = -1; i < 2; i++) {
        for (j = -1; j < 2; j++) {
            if (i == 0 && j == 0) continue; //ignore self
            int row = r + i;
            int col = c + j;
            // Wraparound checks and adjustments. 
            // If a check is out of bounds, adjust the coordinate.
            if (row < 0) row = 31;
            if (row > 31) row = 0;
            if (col < 0) col = 127;
            if (col > 127) col = 0;
            //add to count if a "alive" neighbor is found
            if (ppMatrix[row][col] == 'o') count += 1;
        }
    }
    //if (count > 1) printf("%d\n",count);
    return count;
}

//Updates the matrix
void updateMatrix() {
    int neighbours = 0;
    int i,j;
    //Cycle thru all cells
    for (i = 0; i < ROWS; i++){
        for (j = 0; j < COLS; j++){
            // Perform a neighbor check for given cell, determine if it will be 
            // dead or alive on next generation
            //
            // Game of Life rules state that:
            // For a living cell to live, 2 or 3 neighbors have to be alive
            //
            // A cell dies with less than 2 neighbors due to loneliness
            // A cell dies with more than 3 neighbors due to overpopulation
            //
            // For a cell to be born, there have to be exactly 3 living neighbors 
            neighbours = countNeighbors(i, j);
            if (ppMatrix[i][j] == 'o'){
                if (neighbours == 2 || neighbours == 3) {
                    nextMatrix[i][j] = 'o';
                } else {
                    nextMatrix[i][j] = '-';
                }
            } else {
                if (neighbours == 3) {
                    nextMatrix[i][j] = 'o';
                } else {
                    nextMatrix[i][j] = '-';
                }
            }
        }
    }
    // A somewhat inefficient way to copy contents from nextMatrix to ppMatrix
    for (i = 0; i < ROWS; i++){
        for (j = 0; j < COLS; j++){
            ppMatrix[i][j] = nextMatrix[i][j];
        }
    }
    //Clear nextMatrix before the next update
    clearMatrix();
}

void initGol(int state) {
    clearMatrix();
    //init the ppMatrix with a preset
    //1: gldier, 2: pure glider gen
    //3: glider gun, 4: (R:) Random
    initMatrix(state);
}

// The following four functions translate the ppMatrix into 4 rows that are to be
// sent to the display, each functiomn for each row. Could this have been done better?
// Probably. But time is not unlimited unfortunately.
//
// The display shows things in 4 rows. Each row consists of 128 columns, 8 px tall.
// The func fills each column with a series of 1 or a 0 depending on a cells state.
// Once the series of 0 and 1 is made, the binary number gets converted into decimal and
// added to the rows array. So example:
// We get 0b01001011 from ppMatrix on the first column. This gets turned into dec (75)
// and appended to for example icon[], the array for the first row.
void setRow1 (uint8_t *row) {
    int i, j, k;
    int arr[8];
    for (i = 0; i < 128; i++) {
        for (j = 0; j <= 7; j++) {
            // append the 8 wide array with the current cell
            if (ppMatrix[j][i] == 'o') {
                arr[j] = 1;
            } else {
                arr[j] = 0;
            }
        }
        // Reverse the array. For some reason a y flip is done in the code above and
        // I could not figure out why, but hey this works
        int temp;

        for(k = 0; k<4; k++){
            temp = arr[k];
            arr[k] = arr[8-k-1];
            arr[8-k-1] = temp;
        }

        // Convert the binary number from the array to decimal
        int size = 8;
        int binary = 0;
        int base = 1;
        for(k = size-1; k >= 0; k--) {
            binary += arr[k] * base;
            base *= 2;
        }
        // Append it to the array
        row[i] = binary;
    }
    return;
}

void setRow2 (uint8_t *row) {
    int i, j, k;
    int arr[8];
    for (i = 0; i < 128; i++) {
        for (j = 0; j <= 7; j++) {
            if (ppMatrix[j+8][i] == 'o') {
                arr[j] = 1;
            } else {
                arr[j] = 0;
            }
        }

        int temp;
        for(k = 0; k<4; k++){
            temp = arr[k];
            arr[k] = arr[8-k-1];
            arr[8-k-1] = temp;
        }

        int size = 8;
        int binary = 0;
        int base = 1;
        for(k = size-1; k >= 0; k--) {
            binary += arr[k] * base;
            base *= 2;
        }
        row[i] = binary;
    }
    return;
}

void setRow3 (uint8_t *row) {
    int i, j, k;
    int arr[8];
    for (i = 0; i < 128; i++) {
        for (j = 0; j <= 7; j++) {
            if (ppMatrix[j+16][i] == 'o') {
                arr[j] = 1;
            } else {
                arr[j] = 0;
            }
        }

        int temp;
        for(k = 0; k<4; k++){
            temp = arr[k];
            arr[k] = arr[8-k-1];
            arr[8-k-1] = temp;
        }

        int size = 8;
        int binary = 0;
        int base = 1;
        for(k = size-1; k >= 0; k--) {
            binary += arr[k] * base;
            base *= 2;
        }
        row[i] = binary;
    }
    return;
}

void setRow4 (uint8_t *row) {
    int i, j, k;
    int arr[8];
    for (i = 0; i < 128; i++) {
        for (j = 0; j <= 7; j++) {
            if (ppMatrix[j+24][i] == 'o') {
                arr[j] = 1;
            } else {
                arr[j] = 0;
            }
        }

        int temp;
        for(k = 0; k<4; k++){
            temp = arr[k];
            arr[k] = arr[8-k-1];
            arr[8-k-1] = temp;
        }

        int size = 8;
        int binary = 0;
        int base = 1;
        for(k = size-1; k >= 0; k--) {
            binary += arr[k] * base;
            base *= 2;
        }
        row[i] = binary;
    }
    return;
}