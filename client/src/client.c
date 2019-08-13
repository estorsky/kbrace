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
#include <ncurses.h>
#include <sys/ioctl.h>
#include <pwd.h>
#include <signal.h>

#define h_addr h_addr_list[0] /*  for backward compatibility */

#define true 1
#define false 0

#define PORT 1337
#define MAX_PLAYERS 30
#define MAX_USERNAME 10
#define MAX_WORD_LEN 15
#define MAX_WORDS 100

double race_time = 0;
int player_id = 0;
int num_words = 0;
int progress = 0;

char state = 'x';

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
int reset_stopwatch = 1; // 0 for reset

pthread_mutex_t for_cond = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

void *stat () {
    while(true) {
        pthread_mutex_lock(&for_cond);
        pthread_cond_wait(&cond, &for_cond);
        pthread_mutex_unlock(&for_cond);

        sec = 0;
        miss = 0;
        cpm = 0;

        reset_stopwatch = 1;
        while (reset_stopwatch) {
            uiStatPrint(cpm, miss, sec);
            sleep(1);
            sec++;
        }
    }
}

int reset_prog = 1;

void *prog () {
    char zero = '\0';
    while(true) {
        pthread_mutex_lock(&for_cond);
        pthread_cond_wait(&cond, &for_cond);
        pthread_mutex_unlock(&for_cond);

        // clear structs
        /* for (int i = 0; i < 15; i++) {
            strncpy(stats[i].name, &zero, MAX_USERNAME);
            stats[i].player_id = -1;
            stats[i].speed = 0;
            stats[i].miss = 0;
            stats[i].time = 0;
            stats[i].prog = 0;
            //state
        } */

        reset_prog = 1;
        while (reset_prog) {
            strncpy(player_stat.name, ppasswd->pw_name, MAX_USERNAME);
            player_stat.player_id = player_id;
            player_stat.speed = cpm;
            player_stat.miss = miss;
            player_stat.time = race_time;
            player_stat.prog = progress;
            player_stat.state = state;
            send(sockfd, &player_stat, sizeof(player_stat), 0);

            for (int n = 0; n < MAX_PLAYERS; n++) {
                recv(sockfd, &stats, sizeof(stats), 0);
            }

            int i = 0;
            for(int n = 0; n < MAX_PLAYERS; n++) {
                if ( n == player_id ) {
                    strncpy(p[0]->name, stats[n].name, MAX_USERNAME);
                    p[0]->prog = stats[n].prog;
                    if (p[0]->prog >= 100) {
                        p[0]->prog = 100;
                        p[0]->speed = stats[n].speed;
                        p[0]->miss = stats[n].miss;
                        p[0]->time = stats[n].time;
                        p[0]->state = stats[n].state;
                        continue;
                    }
                }
                if (strcmp(stats[n].name, "")) {
                    strncpy(p[i]->name, stats[n].name, MAX_USERNAME);
                    p[i]->prog = stats[n].prog;
                    if (p[i]->prog >= 100){
                        p[i]->prog = 100;
                        p[i]->speed = stats[n].speed;
                        p[i]->miss = stats[n].miss;
                        p[i]->time = stats[n].time;
                        p[i]->state = stats[n].state;
                    }
                    i++;
                }
            }

            uiProgPrint(p, MAX_PLAYERS);
            sleep(1);
        }
    }
}

struct plaerstr **createstr(int n){
    struct plaerstr **p;
    int i;
    char ch = '\0';

    p = malloc(n * sizeof(struct plaerstr *));
    for(i = 0; i < n; i++){
        p[i] = malloc(sizeof(struct plaerstr));
        strncpy(p[i]->name, &ch, MAX_USERNAME);
        p[i]->prog = 0;
        /* if (p[i]->prog >= 100){
            p[i]->prog = 100;
            p[i]->speed = 356;
            p[i]->miss = 13;
            p[i]->time = 98;
        } */
    }
    return p;
}

void exitprog() {
    uiEnd();
    pthread_cancel(tid[0]);
    pthread_cancel(tid[1]);
    close(sockfd);
    exit(0);
}

void hdl (int sig) {
    exitprog();
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
        fprintf(stderr,"ERROR, no such host.\n");
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
        return 0;
    }

    ppasswd = getpwuid(getuid());  // Check for NULL!
    // printf("User name: %s\n", ppasswd->pw_name);

    pthread_create(&tid[0], NULL, stat, NULL);
    pthread_create(&tid[1], NULL, prog, NULL);

    int bytes = recv(sockfd, &player_id, sizeof(player_id), 0);
    // send(sockfd, ppasswd->pw_name, sizeof(ppasswd->pw_name), 0);
    // bytes = recv(sockfd, text, sizeof(text), 0);
    // printf("bytes:%d id:%d %s\n ", bytes, player_id, text);

    uiInit();

    while (true) {
        uiRun();

        // int cur_num_users = 0;
        // recv(sockfd, &cur_num_users, sizeof(cur_num_users), 0);
        bytes = recv(sockfd, &text, sizeof(text), 0);
        // printf("bytes:%d id:%d\n ", bytes, player_id);

        uiStartBattle(text);


        progress = 0;
        state = 'v';
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

        pthread_mutex_lock(&for_cond);
        pthread_cond_broadcast(&cond);
        pthread_mutex_unlock(&for_cond);

        p = createstr(MAX_PLAYERS);
        uiProgPrint(p, MAX_PLAYERS);

        while(text[i][0] != '\0') {
            while(text[i][j] != '\0') {
            }
        }

        i = 0;
        while(text[i][0] != '\0') {
            uiTextLowline(text, i);

            j = 0;
            char ch = '\0';
            int err = 0;

            if (word_count > 1) {
                cur_t = wtime() - cur_t;
                cpm = ((cur_len_words)/(cur_t / 60));
                uiStatPrint(cpm, miss, sec);

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
                        state = 'x';
                        strncpy(&player_stat.state, &state, 1);
                        send(sockfd, &player_stat, sizeof(player_stat), 0);
                        exitprog();
                        break;
                    case 18:
                        state = 'x';
                        strncpy(&player_stat.state, &state, 1);
                        send(sockfd, &player_stat, sizeof(player_stat), 0);
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
                                uiStatPrint(cpm, miss, sec);
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

        reset_stopwatch = 0;

        uiStatPrint(cpm, miss, race_time);
        uiFinishBattle();

        int exit_lobby = 1;
        char ch = '\0';
        while (exit_lobby) {
            ch = getch();
            switch (ch) {
                    case 27:
                        state = 'x';
                        strncpy(&player_stat.state, &state, 1);
                        send(sockfd, &player_stat, sizeof(player_stat), 0);
                        exitprog();
                        break;
                    case 18:
                        state = 'x';
                        strncpy(&player_stat.state, &state, 1);
                        send(sockfd, &player_stat, sizeof(player_stat), 0);
                        exitprog();
                        break;
                    case 7:
                    case 32:
                    case 10:
                        exit_lobby = 0;
                        break;
                    default:
                        break;
            }

        }

        reset_prog = 0;

        state = 'x';
        strncpy(&player_stat.state, &state, 1);
        send(sockfd, &player_stat, sizeof(player_stat), 0);
    }

    exitprog();
}

