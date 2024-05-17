#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>

void encode_dir_entries(FILE* f, const char* directory) {
    DIR* dir = opendir(directory);
    if (dir == NULL) {
        perror("opendir");
        return;
    }

    fprintf(f, "DIR_START %s\n", directory);

    struct dirent* entry;
    char path[PATH_MAX];

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        snprintf(path, sizeof(path), "%s/%s", directory, entry->d_name);
        struct stat statbuf;
        if (stat(path, &statbuf) == -1) {
            perror("stat");
            continue;
        }

        if (S_ISDIR(statbuf.st_mode)) {
            fprintf(f, "%s\n", entry->d_name);
            encode_dir_entries(f, path); // 재귀적으로 하위 디렉토리를 탐색
        } else {
            fprintf(f, "%s\n", entry->d_name);
        }
    }

    fprintf(f, "DIR_END\n");
    closedir(dir);
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <directory> <catalog_path>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char* directory = argv[1];
    const char* catalog_path = argv[2];

    FILE* f = fopen(catalog_path, "w");
    if (f == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    encode_dir_entries(f, directory);
    printf("Catalog saved to %s\n", catalog_path);

    fclose(f);
    return 0;
}
