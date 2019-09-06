#ifndef CONSTS_H
#define CONSTS_H

#define true 1
#define false 0

#define TOKEN 975434872
#define PORT 1337
#define MAX_PLAYERS 30
#define MAX_USERNAME 10
#define MAX_WORD_LEN 20
#define MAX_WORDS 100

struct stat {
    int id;
    char name[MAX_USERNAME];
    int speed;
    int miss;
    double time;
    int prog;
    char state;
};

#endif

