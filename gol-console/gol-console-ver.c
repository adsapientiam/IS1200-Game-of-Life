#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define ROWS 32
#define COLS 128

// Define two char doublepointers. Chars are used as they take up the least space in theory. 
char** ppMatrix;
char** nextMatrix;

// free allocated memory
void free_ppMatrix() {
    for (int r = 0; r < ROWS; r++) {
        free(ppMatrix[r]);
    }
    free(ppMatrix);
}

// Could this have been done with one function? Yes. Did I want to? Nah, too lazy.
void free_nextMatrix() {
    for (int r = 0; r < ROWS; r++) {
        free(nextMatrix[r]);
    }
    free(nextMatrix);
}

// A debug function to print the matrix inside the console
void debug_printMatrix(char** matrix) {
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) printf("%c", matrix[r][c]);
        printf("\n");
    }
    printf("\n");
}

// test function to see how accessing the 2D array works
void test_editElement(int r, int c) {
    ppMatrix[r][c] = 'o';
}

// fill the 2D array with "dead cells". Just something so that it can be printed/initialized
void clearMatrix(char** matrix) {
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) matrix[r][c] = '-';
    }
}

//initializes a matrix based on the chosen state (1,2,3,R)
void initMatrix(int state) {
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
        break;

    case 4: //R
        srand(time(NULL));
        for (int i = 0; i < ROWS; i++) {
            for (int k = 0; k < COLS; k++) {
                char ran;
                if (rand() % 2 == 1) {
                    ran = 'o';
                } else {
                    ran = '-'; 
                }
                ppMatrix[i][k] = ran;
            }
        }
        break;
    }
}

//count all neighbours of a given cell
int countNeighbors(int r, int c) {
    int count = 0;
    for (int i = -1; i < 2; i++) {
        for (int j = -1; j < 2; j++) {
            if (i == 0 && j == 0) continue; //ignore self
            int row = r + i;
            int col = c + j;
            //wraparound checks and adjustments
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
    //Cycle thru all cells
    for (int i = 0; i < ROWS; i++){
        for (int j = 0; j < COLS; j++){
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
    //TEMP: copy contents, looking for a more efficient way to do this later
    for (int i = 0; i < ROWS; i++){
        for (int j = 0; j < COLS; j++){
            ppMatrix[i][j] = nextMatrix[i][j];
        }
    }
    //Clear the "temp" nextMatrix before the next update
    clearMatrix(nextMatrix);
}

int main() {
    // create the 2D array: a double pointer that points to an array of pointers
    // that point to an array of chars. Cool stuff!
    ppMatrix = (char**)malloc(ROWS * sizeof(char*));
    for (int i = 0; i < ROWS; i++) {
        ppMatrix[i] = (char*)malloc(COLS * sizeof(char));
    }
    // Temp matrix used to store the next state. ppMatrix keeps current state,
    // nextMatrix is the canvas for the next generation iteration
    nextMatrix = (char**)malloc(ROWS * sizeof(char*));
    for (int i = 0; i < ROWS; i++) {
        nextMatrix[i] = (char*)malloc(COLS * sizeof(char));
    }

    // "initialize" the temp matrix (fill it with dead cells)
    clearMatrix(nextMatrix);

    //init the ppMatrix with a preset
    //1: gldier, 2: pure glider gen
    //3: UNK, 4: (R:) Random
    initMatrix(1);

    // Following code is for console debugging. It outputs the matrix onto the console on a PC.
    debug_printMatrix(ppMatrix);
    printf("\n");

    // does 12 generations, prints on each gen
    for (int i = 0; i < 12; i++) {
        updateMatrix();
        debug_printMatrix(ppMatrix);
        printf("\n");
    }

    // once done, free allocated memory
    free_ppMatrix();
    free_nextMatrix();
}