#ifndef CORE_H
#define CORE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define true 1
#define false 0

#define PORT 1337
#define DELAY_WAIT 15
#define MAX_PLAYERS 30
#define MAX_USERNAME 10
#define MAX_WORD_LEN 20
#define MAX_WORDS 100

#define LIM_PACK 900
#define NUM_STRUCTS 100

int getNewText(char [][MAX_WORD_LEN]);

#endif

