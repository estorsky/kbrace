#include "../include/core.h"
// #include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <strings.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
// #include <error.h>

char text[MAX_WORDS][MAX_WORD_LEN] = {{"There "},{"are "},{"different "},{"kinds "},
    // {"of "},{"animals "},{"on "},{"our "},{"planet, "},{"and "},{"all "},
    // {"of "},{"them "},{"are "},{"very "},{"important "},{"for "},{"it. "},
    // {"For " },{"example, "},{"everybody "},{"knows "},{"that "},{"the "},
    // {"sharks " },{"are " },{"dangerous "},{"for "},{"people, "},{"but "},
    // {"they "},{"are "},{"useful "},{"for "},{"cleaning "},{"seawater. "},
    // {"There "},{"are "},{"two "},{"types "},{"of "},{"animals: "},
    // {"domestic "},{"and "},{"wild. "},{"People "},{"keep "},{"pets "},{"in "},
    {"their "},{"homes."},{'\0'}};

int session_num_users = 0;
int session_id = 0;
int online_users = 0;

pthread_mutex_t for_cond = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

pthread_mutex_t start_play = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t nu = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t ou = PTHREAD_MUTEX_INITIALIZER;

struct stat {
    int player_id;
    char name[MAX_USERNAME];
    int speed;
    int miss;
    double time;
    int prog;
    char state;
};


struct stat stats[NUM_STRUCTS][MAX_PLAYERS];

void *player (void *arg) {
    int fd = *(int*) arg;
    int player_id = fd-3;
    struct stat player_stat;
    int usr_session_id = 0;
    int bytes = 0;
    printf("# player id = %d connected!\n", player_id);

    pthread_mutex_lock(&ou);
    online_users++;
    pthread_mutex_unlock(&ou);

    bytes = send(fd, &player_id, sizeof(player_id), 0);
    printf("%db send id\n", bytes);

    int exit_session = 0;
    while (!exit_session) {
        pthread_mutex_unlock(&start_play);
        usr_session_id = session_id;

        pthread_mutex_lock(&nu);
        session_num_users++;
        pthread_mutex_unlock(&nu);

        pthread_mutex_lock(&for_cond);
        pthread_cond_wait(&cond, &for_cond);
        pthread_mutex_unlock(&for_cond);

        bytes = send(fd, &online_users, sizeof(online_users), 0);
        if (bytes <= 0) {
            printf("online not send\n");
            break;
        }
        printf("%db send online\n", bytes);

        bytes = send(fd, text, sizeof(char) * MAX_WORDS * MAX_WORD_LEN, 0);
        if (bytes <= 0) {
            break;
        }
        printf("%db send text\n", bytes);

        int num_pack = 0;
        int exit_race = 0;
        while (!exit_race) {
            // printf("\n");
            bytes = recv(fd, &player_stat, sizeof(player_stat), 0);
            printf("get stat %d\n", bytes);
            printf("## session #%d player %s: id %3d, speed %3d, miss %3d, time %4.2f %c [%d]\n",
                    usr_session_id,
                    player_stat.name,
                    player_stat.player_id,
                    player_stat.speed,
                    player_stat.miss,
                    player_stat.time,
                    player_stat.state,
                    num_pack);

            if (bytes <= 0 || num_pack > LIM_PACK) {
                stats[usr_session_id][player_id].state = 'q';
            }

            strncpy(stats[usr_session_id][player_id].name, player_stat.name, MAX_USERNAME);
            stats[usr_session_id][player_id].player_id = player_stat.player_id;
            stats[usr_session_id][player_id].speed = player_stat.speed;
            stats[usr_session_id][player_id].miss = player_stat.miss;
            stats[usr_session_id][player_id].time = player_stat.time;
            stats[usr_session_id][player_id].prog = player_stat.prog;
            stats[usr_session_id][player_id].state = player_stat.state;

            if (bytes <= 0 || num_pack > LIM_PACK) {
                break;
            }

            if (stats[usr_session_id][player_id].state == 'q' ||
                    stats[usr_session_id][player_id].state == 'x') {
                // printf("q or x\n");
                break;
            }

            bytes = send(fd, stats[usr_session_id], sizeof(player_stat) * MAX_PLAYERS, 0);
            printf("%db send stats\n", bytes);
                    // printf("send %db\n", bytes);
            num_pack++;
            // sleep(1);
            usleep(500000);
        }

        printf("## session #%d player %s end race: id %3d, speed %3d, miss %3d, time %.2f\n",
                usr_session_id,
                player_stat.name,
                player_stat.player_id,
                player_stat.speed,
                player_stat.miss,
                player_stat.time);

        if (player_stat.state == 'q' || bytes <= 0 || num_pack > LIM_PACK) {
            break;
            // exit_session = 1;
        }
    }

    pthread_mutex_lock(&ou);
    online_users--;
    pthread_mutex_unlock(&ou);

    close(fd);
    return 0;
}

void *session (void *arg) {
    printf("session thread start\n");
    pthread_mutex_lock(&start_play);
    char zero = '\0';

    while (true) {
        for (int i = 0; i < MAX_PLAYERS; i++) {
            strncpy(stats[session_id][i].name, &zero, MAX_USERNAME);
            stats[session_id][i].player_id = 0;
            stats[session_id][i].speed = 0;
            stats[session_id][i].miss = 0;
            stats[session_id][i].time = 0;
            stats[session_id][i].prog = 0;
            stats[session_id][i].state = zero;
        }

        pthread_mutex_lock(&start_play);
        printf("new iteration session\n");

        getNewText(text);

        printf("timer start %d sec\n", DELAY_WAIT);
        sleep(DELAY_WAIT);

        pthread_mutex_lock(&for_cond);
        pthread_cond_broadcast(&cond);
        pthread_mutex_unlock(&for_cond);

        printf("session #%d starting! players in session: %d, total players:%d\n",
                session_id,
                session_num_users,
                online_users);

        session_id++;

        pthread_mutex_lock(&nu);
        session_num_users = 0;
        pthread_mutex_unlock(&nu);

        if (session_id > NUM_STRUCTS) {
            session_id = 0;
        }
    }
}

int socket_fds[MAX_PLAYERS];

void hdl (int sig) {
    close(socket_fds[0]);
    printf("server killed\n");
    exit(0);
}

int main(int argc, char *argv[]) {
    srand(time(NULL));

    struct sigaction sa;
    sa.sa_handler = hdl;
    sigaction(SIGINT, &sa, NULL);

    pthread_t tid[MAX_PLAYERS];
    struct sockaddr_in socket_addr[MAX_PLAYERS];
    int i;

    socket_fds[0] = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fds[0] < 0) {
        printf("ERROR opening socket\n");
        return 1;
    }

    bzero((char *) &socket_addr[0], sizeof(socket_addr[0]));
    socket_addr[0].sin_family = AF_INET;
    socket_addr[0].sin_addr.s_addr = INADDR_ANY;
    socket_addr[0].sin_port = htons(PORT);

    if (bind(socket_fds[0], (struct sockaddr *) &socket_addr[0], sizeof(socket_addr[0])) < 0) {
        printf("ERROR on binding\n");
        return 1;
    }

    listen(socket_fds[0], 5);
    socklen_t clilen = sizeof(socket_addr[0]);

    pthread_create(&tid[0], NULL, session, NULL);

    for(i = 1;; i++){
        //Accepting an incoming connection request
        socket_fds[i] = accept(socket_fds[0], (struct sockaddr *) &socket_addr[i], &clilen);
        if (socket_fds[i] < 0)
            printf("ERROR on accept\n");
        //Reset game if someone won
        /* if(someone_won){
            printf("asdf\n");
            someone_won = 0;
        } */
        pthread_create(&tid[i], NULL, player, &socket_fds[i]);
        // make_thread(&gameplay, &socket_fds[i]);
    }

    close(socket_fds[0]);

    return 0;
}

