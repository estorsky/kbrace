#include "../include/ui.h"

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
WINDOW *win_help;
char entrybuffer[MAX_WORD_LEN];
int uimod = MODOFF;

pthread_mutex_t ncur = PTHREAD_MUTEX_INITIALIZER;

void uiInit(){
    if (uimod != MODOFF)
        return;
    pthread_mutex_lock(&ncur);
    initscr();
    cbreak();
    noecho();
    refresh();
    keypad(stdscr, true);
    uimod = MODINIT;
    pthread_mutex_unlock(&ncur);
}

void uiRun(){
    if (uimod != MODINIT && uimod != MODFINISH)
        return;
    pthread_mutex_lock(&ncur);
    if (uimod == MODINIT){ // first start
        win_stat = newwin(SLSTAT, SCMAX, 1, 1);
        win_help = newwin(1, COLS, LINES-1, 0);
        wattron(win_help, A_STANDOUT);
    }
    if (uimod == MODFINISH){ // next start
        delwin(win_prog);
        clear();
        refresh();
    }
    curs_set(0);
    win_text = newwin(7, 8, LINES/2-4, COLS/2-4);
    wprintw(win_text,"########");
    wprintw(win_text,"#      #");
    wprintw(win_text," #    # ");
    wprintw(win_text,"  #  #  ");
    wprintw(win_text," #    # ");
    wprintw(win_text,"#      #");
    wprintw(win_text,"########");
    wrefresh(win_text);

    uimod = MODRUN;
    pthread_mutex_unlock(&ncur);
}

int textprint(char text[][MAX_WORD_LEN], int printoff, int lowline){
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

void timer(int sltext){
    char number[3][45] = {"      ##\n    ####\n      ##\n      ##\n      ##",
                          "  ####  \n##    ##\n    ##  \n  ##    \n########",
                          "######  \n      ##\n  ####  \n      ##\n######  "};
    int i = 3;
    while(i > 0){
        if (sltext >= 5)
            wprintw(win_text,"%s", number[i-1]);
        else
            wprintw(win_text,"%d", i);
        wrefresh(win_text);
        wrefresh(win_entry);  // cursor move
        sleep(SLEEPTIME);
        wclear(win_text);
        i--;
    }
    wrefresh(win_text);
    wrefresh(win_entry); // cursor move
}

void uiStartBattle(char text[][MAX_WORD_LEN]){
    if (uimod != MODRUN)
        return;
    pthread_mutex_lock(&ncur);

    wclear(win_text); // clear and del hourglass
    wrefresh(win_text);
    delwin(win_text);

    curs_set(1); // on cursor (1 = normal mod)

    mvhline(SLSTAT + 1, 0, 0, SCMAX+2);

    win_text = newwin(SLMAX - SLSTAT - SLENTRY - 2, SCMAX, SLSTAT + 2, 1);
    int sltext = textprint(text, 1, 0);
    sltext += 2;
    wresize(win_text, sltext, SCMAX);

    mvhline(SLSTAT + sltext + 2, 0, 0, SCMAX+2);

    win_entry = newwin(SLENTRY, SCMAX, SLSTAT + sltext + 3, 1);

    int slprog = SLMAX - SLSTAT - SLENTRY - sltext - 3; // checking free space for win_prog
    if (slprog > 0){
        mvhline(SLSTAT + SLENTRY + sltext + 3, 0, 0, SCMAX+2);
        win_prog = newwin(slprog, SCMAX, SLSTAT + SLENTRY + sltext + 4, 1);
    }

    refresh();
    wrefresh(win_entry);

    timer(sltext);

    uimod = MODBATLLE;
    pthread_mutex_unlock(&ncur);
}

void uiTextLowline(char text[][MAX_WORD_LEN], int n){
    if (uimod != MODBATLLE)
        return;
    pthread_mutex_lock(&ncur);
    textprint(text, 0, n);
    wrefresh(win_entry);  // cursor move
    pthread_mutex_unlock(&ncur);
}

void uiFinishBattle(){
    if (uimod != MODBATLLE)
        return;
    pthread_mutex_lock(&ncur);
    curs_set(0);
    delwin(win_text);
    delwin(win_entry);
    delwin(win_prog);
    clear();

    mvhline(SLSTAT + 1, 0, 0, SCMAX+2);
    win_prog = newwin(SLMAX - SLSTAT - 1, SCMAX, SLSTAT + 2, 1);
    refresh();
    wrefresh(win_prog);
    uimod = MODFINISH;
    pthread_mutex_unlock(&ncur);
}

void uiStatPrint(int speed, int miss, double time, int online){
    if (uimod != MODBATLLE && uimod != MODFINISH)
        return;
    pthread_mutex_lock(&ncur);
    int precision = 0;
    if (uimod == MODFINISH)
        precision = 2;
    wclear(win_stat);
    wprintw(win_stat, "SPEED %-3d  MISS %-3d  TIME %-3.*f", speed, miss, precision, time);
    mvwprintw(win_stat, 0, SCMAX-9, "ONLINE %-d", online);
    wrefresh(win_stat);
    if (uimod == MODBATLLE)
        wrefresh(win_entry); // cursor move
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
    if (uimod != MODBATLLE)
        return;
    pthread_mutex_lock(&ncur);
    entrybuffer[strlen(entrybuffer)] = a;
    entrybuffer[strlen(entrybuffer)+1] = '\0';
    entryprint(i);
    pthread_mutex_unlock(&ncur);
}

void uiEntryBack(int i){
    if (uimod != MODBATLLE)
        return;
    pthread_mutex_lock(&ncur);
    entrybuffer[strlen(entrybuffer)-1] = '\0';
    entryprint(i);
    pthread_mutex_unlock(&ncur);
}

void uiEntryClear(){
    if (uimod != MODBATLLE)
        return;
    pthread_mutex_lock(&ncur);
    memset(entrybuffer, 0, MAX_WORD_LEN);
    wclear(win_entry);
    wrefresh(win_entry);
    pthread_mutex_unlock(&ncur);
}

void uiProgPrint(struct plaerstr **p, int n){
    if (uimod != MODBATLLE && uimod != MODFINISH)
        return;
    pthread_mutex_lock(&ncur);
    wclear(win_prog);
    int i;
    for (i = 0; i < n; i++){
        if (p[i]->name[0] != '\0'){
            mvwprintw(win_prog, i, 0, "%s ", p[i]->name);
            if (p[i]->prog == 100)
                mvwprintw(win_prog, i, MAX_USERNAME, "SPEED %-3d  MISS %-3d  TIME %-3.2f", p[i]->speed, p[i]->miss, p[i]->time);
            else{
                mvwprintw(win_prog, i, MAX_USERNAME, "[");
                mvwhline(win_prog, i, MAX_USERNAME+1, '=', (int)(((float)SCMAX-MAX_USERNAME-7)/100*p[i]->prog));
                mvwprintw(win_prog, i, SCMAX-5, "]");
            }
            mvwprintw(win_prog, i, SCMAX-2, "%c", p[i]->state);
        }
    }
    wrefresh(win_prog);
    if (uimod == MODBATLLE)
        wrefresh(win_entry); // cursor move
    pthread_mutex_unlock(&ncur);
}

void uiHelpPrint(char texthelp[]){
    if (uimod != MODRUN && uimod != MODBATLLE && uimod != MODFINISH)
        return;
    pthread_mutex_lock(&ncur);
    wclear(win_help);
    wprintw(win_help," %s%*s", texthelp, COLS-strlen(texthelp), "");
    wrefresh(win_help);
    if (uimod == MODBATLLE)
        wrefresh(win_entry); // cursor move
    pthread_mutex_unlock(&ncur);
}

void uiEnd(){
    pthread_mutex_lock(&ncur);
    endwin();
    uimod = MODOFF;
    pthread_mutex_unlock(&ncur);
}
