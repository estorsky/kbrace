#ifndef CORE_H
#define CORE_H

#include "../../shared/include/consts.h"

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <ncurses.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <strings.h>
#include <netdb.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <pwd.h>
#include <signal.h>

#define ARROW_D   2
#define ARROW_U   3
#define ARROW_L   4
#define ARROW_R   5
#define BACKSPACE 7
#define ENTER     10
#define F10       18
#define CTRL_U    21
#define ESC       27

void exitprog();
double wtime();
void hdl (int);

extern char username[MAX_USERNAME];
extern char state;

extern pthread_t tid[2];
extern int sockfd;

#endif
