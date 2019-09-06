#include "../include/core.h"
#include "../include/ui.h"

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
    pthread_cancel(tid[0]);
    char bye_text[MAX_USERNAME+10];
    sprintf(bye_text, "%s%s", "good bye, ", username);
    uiHelpPrint(bye_text);
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

