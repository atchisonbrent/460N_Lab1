/*
 Name 1: Brent Atchison
 Name 2: Kyle Garza
 UTEID 1: bma862
 UTEID 2: k
*/

#include <stdio.h>  /* standard input/output library */
#include <stdlib.h> /* Standard C Library */
#include <string.h> /* String operations library */
#include <ctype.h>  /* Library for useful character operations */
#include <limits.h> /* Library for definitions of common variable type characteristics */

#define MAX_LINE_LENGTH 255

FILE* infile = NULL;
FILE* outfile = NULL;

enum {
    DONE,
    OK,
    EMPTY_LINE
};

const char * instructions[15] = {
    "ADD",  // 0001
    "AND",  // 1010
    "BR",   // 0000
    "JMP",  // 1100
    "JSR",  // 0100
    "LDB",  // 0010
    "LDW",  // 0110
    "LEA",  // 1110
    "RTI",  // 1000
    "SHF",  // 1101
    "STB",  // 0011
    "STW",  // 0111
    "TRAP", // 1111
    "XOR",  // 1001
    "NOP"   // No-op
};


/* Parse Command Line Arguments */
int parse_args(int argc, char* argv[]) {
    
    char *prgName   = NULL;
    char *iFileName = NULL;
    char *oFileName = NULL;
    
    prgName   = argv[0];
    iFileName = argv[1];
    oFileName = argv[2];
    
    printf("program name = '%s'\n", prgName);
    printf("input file name = '%s'\n", iFileName);
    printf("output file name = '%s'\n", oFileName);
    
    return 0;
}


/* File Handler */
int handle_files(int argc, char* argv[]) {
    
    /* open the source file */
    infile = fopen(argv[1], "r");
    outfile = fopen(argv[2], "w");
    
    if (!infile) {
        printf("Error: Cannot open file %s\n", argv[1]);
        exit(4);
    }
    if (!outfile) {
        printf("Error: Cannot open file %s\n", argv[2]);
        exit(4);
    }
    
    /* Do stuff with files */
    
    fclose(infile);
    fclose(outfile);
    
    return 0;
}


/* Convert String to Number */
int toNum(char* pStr) {
    char * t_ptr;
    char * orig_pStr;
    int t_length,k;
    int lNum, lNeg = 0;
    long int lNumLong;
    
    orig_pStr = pStr;
    if (*pStr == '#') {     /* Decimal */
        pStr++;
        if (*pStr == '-') { /* Decimal is negative */
            lNeg = 1;
            pStr++;
        }
        t_ptr = pStr;
        t_length = strlen(t_ptr);
        for (k=0;k < t_length;k++) {
            if (!isdigit(*t_ptr)) {
                printf("Error: invalid decimal operand, %s\n",orig_pStr);
                exit(4);
            }
            t_ptr++;
        }
        lNum = atoi(pStr);
        if (lNeg) { lNum = -lNum; }
        return lNum;
    }
    else if (*pStr == 'x') { /* Hex */
        pStr++;
        if(*pStr == '-') {  /* Hex is negative */
            lNeg = 1;
            pStr++;
        }
        t_ptr = pStr;
        t_length = strlen(t_ptr);
        for(k=0;k < t_length;k++) {
            if (!isxdigit(*t_ptr)) {
                printf("Error: invalid hex operand, %s\n",orig_pStr);
                exit(4);
            }
            t_ptr++;
        }
        lNumLong = strtol(pStr, NULL, 16);    /* convert hex string into integer */
        lNum = (lNumLong > INT_MAX) ? INT_MAX : lNumLong;
        if(lNeg) { lNum = -lNum; }
        return lNum;
    }
    else {
        printf( "Error: invalid operand, %s\n", orig_pStr);
        exit(4);  /* This has been changed from error code 3 to error code 4, see clarification 12 */
    }
}


/* Check if Valid Opcode */
int isOpcode(char* op) {
    
    return 0;
}


/*
 * Read and Parse a Line of Assembly File
 * Note: MAX_LINE_LENGTH, OK, EMPTY_LINE, and DONE are defined values
 */
int readAndParse(FILE * pInfile, char * pLine, char ** pLabel, char ** pOpcode,
                 char ** pArg1, char ** pArg2, char ** pArg3, char ** pArg4) {
    
    char * lPtr;
    int i;
    if( !fgets( pLine, MAX_LINE_LENGTH, pInfile ) )
        return( DONE );
    for( i = 0; i < strlen( pLine ); i++ )
        pLine[i] = tolower( pLine[i] );
    
    /* convert entire line to lowercase */
    *pLabel = *pOpcode = *pArg1 = *pArg2 = *pArg3 = *pArg4 = pLine + strlen(pLine);
    
    /* ignore the comments */
    lPtr = pLine;
    
    while( *lPtr != ';' && *lPtr != '\0' && *lPtr != '\n' )
        lPtr++;
    
    *lPtr = '\0';
    if( !(lPtr = strtok( pLine, "\t\n ," ) ) )
        return( EMPTY_LINE );
    
    if( isOpcode( lPtr ) == -1 && lPtr[0] != '.' ) {    /* Found a label */
        *pLabel = lPtr;
        if( !( lPtr = strtok( NULL, "\t\n ," ) ) ) return( OK );
    }
    
    *pOpcode = lPtr;
    
    if( !( lPtr = strtok( NULL, "\t\n ," ) ) ) return( OK );
    
    *pArg1 = lPtr;
    
    if( !( lPtr = strtok( NULL, "\t\n ," ) ) ) return( OK );
    
    *pArg2 = lPtr;
    
    if( !( lPtr = strtok( NULL, "\t\n ," ) ) ) return( OK );
    
    *pArg3 = lPtr;
    
    if( !( lPtr = strtok( NULL, "\t\n ," ) ) ) return( OK );
    
    *pArg4 = lPtr;
    
    return( OK );
}

/* Example Usage of Read and Parse */
//func() {
//    char lLine[MAX_LINE_LENGTH + 1], *lLabel, *lOpcode, *lArg1,
//    *lArg2, *lArg3, *lArg4;
//
//    int lRet;
//
//    FILE * lInfile;
//
//    lInfile = fopen( "data.in", "r" );    /* open the input file */
//
//    do {
//        lRet = readAndParse( lInfile, lLine, &lLabel,
//                            &lOpcode, &lArg1, &lArg2, &lArg3, &lArg4 );
//        if( lRet != DONE && lRet != EMPTY_LINE ) {
//            ...
//        }
//    } while( lRet != DONE );
//}

/* Example Output Code */
//FILE * pOutfile;
//pOutfile = fopen( "data.out", "w" );
//
//...
//
//fprintf( pOutfile, "0x%.4X\n", lInstr );    /* where lInstr is declared as an int */


/* Main Method */
int main(int argc, char* argv[]) {
    
    // TODO Open Assembly File
    
    
    // TODO Parse Command Line Arguments
    
    
    // TODO Pass 1: Fill out symbol table
    
    
    // TODO Pass 2: Parse and convert instructions to machine code
    
    
    // TODO Write Output File
    
    
    return 0;
}
