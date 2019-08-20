#ifndef UI_H
#define UI_H

#include "../include/core.h"

struct plaerstr{
    char name[MAX_USERNAME];
    int prog;
    int speed;
    int miss;
    double time;
    char state;
};

void uiInit();
void uiRun();
void uiStartBattle(char [][MAX_WORD_LEN]);
void uiTextLowline(char [][MAX_WORD_LEN], int);
void uiFinishBattle();
void uiStatPrint(int, int, double, int);
void uiEntryPrint(char, int);
void uiEntryBack(int);
void uiEntryClear();
void uiProgPrint(struct plaerstr **, int n);
void uiHelpPrint(char []);
void uiEnd();

#endif
