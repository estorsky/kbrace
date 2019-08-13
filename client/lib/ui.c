#include "../include/ui.h"

#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#define SWORDMAX 15 //maximum word size
#define SNAMEMAX 10 //maximum name size
#define SLEEPTIME 2

#define MODOFF 0
#define MODINIT 1
#define MODRUN 2
#define MODBATLLE 3
#define MODFINISH 4

#define SLMAX LINES - 2
#define SCMAX COLS - 2
#define SLSTAT 1
#define SLENTRY 1

WINDOW *win_stat;
WINDOW *win_text;
WINDOW *win_entry;
WINDOW *win_prog;
char entrybuffer[SWORDMAX];
int uimod = MODOFF;

pthread_mutex_t ncur = PTHREAD_MUTEX_INITIALIZER;

void uiInit(){
    pthread_mutex_lock(&ncur);
    if (uimod != MODOFF)
        return;
    initscr();
    cbreak();
    noecho();
    refresh();
    keypad(stdscr, true);
    uimod = MODINIT;
    pthread_mutex_unlock(&ncur);
}

void uiRun(){
    pthread_mutex_lock(&ncur);
    if (uimod != MODINIT && uimod != MODFINISH)
        return;
    if (uimod == MODFINISH){
        delwin(win_stat);
        delwin(win_prog);
        clear();
    }
    box(stdscr, 0, 0);
    win_stat = newwin(7, 8, LINES/2-4, COLS/2-4);
    wprintw(win_stat,"########");
    wprintw(win_stat,"#      #");
    wprintw(win_stat," #    # ");
    wprintw(win_stat,"  #  #  ");
    wprintw(win_stat," #    # ");
    wprintw(win_stat,"#      #");
    wprintw(win_stat,"########");
    refresh();
    wrefresh(win_stat);
    uimod = MODRUN;
    pthread_mutex_unlock(&ncur);
}

int textprint(char text[][SWORDMAX], int printoff, int lowline){
    int i = 0, j = 0, sltext = 1, textlim = SCMAX;
    if (printoff == 0)
        wclear(win_text);
    while(text[i][0] != '\0'){
        textlim -= strlen(text[i]);
        if (textlim <= 0){
            if (printoff == 0 && textlim < 0)
                waddch(win_text, '\n');
            textlim = SCMAX - strlen(text[i]);
            sltext++;
        }
        if (printoff == 0){
            if (lowline == i)
                wattron(win_text, A_UNDERLINE);
            while(text[i][j] != ' ' && text[i][j] != '\0'){
                waddch(win_text, text[i][j]);
                j++;
            }
            if (lowline == i)
                wattroff(win_text, A_UNDERLINE);
            waddch(win_text, ' ');
            j = 0;
        }
        i++;
    }
    if (printoff == 0)
        wrefresh(win_text);
    return sltext;
}

void uiStartBattle(char text[][SWORDMAX]){
    pthread_mutex_lock(&ncur);
    if (uimod != MODRUN)
        return;
    delwin(win_stat);
    clear();

    box(stdscr, 0, 0);

    win_stat = newwin(SLSTAT, SCMAX, 1, 1);

    mvhline(SLSTAT + 1, 1, 0, SCMAX);

    win_text = newwin(SLMAX - SLSTAT - SLENTRY - 2, SCMAX, SLSTAT + 2, 1);
    int sltext = textprint(text, 1, 0);
    sltext += 2;
    wresize(win_text, sltext, SCMAX);

    mvhline(SLSTAT + sltext + 2, 1, 0, SCMAX);

    win_entry = newwin(SLENTRY, SCMAX, SLSTAT + sltext + 3, 1);

    int slprog = SLMAX - SLSTAT - SLENTRY - sltext - 3;
    if (slprog > 0){
        mvhline(SLSTAT + SLENTRY + sltext + 3, 1, 0, SCMAX);
        win_prog = newwin(slprog, SCMAX, SLSTAT + SLENTRY + sltext + 4, 1);
    }

    refresh();
    wrefresh(win_text);
    wrefresh(win_entry);

    wprintw(win_text,"######  \n");
    wprintw(win_text,"      ##\n");
    wprintw(win_text,"  ####  \n");
    wprintw(win_text,"      ##\n");
    wprintw(win_text,"######  \n");
    wrefresh(win_text);
    wrefresh(win_entry);
    sleep(SLEEPTIME);
    wclear(win_text);
    wprintw(win_text,"  ####  \n");
    wprintw(win_text,"##    ##\n");
    wprintw(win_text,"    ##  \n");
    wprintw(win_text,"  ##    \n");
    wprintw(win_text,"########\n");
    wrefresh(win_text);
    wrefresh(win_entry);
    sleep(SLEEPTIME);
    wclear(win_text);
    wprintw(win_text,"      ##\n");
    wprintw(win_text,"    ####\n");
    wprintw(win_text,"      ##\n");
    wprintw(win_text,"      ##\n");
    wprintw(win_text,"      ##\n");
    wrefresh(win_text);
    wrefresh(win_entry);
    sleep(SLEEPTIME);
    wclear(win_text);
    wrefresh(win_text);
    wrefresh(win_entry);
    uimod = MODBATLLE;
    pthread_mutex_unlock(&ncur);
}

void uiTextLowline(char text[][SWORDMAX], int n){
    pthread_mutex_lock(&ncur);
    if (uimod != MODBATLLE)
        return;
    textprint(text, 0, n);
    wrefresh(win_entry);
    pthread_mutex_unlock(&ncur);
}

void uiFinishBattle(){
    pthread_mutex_lock(&ncur);
    if (uimod != MODBATLLE)
        return;
    delwin(win_text);
    delwin(win_entry);
    delwin(win_prog);
    win_prog = newwin(SLMAX - SLSTAT - 1, SCMAX, SLSTAT + 2, 1);
    wrefresh(win_prog);
    uimod = MODFINISH;
    pthread_mutex_unlock(&ncur);
}

void uiStatPrint(int speed, int miss, double time){
    pthread_mutex_lock(&ncur);
    if (uimod != MODBATLLE && uimod != MODFINISH)
        return;
    wclear(win_stat);
    wprintw(win_stat, "SPEED %3d   MISS %3d   TIME %3.2f", speed, miss, time);
    wrefresh(win_stat);
    if (uimod == MODBATLLE)
        wrefresh(win_entry);
    pthread_mutex_unlock(&ncur);
}

void entryprint(int i){
    wclear(win_entry);
    if (i == 1)
        wattron(win_entry, A_STANDOUT);
    else
        wattroff(win_entry, A_STANDOUT);
    waddstr(win_entry, entrybuffer);
    wrefresh(win_entry);
}

void uiEntryPrint(char a, int i){
    pthread_mutex_lock(&ncur);
    if (uimod != MODBATLLE)
        return;
    entrybuffer[strlen(entrybuffer)] = a;
    entrybuffer[strlen(entrybuffer)+1] = '\0';
    entryprint(i);
    pthread_mutex_unlock(&ncur);
}

void uiEntryBack(int i){
    pthread_mutex_lock(&ncur);
    if (uimod != MODBATLLE)
        return;
    entrybuffer[strlen(entrybuffer)-1] = '\0';
    entryprint(i);
    pthread_mutex_unlock(&ncur);
}

void uiEntryClear(){
    pthread_mutex_lock(&ncur);
    if (uimod != MODBATLLE)
        return;
    memset(entrybuffer, 0, SWORDMAX);
    wclear(win_entry);
    wrefresh(win_entry);
    pthread_mutex_unlock(&ncur);
}

void uiProgPrint(struct plaerstr **p, int n){
    pthread_mutex_lock(&ncur);
    if (uimod != MODBATLLE && uimod != MODFINISH)
        return;
    wclear(win_prog);
    int i;
    for (i = 0; i < n; i++){
        if (p[i]->name[0] != '\n'){
            mvwprintw(win_prog, i, 0, "%s ", p[i]->name);
            if (p[i]->prog == 100)
                mvwprintw(win_prog, i, SNAMEMAX, "SPEED %3d   MISS %3d  TIME %3.2f", p[i]->speed, p[i]->miss, p[i]->time);
            else{
                mvwprintw(win_prog, i, SNAMEMAX, "[");
                mvwhline(win_prog, i, SNAMEMAX+1, '=', (int)(((float)SCMAX-SNAMEMAX-7)/100*p[i]->prog));
                mvwprintw(win_prog, i, SCMAX-5, "]");
            }
            mvwprintw(win_prog, i, SCMAX-2, "%c", p[i]->state);
        }
    }
    wrefresh(win_prog);
    if (uimod == MODBATLLE)
        wrefresh(win_entry);
    pthread_mutex_unlock(&ncur);
}

void uiEnd(){
    pthread_mutex_lock(&ncur);
    endwin();
    uimod = MODOFF;
    pthread_mutex_unlock(&ncur);
}