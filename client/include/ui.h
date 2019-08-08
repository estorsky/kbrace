#ifndef UI_H
#define UI_H

struct plaerstr{
    char name[10];
    int prog;
    int speed;
    int miss;
    float time;
};

void uiRun();
int uiStartBattle(char [][15]);
void uiTextLowline(char [][15], int);
void uiFinishBattle();
void uiStatPrint(int, int, float);
void uiEntryPrint(char, int);
void uiEntryBack(int);
void uiEntryClear();
void uiProgPrint(struct plaerstr **, int n);
void uiEnd();

#endif
