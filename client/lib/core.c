#include "../include/core.h"
#include "../include/ui.h"

void exitprog() {
    pthread_cancel(tid[0]);
    char text [] = "good bye, ";
    char bye[MAX_USERNAME+strlen(text)];
    sprintf(bye, "%s%s", text, username);
    uiHelpPrint(bye);
    state = 'q';
    pthread_join(tid[1], NULL);
    uiEnd();
    close(sockfd);
    exit(0);
}

double wtime() {
    struct timeval t;
    gettimeofday(&t, NULL);
    return (double)t.tv_sec + (double)t.tv_usec * 1E-6;
}

void hdl (int sig) {
    uiEnd();
    pthread_cancel(tid[0]);
    pthread_cancel(tid[1]);
    sleep(1);
    close(sockfd);
    exit(0);
}

/* struct plaerstr **createstr (int n) {
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
} */

