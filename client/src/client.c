#include "../include/core.h"
#include "../include/ui.h"

#define h_addr h_addr_list[0] /*  for backward compatibility */

const int token = TOKEN;

int player_id = 0;
int num_words = 0;
int progress = 0;

double race_time = 0;
int num_char = 0;
int miss = 0, cpm = 0;
int sec = 0;
int reset_stopwatch = 0; // 1 for reset

char state = '\0';
int online = 0;

int local = 0;
char username[MAX_USERNAME] = "\0";

struct passwd *ppasswd;
struct stat player_stat;
struct plaerstr **p;

int                 sockfd;
struct sockaddr_in  serv_addr = { 0 };
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

struct stat stats[MAX_PLAYERS];

pthread_mutex_t for_cond = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

pthread_mutex_t sndr = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t sw = PTHREAD_MUTEX_INITIALIZER;

void *stopwatch () {
    while(true) {
        pthread_mutex_lock(&for_cond);
        pthread_cond_wait(&cond, &for_cond);
        pthread_mutex_unlock(&for_cond);

        pthread_mutex_lock(&sw);
        pthread_mutex_unlock(&sw);

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

    bytes = send(sockfd, &token, sizeof(token), 0);

    bytes = recv(sockfd, &player_id, sizeof(player_id), 0);
    // printf("id? %db player id = %d\n", bytes, player_id);

    while(true) {
        strncpy(player_stat.name, &zero, MAX_USERNAME);
        player_stat.id = 0;
        player_stat.speed = 0;
        player_stat.miss = 0;
        player_stat.time = 0;
        player_stat.prog = 0;
        player_stat.state = '\0';

        for (int i = 0; i < MAX_PLAYERS; i++) {
            strncpy(stats[i].name, &zero, MAX_USERNAME);
            stats[i].id = 0;
            stats[i].speed = 0;
            stats[i].miss = 0;
            stats[i].time = 0;
            stats[i].prog = 0;
            stats[i].state = '\0';
        }

        memset(text, 0, sizeof(text[0][0]) * MAX_WORDS * MAX_WORD_LEN);
        state = 'v';

        bytes = recv(sockfd, &online, sizeof(online), 0);
        if (bytes <= 0) {
            exitprog();
        }
        bytes = recv(sockfd, text, sizeof(char) * MAX_WORDS * MAX_WORD_LEN, 0);
        if (bytes <= 0) {
            exitprog();
        }

        pthread_mutex_lock(&for_cond);
        pthread_cond_broadcast(&cond);
        pthread_mutex_unlock(&for_cond);

        // pthread_mutex_lock(&sndr);
        // pthread_mutex_unlock(&sndr);

        reset_sender = 0;
        while (!reset_sender) {
            if (state != 'x') {
                strncpy(player_stat.name, username, MAX_USERNAME);
                player_stat.id = player_id;
                player_stat.speed = cpm;
                player_stat.miss = miss;
                player_stat.time = race_time;
                player_stat.prog = progress;
            }
            player_stat.state = state;
            bytes = send(sockfd, &player_stat, sizeof(player_stat), 0);

            if (!local) { //very bed crutch
                bytes = recv(sockfd, stats, sizeof(stats), 0);
            }

            uiProgPrint(stats, MAX_PLAYERS, player_id);
            if (player_stat.state == 'x' || player_stat.state == 'q') {
                break;
            }
            if (local) {
                bytes = recv(sockfd, stats, sizeof(stats), 0);
            }
            if (bytes <= 0) {
                exitprog();
            }
        }
        if (player_stat.state == 'q') {
            break;
        }
    }
}

int main(int argc, char *argv[]) {

    // struct sigaction sa;
    // sa.sa_handler = hdl;
    // sigaction(SIGINT, &sa, NULL);

    signal(SIGPIPE, SIG_IGN);

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

    if ( !strcmp(argv[1], "127.0.0.1")) {
        local = 1;
    }

    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
            (char *)&serv_addr.sin_addr.s_addr,
            server->h_length);


    serv_addr.sin_port = htons(PORT);

    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) {
        printf("ERROR connecting\n");
        exit(0);
    }

    ppasswd = getpwuid(getuid());
    if (ppasswd == NULL) {
        printf("ERROR no such username\n");
    }
    sprintf(username, "%s", ppasswd->pw_name);
    // printf("User name: %s\n", ppasswd->pw_name);

    pthread_create(&tid[0], NULL, stopwatch, NULL);
    pthread_create(&tid[1], NULL, sender, NULL);

    uiInit();

    while (true) {
        num_char = 0;
        miss = 0;
        cpm = 0;
        progress = 0;
        race_time = 0;

        pthread_mutex_lock(&sw);
        // pthread_mutex_lock(&sndr);

        uiRun();
        uiHelpPrint("awaiting text...");

        pthread_mutex_lock(&for_cond);
        pthread_cond_wait(&cond, &for_cond);
        pthread_mutex_unlock(&for_cond);

        uiStartBattle(text);
        uiHelpPrint("[ESC/F10] exit | [CTRL+U] clear text line");

        int i = 0, j = 0;

        num_words = 0;
        while(text[i][0] != '\0') {
            num_words++;
            i++;
        }

        race_time = wtime();

        int word_count = 0;
        double cur_t = wtime();
        int cur_len_words = 0;

        uiProgPrint(stats, MAX_PLAYERS, player_id);
        pthread_mutex_unlock(&sw);
        // pthread_mutex_unlock(&sndr);

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
                    case BACKSPACE:
                        if (err > 0) {
                            err--;
                            if (err == 0) uiEntryBack(0);
                            else uiEntryBack(1);
                        } else {
                            if (j > 0) j--;
                            uiEntryBack(0);
                        }
                        break;
                    case CTRL_U:
                        uiEntryBack(0);
                        uiEntryClear();
                        err = 0;
                        j = 0;
                        break;
                    case ESC:
                    case F10:
                        exitprog();
                        break;
                    case ARROW_L:
                    case ARROW_R:
                    case ARROW_U:
                    case ARROW_D:
                    case ENTER:
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
        uiProgPrint(stats, MAX_PLAYERS, player_id);
        uiHelpPrint("[ESC/F10] exit | [ENTER] new race");

        sleep(3);

        int exit_lobby = 0;
        char ch = '\0';
        while (!exit_lobby) {
            ch = getch();
            switch (ch) {
                    case ESC:
                        exitprog();
                        break;
                    case F10:
                        exitprog();
                        break;
                    case ENTER:
                        exit_lobby = 1;
                        break;
                    default:
                        break;
            }
        }
        // reset_sender = 1;
        state = 'x';
    }
    exitprog();
}

