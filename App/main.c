/**************************************************************************//**
 * @file     main.c
 * @version  
 * @brief    
 *
 ******************************************************************************/
//#include <stdio.h>

#include "portable.h"

void main() {
    vSystemInit();
    //printf("\n");
    //printf("+----------------------------------------------+\n");
    //printf("|              Simple Blinky Demo              |\n");
    //printf("+----------------------------------------------+\n");
    while(1) {
        vSystemTask();
    }
}
