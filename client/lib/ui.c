#include "../include/ui.h"

#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define SWORDMAX 15 //maximum word size
#define SNAMEMAX 10 //maximum name size
#define SLEEPTIME 2

#define MODRUN 0
#define MODBATLLE 1
#define MODFINISH 2

#define SLMAX LINES - 2
#define SCMAX COLS - 2
#define SLSTAT 1
#define SLENTRY 1

WINDOW *win_stat;
WINDOW *win_text;
WINDOW *win_entry;
WINDOW *win_prog;
char entrybuffer[SWORDMAX];
int uimod;

void init(){
	initscr();
    cbreak();
    noecho();
    refresh();
}

void uiRun(){
    init();
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
}

int textprint(char text[][SWORDMAX], int printoff, int lowline){
    int i = 0, j = 0, sltext = 1, textlim = SCMAX;
    if (printoff == 0)
        wclear(win_text);
    while(text[i][0] != '\0'){
        j = 0;
        while(text[i][j] != '\0'){
            textlim--;
            j++;
        }
        if (textlim <= 0){
            if (textlim + j != 0 && printoff == 0) 
                wprintw(win_text, "\n");
            textlim = SCMAX - j;
            sltext++;
        }
        else 
            if (textlim + j != SCMAX){
                if (printoff == 0)
                    wprintw(win_text, " ");
                textlim--;
            }
        if (printoff == 0){
            if (lowline == i){
                wattron(win_text, A_UNDERLINE);
                waddstr(win_text, text[i]);
                wattroff(win_text, A_UNDERLINE);
            }
            else
                waddstr(win_text, text[i]);
        }
        i++;    
    }
    if (printoff == 0)
        wrefresh(win_text);
    return sltext;    
}

int uiStartBattle(char text[][SWORDMAX]){
    if (uimod != MODRUN)
        return 0;
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
    return slprog;
}

void uiTextLowline(char text[][SWORDMAX], int n){
    if (uimod != MODBATLLE)
        return;
    textprint(text, 0, n);
    wrefresh(win_entry);
}

void uiFinishBattle(){
    if (uimod != MODBATLLE)
        return;
    delwin(win_text);
    delwin(win_entry);
    delwin(win_prog);
    win_prog = newwin(SLMAX - SLSTAT - 1, SCMAX, SLSTAT + 2, 1);
    box(win_prog, 0, 0);
    wrefresh(win_prog);
    uimod = MODFINISH;
}

void uiStatPrint(int speed, int miss, float time){
    if (uimod != MODBATLLE && uimod != MODFINISH)
        return;
    wclear(win_stat);
    wprintw(win_stat, "SPEED %d   MISS %d   TIME %.2f", speed, miss, time);
    wrefresh(win_stat); 
    if (uimod == MODBATLLE)
        wrefresh(win_entry);
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
    if (uimod != MODBATLLE)
        return;
    entrybuffer[strlen(entrybuffer)] = a;
    entrybuffer[strlen(entrybuffer)+1] = '\0';
    entryprint(i);
}

void uiEntryBack(int i){
    if (uimod != MODBATLLE)
        return;
    entrybuffer[strlen(entrybuffer)-1] = '\0';
    entryprint(i);
}

void uiEntryClear(){
    if (uimod != MODBATLLE)
        return;
    wclear(win_entry);
    wrefresh(win_entry);    
}

void uiProgPrint(struct plaerstr **p, int n){
    if (uimod != MODBATLLE && uimod != MODFINISH)
        return;
    wclear(win_prog);
    int i;
    for (i = 0; i < n; i++){
        mvwprintw(win_prog, i, 0, "%s ", p[i]->name);
        if (p[i]->prog == 100)
            wprintw(win_prog, "SPEED %d   MISS %d  TIME %.2f", p[i]->speed, p[i]->miss, p[i]->time);
        else 
            mvwhline(win_prog, i, SNAMEMAX, '=', (int)(((float)SCMAX-SNAMEMAX)/100*p[i]->prog));
    }
    wrefresh(win_prog);
    if (uimod == MODBATLLE)
        wrefresh(win_entry);   
}

void uiEnd(){
    endwin();
}
