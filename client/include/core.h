#ifndef CORE_H
#define CORE_H

#include "../../shared/include/consts.h"

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <ncursesw/curses.h>
// #include <ncurses.h>
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

void exitprog();
double wtime();
struct plaerstr **createstr (int);
void hdl (int);

extern char username[MAX_USERNAME];
extern char state;

extern pthread_t tid[2];
extern int sockfd;

#endif
