#include "../include/core.h"

#include <dirent.h>

int getNewText (char text[][MAX_WORD_LEN]) {
    DIR *d;
    struct dirent *dir;
    char path[sizeof(dir->d_name)+strlen(PATH_TO_TEXTS)];
    sprintf(path, "%s", PATH_TO_TEXTS);
    d = opendir(path);
    if (d) {
        int num_files = 0;
        while ((dir = readdir(d)) != NULL) {
            // printf("%s\n", dir->d_name);
            num_files++;
        }
        // printf("%d\n", num_files);

        rewinddir(d);

        int sel_file = (rand() % (num_files-1 - 0 + 1)) + 0; // 0 lower
        // printf("sel file = %d\n", sel_file);
        int i = 0;
        while ((dir = readdir(d)) != NULL) {
            if (i == sel_file) {
                if ( !strcmp(dir->d_name, ".") || !strcmp(dir->d_name, "..")) {
                    sel_file++;
                    // printf(".. .\n");
                    if (sel_file >= num_files) {
                        sel_file = 0;
                        i = 0;
                        rewinddir(d);
                        continue;
                    }
                } else {
                    sprintf(path, "%s%s", PATH_TO_TEXTS, dir->d_name);
                    break;
                }
            }
            i++;
        }
        closedir(d);
    } else return 1;

    // printf("%s\n", path);

    memset(text, 0, sizeof(text[0][0]) * MAX_WORDS * MAX_WORD_LEN);

    FILE *file = fopen(path, "r");
    if (file) {
        short int c; // for arm
        c = fgetc(file);
        int i = 0, j = 0, tempJ = 0;
        while ( c != EOF ) {
            tempJ = j;
            if (c == '\n') c = ' ';
            text[i][j] = c;
            j++;
            if (c == ' ') {
                i++;
                j = 0;
            }
            // printf("%c", c);
            c = fgetc(file);
        }
        i--;
        text[i][tempJ] = '\0';
        i++;
        text[i][0] = '\0';
        fclose(file);
    } else return 1;
    printf("(%s) selected text %s\n", curTime(), path);
    return 0;
}

const char* curTime() {
    time_t rawtime;
    struct tm * timeinfo;

    static char result[26];
    time (&rawtime);
    timeinfo = localtime (&rawtime);
    sprintf(result, "%s", asctime(timeinfo));
    result[strlen(result)-1] = '\0';
    return result;
}

