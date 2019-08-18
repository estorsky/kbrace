#include "../include/core.h"

#include <dirent.h>

int getNewText (char text[][MAX_WORD_LEN]) {
    char path[256] = "./server/texts/";
    DIR *d;
    struct dirent *dir;
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
                    strncat(path, dir->d_name, sizeof(path));
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
        char c;
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
    printf("selected text %s\n", path);
    return 0;
}

