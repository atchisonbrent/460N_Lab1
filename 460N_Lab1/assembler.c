/*
 Name 1: Brent Atchison
 UTEID 1: bma862
*/

#include <stdio.h>  /* standard input/output library */
#include <stdlib.h> /* Standard C Library */
#include <string.h> /* String operations library */
#include <ctype.h>  /* Library for useful character operations */
#include <limits.h> /* Library for definitions of common variable type characteristics */

int main(int argc, char* argv[]) {
    
    char *prgName   = NULL;
    char *iFileName = NULL;
    char *oFileName = NULL;
    
    prgName   = argv[0];
    iFileName = argv[1];
    oFileName = argv[2];
    
    printf("program name = '%s'\n", prgName);
    printf("input file name = '%s'\n", iFileName);
    printf("output file name = '%s'\n", oFileName);
}
