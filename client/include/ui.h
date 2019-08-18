#ifndef UI_H
#define UI_H

#define SWORDMAX 20 //maximum word size
#define SNAMEMAX 10 //maximum name size
#define SLEEPTIME 2

struct plaerstr{
    char name[10];
    int prog;
    int speed;
    int miss;
    double time;
    char state;
};

void uiInit();
void uiRun();
void uiStartBattle(char [][SWORDMAX]);
void uiTextLowline(char [][SWORDMAX], int);
void uiFinishBattle();
void uiStatPrint(int, int, double, int);
void uiEntryPrint(char, int);
void uiEntryBack(int);
void uiEntryClear();
void uiProgPrint(struct plaerstr **, int n);
void uiHelpPrint(char []);
void uiEnd();

#endif
