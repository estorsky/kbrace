#ifndef CORE_H
#define CORE_H

#include "../../shared/include/consts.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <strings.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>
// #include <error.h>

#define DELAY_WAIT 20
#define LIM_PACK 900
#define NUM_STRUCTS 100
#define PATH_TO_TEXTS "./server/texts/"

int getNewText(char [][MAX_WORD_LEN]);
const char* curTime();

#endif

