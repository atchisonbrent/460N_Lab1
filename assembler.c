/*
 Name 1: Brent Atchison
 Name 2: Kyle Garza
 UTEID 1: bma862
 UTEID 2: kcg568
*/

#include <stdio.h>  /* standard input/output library */
#include <stdlib.h> /* Standard C Library */
#include <string.h> /* String operations library */
#include <ctype.h>  /* Library for useful character operations */
#include <limits.h> /* Library for definitions of common variable type characteristics */

#define MAX_LINE_LENGTH 255
#define MAX_LABEL_LEN 20
#define MAX_SYMBOLS 255

FILE* infile = NULL;
FILE* outfile = NULL;

enum {
    DONE,
    OK,
    EMPTY_LINE
};

/* Symbol Table Struct */
typedef struct {
	char* label;
    int address;
} TableEntry;
TableEntry symbolTable[MAX_SYMBOLS];

int num_ops = 28;
const char * instructions[28] = {
    "add",      // 0001
    "and",      // 1010
    "br",       // 0000
    "brn",      // 0000
    "brz",      // 0000
    "brp",      // 0000
    "brnz",     // 0000
    "brnp",     // 0000
    "brzp",     // 0000
    "brnzp",    // 0000
    "jmp",      // 1100
    "ret",      // 1100
    "jsr",      // 0100
    "jsrr",     // 0100
    "ldb",      // 0010
    "ldw",      // 0110
    "lea",      // 1110
    "rti",      // 1000
    "lshf",     // 1101
    "rshfl",    // 1101
    "rshfa",    // 1101
    "stb",      // 0011
    "stw",      // 0111
    "trap",     // 1111
    "xor",      // 1001
    "not",      // 1001
    "nop",      // No-op
	"halt"		// Trap x25
};


/* List of Valid Registers */
int num_regs = 8;
const char * registers[8] = {
	"r0", "r1", "r2",
	"r3", "r4", "r5",
	"r6", "r7"
};


/* List of Register Bit Mappings */
const char * register_map[8] = {
	"000", "001", "010",
	"011", "100", "101",
	"110", "111"
};


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


/* Convert Decimal to Binary */
int toBinary(int i) {
	if (i == 0) { return 0; }
	else { return (i % 2 + 10 * toBinary(i / 2)); }
}


/* Convert 16 Bits to Hex
 * Returns result in form of "0xABCD" */
char* toHex(char* bits) {
	char result[7], hex[5];
	strcpy(result, "0x");
    if (bits[0] == '0' && bits[1] == '0' && bits[2] == '0' &&
        bits[3] == '0') { strcat(result, "0"); }
	int val = (int) strtol(bits, NULL, 2);
	sprintf(hex, "%x", val);
	strcat(result, hex);
	char *str = (char *)malloc(sizeof(char) * 3);
	for (int i = 0; i < strlen(result) + 1; i++) { str[i] = result[i]; }	/* Convert to char* */
	return str;
}


/* Check if Valid Opcode */
int isOpcode(char* op) {
	for (int i = 0; i < num_ops; i++)
		if (strcmp(op, instructions[i]) == 0) { return 1; }
    return -1;
}


/* Check if Valid Register */
int isRegister(char* r) {
	for (int i = 0; i < num_regs; i++)
		if (strcmp(r, registers[i]) == 0) { return 1; }
	return -1;
}


/* Map Register to Binary Value */
const char* mapRegister(char* r) {
	for (int i = 0; i < num_regs; i++)
		if (strcmp(r, registers[i]) == 0)
			return register_map[i];
	return register_map[0];
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


/* Main Method */
int main(int argc, char* argv[]) {
    
    /* Parse Command Line Arguments */
	char * prgName = NULL;
	char * iFileName = NULL;
	char * oFileName = NULL;

	prgName = argv[0];
	iFileName = argv[1];
	oFileName = argv[2];

	printf("program name = '%s'\n", prgName);
	printf("input file name = '%s'\n", iFileName);
	printf("output file name = '%s'\n", oFileName);

    /* Open Assembly File */
    FILE * lInfile;
    lInfile = fopen(iFileName, "r");

	/* Open Output File */
	FILE * lOutfile;
	lOutfile = fopen(oFileName, "w");
    
    /* Pass 1: Fill out symbol table */
    char lLine[MAX_LINE_LENGTH + 1], *lLabel, *lOpcode, *lArg1, *lArg2, *lArg3, *lArg4;
	char labelArray[MAX_SYMBOLS][MAX_LABEL_LEN];
    int lRet, pc = 0;
	int origSet = 0, symTableIndex = 0;
    do {
        lRet = readAndParse(lInfile, lLine, &lLabel, &lOpcode, &lArg1, &lArg2, &lArg3, &lArg4);
        if(lRet != DONE && lRet != EMPTY_LINE) {

			/* Set Origin */
			if (strcmp(lLine, ".orig") == 0) {
				if (origSet == 0) {
					pc = toNum(lArg1) - 2;
					origSet = 1;
				}
				else { exit(4); }			/* .ORIG appears more than once */
			}
			if (pc % 2 != 0) { exit(3); }	/* Invalid constant if pc % 2 is odd */

			/* Check if Label */
			if (isOpcode(lLine) == -1 && lLine[0] != '.') {
				/* Copy label and address to symbolTable */
				symbolTable[symTableIndex].label = strcpy(labelArray[symTableIndex], lLabel);
				symbolTable[symTableIndex].address = pc;
				symTableIndex++;
			}

			/* Increment PC */
			pc += 2;
        }
    } while(lRet != DONE);
    
    // TODO Pass 2: Parse and convert instructions to machine code
	lInfile = fopen(iFileName, "r");
	pc = 0; origSet = 0; symTableIndex = 0;
	do {
		lRet = readAndParse(lInfile, lLine, &lLabel, &lOpcode, &lArg1, &lArg2, &lArg3, &lArg4);
		if (lRet != DONE && lRet != EMPTY_LINE) {

			/* Set Origin */
			if (strcmp(lLine, ".orig") == 0) {
				if (origSet == 0) {
					pc = toNum(lArg1) - 2;
					origSet = 1;
                    continue;
				}
				else { exit(4); }			/* .ORIG appears more than once */
			}
			if (pc % 2 != 0) { exit(3); }	/* Invalid constant if pc % 2 is odd */
            
            /* Check if Invalid Opcode */
            if (*lLabel != '\0' && isOpcode(lOpcode) == -1 && lOpcode[0] != '.') { exit(2); }

			/* If valid opcode, produce binary */
			if (isOpcode(lOpcode) == 1) {
				if (strcmp(lOpcode, "add") == 0) {
					char str[17];
					strcpy(str, "0001");
					/* Check if valid registers and set binary values if so */
					if (isRegister(lArg1) == 1 && isRegister(lArg2) == 1) {
						strcat(str, mapRegister(lArg1));
						strcat(str, mapRegister(lArg2));
					} else { exit(4); }
					if (isRegister(lArg3) == 1) {
						strcat(str, "000");
						strcat(str, mapRegister(lArg3));
					}
					else {
						strcat(str, "1");
						char imm[6];
						int num = toNum(lArg3);
						if (num > 15 || num < -16) { exit(3); }
						int n = sprintf(imm, "%d", abs(toBinary(num)));
						int fill = 5 - n;								/* Number of 0s needed to fill gap in vector */
						for (; fill > 0; fill--) { strcat(str, "0"); }	/* Fill in excess 0s */
						strcat(str, imm);
					}
					fputs(toHex(str), lOutfile);
					fputs("\n", lOutfile);
				}
				else if (strcmp(lOpcode, "and") == 0) {
					char str[17];
					strcpy(str, "0101");
					/* Check if valid registers and set binary values if so */
					if (isRegister(lArg1) == 1 && isRegister(lArg2) == 1) {
						strcat(str, mapRegister(lArg1));
						strcat(str, mapRegister(lArg2));
					}
					else { exit(4); }
					if (isRegister(lArg3) == 1) {
						strcat(str, "000");
						strcat(str, mapRegister(lArg3));
					}
					else {
						strcat(str, "1");
						char imm[6];
						int num = toNum(lArg3);
						if (num > 15 || num < -16) { exit(3); }
						int n = sprintf(imm, "%d", abs(toBinary(num)));
						int fill = 5 - n;								/* Number of 0s needed to fill gap in vector */
						for (; fill > 0; fill--) { strcat(str, "0"); }	/* Fill in excess 0s */
						strcat(str, imm);
					}
					fputs(toHex(str), lOutfile);
					fputs("\n", lOutfile);
				}
				else if (strcmp(lOpcode, "br") == 0) {
					char str[17], imm[10];
					strcpy(str, "0000111");
					int address = 0;
                    for (int j = 0; j < MAX_SYMBOLS; j++) {
						if (strcmp(lArg1, symbolTable[j].label) == 0) {
                            address = symbolTable[j].address;
                            break;
                        }
                        if (j == MAX_SYMBOLS - 1) { exit(1); }
                    }
					int offset = (address - pc - 2) / 2;
					int n = sprintf(imm, "%d", abs(toBinary(offset)));
					int fill = 9 - n;								/* Number of 0s needed to fill gap in vector */
					for (; fill > 0; fill--) { strcat(str, "0"); }	/* Fill in excess 0s */
					strcat(str, imm);
					fputs(toHex(str), lOutfile);
					fputs("\n", lOutfile);
				}
				else if (strcmp(lOpcode, "brn") == 0) {
                    char str[17], imm[10];
                    strcpy(str, "0000100");
                    int address = 0;
                    for (int j = 0; j < MAX_SYMBOLS; j++) {
                        if (strcmp(lArg1, symbolTable[j].label) == 0) {
                            address = symbolTable[j].address;
                            break;
                        }
                        if (j == MAX_SYMBOLS - 1) { exit(1); }
                    }
                    int offset = (address - pc - 2) / 2;
                    int n = sprintf(imm, "%d", abs(toBinary(offset)));
                    int fill = 9 - n;                               /* Number of 0s needed to fill gap in vector */
                    for (; fill > 0; fill--) { strcat(str, "0"); }  /* Fill in excess 0s */
                    strcat(str, imm);
                    fputs(toHex(str), lOutfile);
                    fputs("\n", lOutfile);
				}
				else if (strcmp(lOpcode, "brz") == 0) {
                    char str[17], imm[10];
                    strcpy(str, "0000010");
                    int address = 0;
                    for (int j = 0; j < MAX_SYMBOLS; j++) {
                        if (strcmp(lArg1, symbolTable[j].label) == 0) {
                            address = symbolTable[j].address;
                            break;
                        }
                        if (j == MAX_SYMBOLS - 1) { exit(1); }
                    }
                    int offset = (address - pc - 2) / 2;
                    int n = sprintf(imm, "%d", abs(toBinary(offset)));
                    int fill = 9 - n;                               /* Number of 0s needed to fill gap in vector */
                    for (; fill > 0; fill--) { strcat(str, "0"); }  /* Fill in excess 0s */
                    strcat(str, imm);
                    fputs(toHex(str), lOutfile);
                    fputs("\n", lOutfile);
				}
				else if (strcmp(lOpcode, "brp") == 0) {
                    char str[17], imm[10];
                    strcpy(str, "0000001");
                    int address = 0;
                    for (int j = 0; j < MAX_SYMBOLS; j++) {
                        if (strcmp(lArg1, symbolTable[j].label) == 0) {
                            address = symbolTable[j].address;
                            break;
                        }
                        if (j == MAX_SYMBOLS - 1) { exit(1); }
                    }
                    int offset = (address - pc - 2) / 2;
                    int n = sprintf(imm, "%d", abs(toBinary(offset)));
                    int fill = 9 - n;                               /* Number of 0s needed to fill gap in vector */
                    for (; fill > 0; fill--) { strcat(str, "0"); }  /* Fill in excess 0s */
                    strcat(str, imm);
                    fputs(toHex(str), lOutfile);
                    fputs("\n", lOutfile);
				}
				else if (strcmp(lOpcode, "brnz") == 0) {
                    char str[17], imm[10];
                    strcpy(str, "0000110");
                    int address = 0;
                    for (int j = 0; j < MAX_SYMBOLS; j++) {
                        if (strcmp(lArg1, symbolTable[j].label) == 0) {
                            address = symbolTable[j].address;
                            break;
                        }
                        if (j == MAX_SYMBOLS - 1) { exit(1); }
                    }
                    int offset = (address - pc - 2) / 2;
                    int n = sprintf(imm, "%d", abs(toBinary(offset)));
                    int fill = 9 - n;                               /* Number of 0s needed to fill gap in vector */
                    for (; fill > 0; fill--) { strcat(str, "0"); }  /* Fill in excess 0s */
                    strcat(str, imm);
                    fputs(toHex(str), lOutfile);
                    fputs("\n", lOutfile);
				}
				else if (strcmp(lOpcode, "brnp") == 0) {
                    char str[17], imm[10];
                    strcpy(str, "0000101");
                    int address = 0;
                    for (int j = 0; j < MAX_SYMBOLS; j++) {
                        if (strcmp(lArg1, symbolTable[j].label) == 0) {
                            address = symbolTable[j].address;
                            break;
                        }
                        if (j == MAX_SYMBOLS - 1) { exit(1); }
                    }
                    int offset = (address - pc - 2) / 2;
                    int n = sprintf(imm, "%d", abs(toBinary(offset)));
                    int fill = 9 - n;                               /* Number of 0s needed to fill gap in vector */
                    for (; fill > 0; fill--) { strcat(str, "0"); }  /* Fill in excess 0s */
                    strcat(str, imm);
                    fputs(toHex(str), lOutfile);
                    fputs("\n", lOutfile);
				}
				else if (strcmp(lOpcode, "brzp") == 0) {
                    char str[17], imm[10];
                    strcpy(str, "0000011");
                    int address = 0;
                    for (int j = 0; j < MAX_SYMBOLS; j++) {
                        if (strcmp(lArg1, symbolTable[j].label) == 0) {
                            address = symbolTable[j].address;
                            break;
                        }
                        if (j == MAX_SYMBOLS - 1) { exit(1); }
                    }
                    int offset = (address - pc - 2) / 2;
                    int n = sprintf(imm, "%d", abs(toBinary(offset)));
                    int fill = 9 - n;                               /* Number of 0s needed to fill gap in vector */
                    for (; fill > 0; fill--) { strcat(str, "0"); }  /* Fill in excess 0s */
                    strcat(str, imm);
                    fputs(toHex(str), lOutfile);
                    fputs("\n", lOutfile);
				}
				else if (strcmp(lOpcode, "brnzp") == 0) {
                    char str[17], imm[10];
                    strcpy(str, "0000111");
                    int address = 0;
                    for (int j = 0; j < MAX_SYMBOLS; j++) {
                        if (strcmp(lArg1, symbolTable[j].label) == 0) {
                            address = symbolTable[j].address;
                            break;
                        }
                        if (j == MAX_SYMBOLS - 1) { exit(1); }
                    }
                    int offset = (address - pc - 2) / 2;
                    int n = sprintf(imm, "%d", abs(toBinary(offset)));
                    int fill = 9 - n;                               /* Number of 0s needed to fill gap in vector */
                    for (; fill > 0; fill--) { strcat(str, "0"); }  /* Fill in excess 0s */
                    strcat(str, imm);
                    fputs(toHex(str), lOutfile);
                    fputs("\n", lOutfile);
				}
                else if (strcmp(lOpcode, "jmp") == 0) {
                    char str[17];
                    strcpy(str, "1100000");
                    /* Check if valid register and set binary values if so */
                    if (isRegister(lArg1) == 1) { strcat(str, mapRegister(lArg1)); }
                    else { exit(4); }
                    strcat(str, "000000");
                    fputs(toHex(str), lOutfile);
                    fputs("\n", lOutfile);
                }
                else if (strcmp(lOpcode, "ret") == 0) {
                    fputs("0xc1c0", lOutfile);
                    fputs("\n", lOutfile);
                }
                else if (strcmp(lOpcode, "jsr") == 0) {
                    char str[17], imm[12];
                    strcpy(str, "01001");
                    int address = 0;
                    for (int j = 0; j < MAX_SYMBOLS; j++) {
                        if (strcmp(lArg1, symbolTable[j].label) == 0) {
                            address = symbolTable[j].address;
                            break;
                        }
                        if (j == MAX_SYMBOLS - 1) { exit(1); }
                    }
                    int offset = (address - pc - 2) / 2;
                    int n = sprintf(imm, "%d", abs(toBinary(offset)));
                    int fill = 11 - n;                              /* Number of 0s needed to fill gap in vector */
                    for (; fill > 0; fill--) { strcat(str, "0"); }  /* Fill in excess 0s */
                    strcat(str, imm);
                    fputs(toHex(str), lOutfile);
                    fputs("\n", lOutfile);
                }
                else if (strcmp(lOpcode, "jsrr") == 0) {
                    char str[17];
                    strcpy(str, "0100000");
                    if (isRegister(lArg1) == 1) { strcat(str, mapRegister(lArg1)); }
                    else { exit(4); }
                    fputs(toHex(str), lOutfile);
                    fputs("\n", lOutfile);
                }
				else if (strcmp(lOpcode, "ldb") == 0) {
					char str[17];
					strcpy(str, "0010");
					/* Check if valid registers and set binary values if so */
					if (isRegister(lArg1) == 1 && isRegister(lArg2) == 1) {
						strcat(str, mapRegister(lArg1));
						strcat(str, mapRegister(lArg2));
					}
					else { exit(4); }
					char imm[7];
					int num = toNum(lArg3);
					if (num > 30 || num < -31) { exit(3); }
					int n = sprintf(imm, "%d", abs(toBinary(num)));
					int fill = 6 - n;								/* Number of 0s needed to fill gap in vector */
					for (; fill > 0; fill--) { strcat(str, "0"); }	/* Fill in excess 0s */
					strcat(str, imm);
					fputs(toHex(str), lOutfile);
					fputs("\n", lOutfile);
				}
				else if (strcmp(lOpcode, "ldw") == 0) {
					char str[17];
					strcpy(str, "0110");
					/* Check if valid registers and set binary values if so */
					if (isRegister(lArg1) == 1 && isRegister(lArg2) == 1) {
						strcat(str, mapRegister(lArg1));
						strcat(str, mapRegister(lArg2));
					}
					else { exit(4); }
					char imm[7];
					int num = toNum(lArg3);
					if (num > 30 || num < -31) { exit(3); }
					int n = sprintf(imm, "%d", abs(toBinary(num)));
					int fill = 6 - n;								/* Number of 0s needed to fill gap in vector */
					for (; fill > 0; fill--) { strcat(str, "0"); }	/* Fill in excess 0s */
					strcat(str, imm);
					fputs(toHex(str), lOutfile);
					fputs("\n", lOutfile);
				}
				else if (strcmp(lOpcode, "lea") == 0) {
                    char str[17], imm[10];
                    strcpy(str, "1110");
                    
                    /* Check Register */
                    if (isRegister(lArg1) == 1) { strcat(str, mapRegister(lArg1)); }
                    else { exit(4); }
                    
                    /* Check Label */
                    int address = 0;
                    for (int j = 0; j < MAX_SYMBOLS; j++) {
                        if (strcmp(lArg2, symbolTable[j].label) == 0) {
                            address = symbolTable[j].address;
                            break;
                        }
                        if (j == MAX_SYMBOLS - 1) { exit(1); }
                    }
                    
                    int offset = (address - pc - 2) / 2;
                    int n = sprintf(imm, "%d", abs(toBinary(offset)));
                    int fill = 9 - n;                               /* Number of 0s needed to fill gap in vector */
                    for (; fill > 0; fill--) { strcat(str, "0"); }  /* Fill in excess 0s */
                    strcat(str, imm);
                    fputs(toHex(str), lOutfile);
                    fputs("\n", lOutfile);
				}
				else if (strcmp(lOpcode, "rti") == 0) {
					fputs("0x8000", lOutfile);
					fputs("\n", lOutfile);
				}
                else if (strcmp(lOpcode, "lshf") == 0) {
                    char str[17];
                    strcpy(str, "1101");
                    /* Check if valid registers and set binary values if so */
                    if (isRegister(lArg1) == 1 && isRegister(lArg2) == 1) {
                        strcat(str, mapRegister(lArg1));
                        strcat(str, mapRegister(lArg2));
                    }
                    else { exit(4); }
                    strcat(str, "00");
                    char imm[5];
                    int num = toNum(lArg3);
                    if (num > 7 || num < -8) { exit(3); }
                    int n = sprintf(imm, "%d", abs(toBinary(num)));
                    int fill = 4 - n;                                /* Number of 0s needed to fill gap in vector */
                    for (; fill > 0; fill--) { strcat(str, "0"); }    /* Fill in excess 0s */
                    strcat(str, imm);
                    fputs(toHex(str), lOutfile);
                    fputs("\n", lOutfile);
                }
                else if (strcmp(lOpcode, "rshfl") == 0) {
                    char str[17];
                    strcpy(str, "1101");
                    /* Check if valid registers and set binary values if so */
                    if (isRegister(lArg1) == 1 && isRegister(lArg2) == 1) {
                        strcat(str, mapRegister(lArg1));
                        strcat(str, mapRegister(lArg2));
                    }
                    else { exit(4); }
                    strcat(str, "01");
                    char imm[5];
                    int num = toNum(lArg3);
                    if (num > 7 || num < -8) { exit(3); }
                    int n = sprintf(imm, "%d", abs(toBinary(num)));
                    int fill = 4 - n;                                /* Number of 0s needed to fill gap in vector */
                    for (; fill > 0; fill--) { strcat(str, "0"); }    /* Fill in excess 0s */
                    strcat(str, imm);
                    fputs(toHex(str), lOutfile);
                    fputs("\n", lOutfile);
                }
                else if (strcmp(lOpcode, "rshfa") == 0) {
                    char str[17];
                    strcpy(str, "1101");
                    /* Check if valid registers and set binary values if so */
                    if (isRegister(lArg1) == 1 && isRegister(lArg2) == 1) {
                        strcat(str, mapRegister(lArg1));
                        strcat(str, mapRegister(lArg2));
                    }
                    else { exit(4); }
                    strcat(str, "11");
                    char imm[5];
                    int num = toNum(lArg3);
                    if (num > 7 || num < -8) { exit(3); }
                    int n = sprintf(imm, "%d", abs(toBinary(num)));
                    int fill = 4 - n;                                /* Number of 0s needed to fill gap in vector */
                    for (; fill > 0; fill--) { strcat(str, "0"); }    /* Fill in excess 0s */
                    strcat(str, imm);
                    fputs(toHex(str), lOutfile);
                    fputs("\n", lOutfile);
                }
				else if (strcmp(lOpcode, "stb") == 0) {
					char str[17];
					strcpy(str, "0011");
					/* Check if valid registers and set binary values if so */
					if (isRegister(lArg1) == 1 && isRegister(lArg2) == 1) {
						strcat(str, mapRegister(lArg1));
						strcat(str, mapRegister(lArg2));
					}
					else { exit(4); }
					char imm[7];
					int num = toNum(lArg3);
					if (num > 30 || num < -31) { exit(3); }
					int n = sprintf(imm, "%d", abs(toBinary(num)));
					int fill = 6 - n;								/* Number of 0s needed to fill gap in vector */
					for (; fill > 0; fill--) { strcat(str, "0"); }	/* Fill in excess 0s */
					strcat(str, imm);
					fputs(toHex(str), lOutfile);
					fputs("\n", lOutfile);
				}
				else if (strcmp(lOpcode, "stw") == 0) {
					char str[17];
					strcpy(str, "0111");
					/* Check if valid registers and set binary values if so */
					if (isRegister(lArg1) == 1 && isRegister(lArg2) == 1) {
						strcat(str, mapRegister(lArg1));
						strcat(str, mapRegister(lArg2));
					}
					else { exit(4); }
					char imm[7];
					int num = toNum(lArg3);
					if (num > 30 || num < -31) { exit(3); }
					int n = sprintf(imm, "%d", abs(toBinary(num)));
					int fill = 6 - n;								/* Number of 0s needed to fill gap in vector */
					for (; fill > 0; fill--) { strcat(str, "0"); }	/* Fill in excess 0s */
					strcat(str, imm);
					fputs(toHex(str), lOutfile);
					fputs("\n", lOutfile);
				}
				else if (strcmp(lOpcode, "trap") == 0) {
					if (toNum(lArg1) != 37) { exit(3); }			/* Check for trap vector x25 */
					char vector[8], str[17];
					int bin = toBinary(toNum(lArg1));
					int n = sprintf(vector, "%d", bin);				/* Binary -> string */
					int fill = 8 - n;								/* Number of 0s needed to fill gap in vector */
					strcpy(str, "11110000");
					for (; fill > 0; fill--) { strcat(str, "0"); }	/* Fill in excess 0s */
					strcat(str, vector);
					fputs(toHex(str), lOutfile);
					fputs("\n", lOutfile);
				}
                else if (strcmp(lOpcode, "xor") == 0) {
                    char str[17];
                    strcpy(str, "1001");
                    /* Check if valid registers and set binary values if so */
                    if (isRegister(lArg1) == 1 && isRegister(lArg2) == 1) {
                        strcat(str, mapRegister(lArg1));
                        strcat(str, mapRegister(lArg2));
                    }
                    else { exit(4); }
                    if (isRegister(lArg3) == 1) {
                        strcat(str, "000");
                        strcat(str, mapRegister(lArg3));
                    }
                    else {
                        strcat(str, "1");
                        char imm[6];
                        int num = toNum(lArg3);
                        if (num > 15 || num < -16) { exit(3); }
                        int n = sprintf(imm, "%d", abs(toBinary(num)));
                        int fill = 5 - n;                                /* Number of 0s needed to fill gap in vector */
                        for (; fill > 0; fill--) { strcat(str, "0"); }    /* Fill in excess 0s */
                        strcat(str, imm);
                    }
                    fputs(toHex(str), lOutfile);
                    fputs("\n", lOutfile);
                }
                else if (strcmp(lOpcode, "not") == 0) {
                    char str[17];
                    strcpy(str, "1001");
                    /* Check if valid registers and set binary values if so */
                    if (isRegister(lArg1) == 1 && isRegister(lArg2) == 1) {
                        strcat(str, mapRegister(lArg1));
                        strcat(str, mapRegister(lArg2));
                    }
                    else { exit(4); }
                    strcat(str, "111111");
                    fputs(toHex(str), lOutfile);
                    fputs("\n", lOutfile);
                }
				else if (strcmp(lOpcode, "nop") == 0) {
					fputs("0x0000", lOutfile);
					fputs("\n", lOutfile);
				}
				else if (strcmp(lOpcode, "halt") == 0) {
					fputs("0xf025", lOutfile);
					fputs("\n", lOutfile);
				}
				else {
					printf("Congrats, you broke the program.");
					exit(4);
				}
			}

			/* Increment PC */
			pc += 2;
		}
	} while (lRet != DONE);
    
    return 0;
}
