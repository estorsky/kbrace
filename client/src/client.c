#include "../include/core.h"
#include "../include/ui.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <strings.h>
#include <unistd.h>
#include <pthread.h>
#include <netdb.h>
#include <sys/time.h>
#include <ncursesw/curses.h>
// #include <ncurses.h>
#include <sys/ioctl.h>
#include <pwd.h>
#include <signal.h>

#define h_addr h_addr_list[0] /*  for backward compatibility */

#define true 1
#define false 0

#define PORT 1337
#define MAX_PLAYERS 30
#define MAX_USERNAME 10
#define MAX_WORD_LEN 20
#define MAX_WORDS 100

char start[10] = "start";

double race_time = 0;
int player_id = 0;
int num_words = 0;
int progress = 0;

char state = 'x';
int online = 0;

struct passwd *ppasswd;
struct stat player_stat;
struct plaerstr **p;

int                 sockfd;
struct sockaddr_in  serv_addr;
struct hostent*     server;
char                key_buffer;

pthread_t tid[2];

/* char text[60][15] = {{"There "},{"are "},{"different "},{"kinds "},
    {"of "},{"animals "},{"on "},{"our "},{"planet, "},{"and "},{"all "},
    {"of "},{"them "},{"are "},{"very "},{"important "},{"for "},{"it. "},
    {"For" },{"example, "},{"everybody "},{"knows "},{"that "},{"the "},
    {"sharks" },{"are" },{"dangerous "},{"for "},{"people, "},{"but "},
    {"they "},{"are "},{"useful "},{"for "},{"cleaning "},{"seawater. "},
    {"There "},{"are "},{"two "},{"types "},{"of "},{"animals: "},
    {"domestic "},{"and "},{"wild. "},{"People "},{"keep "},{"pets "},{"in "},
    {"their "},{"homes."},{'\0'}}; */

char text [MAX_WORDS][MAX_WORD_LEN] = {{"404"}};

struct stat {
    int player_id;
    char name[MAX_USERNAME];
    int speed;
    int miss;
    double time;
    int prog;
    char state;
};

struct stat stats[MAX_PLAYERS];

double wtime() {
    struct timeval t;
    gettimeofday(&t, NULL);
    return (double)t.tv_sec + (double)t.tv_usec * 1E-6;
}

int miss = 0, cpm = 0;
int sec = 0;
int reset_stopwatch = 0; // 1 for reset

pthread_mutex_t for_cond = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

pthread_mutex_t start_sender = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t start_stat = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t start_battle = PTHREAD_MUTEX_INITIALIZER;

void *stat () {
    while(true) {
        // pthread_mutex_lock(&start_stat);

        pthread_mutex_lock(&for_cond);
        pthread_cond_wait(&cond, &for_cond);
        pthread_mutex_unlock(&for_cond);

        sec = 0;

        reset_stopwatch = 0;
        while (!reset_stopwatch) {
            uiStatPrint(cpm, miss, sec, online);
            sleep(1);
            sec++;
        }
    }
}

int reset_sender = 0;

void *sender () {
    char zero = '\0';
    int bytes = 0;

    bytes = recv(sockfd, &player_id, sizeof(player_id), 0);
    // printf("id? %db player id = %d\n", bytes, player_id);

    while(true) {
        for (int i = 0; i < MAX_PLAYERS; i++) {
            strncpy(stats[i].name, &zero, MAX_USERNAME);
            stats[i].player_id = 0;
            stats[i].speed = 0;
            stats[i].miss = 0;
            stats[i].time = 0;
            stats[i].prog = 0;
            stats[i].state = zero;

            strncpy(p[i]->name, &zero, MAX_USERNAME);
            p[i]->prog = 0;
            p[i]->speed = 0;
            p[i]->miss = 0;
            p[i]->time = 0;
            p[i]->state = zero;
        }

        memset(text, 0, sizeof(text[0][0]) * MAX_WORDS * MAX_WORD_LEN);
        state = 'v';

        bytes = recv(sockfd, &online, sizeof(online), 0);
        bytes = recv(sockfd, text, sizeof(char) * MAX_WORDS * MAX_WORD_LEN, 0);

        pthread_mutex_lock(&for_cond);
        pthread_cond_broadcast(&cond);
        pthread_mutex_unlock(&for_cond);

        reset_sender = 0;
        while (!reset_sender) {

            strncpy(player_stat.name, ppasswd->pw_name, MAX_USERNAME);
            player_stat.player_id = player_id;
            player_stat.speed = cpm;
            player_stat.miss = miss;
            player_stat.time = race_time;
            player_stat.prog = progress;
            player_stat.state = state;
            bytes = send(sockfd, &player_stat, sizeof(player_stat), 0);

            recv(sockfd, stats, sizeof(stats), 0); //comment if testing in local

            int i = 1;
            for(int n = 0; n < MAX_PLAYERS; n++) {
                if ( n == player_id ) {
                    strncpy(p[0]->name, stats[n].name, MAX_USERNAME);
                    p[0]->prog = stats[n].prog;
                    p[0]->state = stats[n].state;
                    if (p[0]->prog >= 100) {
                        p[0]->prog = 100;
                        p[0]->speed = stats[n].speed;
                        p[0]->miss = stats[n].miss;
                        p[0]->time = stats[n].time;
                        // continue;
                    }
                } else {
                    if (strcmp(stats[n].name, "")) {
                        strncpy(p[i]->name, stats[n].name, MAX_USERNAME);
                        p[i]->prog = stats[n].prog;
                        p[i]->state = stats[n].state;
                        if (p[i]->prog >= 100){
                            p[i]->prog = 100;
                            p[i]->speed = stats[n].speed;
                            p[i]->miss = stats[n].miss;
                            p[i]->time = stats[n].time;
                        }
                        i++;
                    }
                }
            }
            uiProgPrint(p, MAX_PLAYERS);
            if (player_stat.state == 'x' || player_stat.state == 'q') {
                break;
            }
            // recv(sockfd, stats, sizeof(stats), 0); // uncomment if testing in local
        }
        if (player_stat.state == 'q') {
            break;
        }
    }
}

struct plaerstr **createstr (int n) {
    struct plaerstr **p;
    int i;
    char zero = '\0';

    p = malloc(n * sizeof(struct plaerstr *));
    for(i = 0; i < n; i++){
        p[i] = malloc(sizeof(struct plaerstr));
        strncpy(p[i]->name, &zero, MAX_USERNAME);
        p[i]->prog = 0;
        p[i]->speed = 0;
        p[i]->miss = 0;
        p[i]->time = 0;
        p[i]->state = zero;
    }
    return p;
}

void exitprog() {
    state = 'q';
    pthread_join(tid[1], NULL);
    uiEnd();
    // pthread_cancel(tid[0]);
    // pthread_cancel(tid[1]);
    close(sockfd);
    exit(0);
}

void hdl (int sig) {
    uiEnd();
    pthread_cancel(tid[0]);
    pthread_cancel(tid[1]);
    sleep(1);
    close(sockfd);
    exit(0);
}

int main(int argc, char *argv[]) {

    struct sigaction sa;
    sa.sa_handler = hdl;
    sigaction(SIGINT, &sa, NULL);

    if (argc < 2){
        fprintf(stderr,"Please type:\n\t %s [server ip]\n to launch the game.\n", argv[0]);
        exit(0);
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        printf("ERROR opening socket\n");
        exit(0);
    }

    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR no such host.\n");
        exit(0);
    }


    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
            (char *)&serv_addr.sin_addr.s_addr,
            server->h_length);


    serv_addr.sin_port = htons(PORT);

    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) {
        printf("ERROR connecting\n");
        exit(0);
    }

    ppasswd = getpwuid(getuid());  // Check for NULL!
    // printf("User name: %s\n", ppasswd->pw_name);

    p = createstr(MAX_PLAYERS);

    pthread_create(&tid[0], NULL, stat, NULL);
    pthread_create(&tid[1], NULL, sender, NULL);

    uiInit();

    while (true) {
        uiRun();
        uiHelpPrint("awaiting text");

        pthread_mutex_lock(&for_cond);
        pthread_cond_wait(&cond, &for_cond);
        pthread_mutex_unlock(&for_cond);

        uiHelpPrint("[ESC/F10] exit | [CTRL+U] clear word");
        uiStartBattle(text);

        miss = 0;
        cpm = 0;
        progress = 0;

        int i = 0, j = 0;

        num_words = 0;
        while(text[i][0] != '\0') {
            num_words++;
            i++;
        }

        race_time = wtime();
        int num_char = 0;

        int word_count = 0;
        double cur_t = wtime();
        int cur_len_words = 0;

        uiProgPrint(p, MAX_PLAYERS);

        i = 0;
        while(text[i][0] != '\0') {
            uiTextLowline(text, i);

            j = 0;
            char ch = '\0';
            int err = 0;

            if (word_count > 1) {
                cur_t = wtime() - cur_t;
                cpm = ((cur_len_words)/(cur_t / 60));
                uiStatPrint(cpm, miss, sec, online);

                cur_t = wtime();
                cur_len_words = 0;
                word_count = 0;
            }

            while(text[i][j] != '\0') {
                ch = getch();
                // printw("%d", ch);;
                switch (ch) {
                    case 7:
                        if (err > 0) {
                            err--;
                            uiEntryBack(1); //bag
                        } else {
                            if (j > 0) j--;
                            uiEntryBack(0);
                        }
                        break;
                    case 21:
                        uiEntryBack(0);
                        uiEntryClear();
                        err = 0;
                        j = 0;
                        break;
                    case 27:
                        exitprog();
                        break;
                    case 18:
                        exitprog();
                        break;
                    default:
                        if (ch == text[i][j] && err == 0) {
                            uiEntryPrint(ch, 0);
                            j++;
                            cur_len_words++;
                            num_char++;
                        } else {
                            uiEntryPrint(ch, 1);
                            err++;
                            if (err == 1) {
                                miss++;
                                uiStatPrint(cpm, miss, sec, online);
                            }
                        }
                        break;
                }
                // printw("er:%d  j:%d \n", err, j);;
            }
            word_count++;
            // printw("%d %d\n", len_words, word_count);
            uiEntryClear();

            i++;
            progress = i * 100 / num_words;
            // printw("Progress %d", progress);
        }

        race_time = wtime() - race_time;
        cpm = ((num_char) / (race_time / 60));

        reset_stopwatch = 1;

        uiFinishBattle();
        uiStatPrint(cpm, miss, race_time, online);
        uiProgPrint(p, MAX_PLAYERS);
        uiHelpPrint("[ESC/F10] exit | press any key to start a new race");

        int exit_lobby = 0;
        char ch = '\0';
        while (!exit_lobby) {
            ch = getch();
            switch (ch) {
                    case 27:
                        exitprog();
                        break;
                    case 18:
                        exitprog();
                        break;
                    default:
                        exit_lobby = 1;
                        break;
            }
        }
        // reset_sender = 1;
        state = 'x';
    }
    exitprog();
}

