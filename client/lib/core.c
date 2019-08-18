#include "../include/core.h"
#include "../include/ui.h"

void exitprog() {
    char bye_text[100] = "good bye, ";
    // strncat(bye_text, ppasswd->pw_name, sizeof(bye_text));
    strncat(bye_text, username, sizeof(bye_text));
    uiHelpPrint(bye_text);
    state = 'q';
    pthread_join(tid[1], NULL);
    uiEnd();
    // pthread_cancel(tid[0]);
    // pthread_cancel(tid[1]);
    close(sockfd);
    exit(0);
}

