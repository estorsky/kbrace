#include "../include/core.h"
#include "../include/ui.h"

void exitprog() {
    pthread_cancel(tid[0]);
    char bye_text[100] = "good bye, ";
    strncat(bye_text, username, sizeof(bye_text));
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


