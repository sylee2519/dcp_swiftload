#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <unistd.h>

void encode_dir_entries(const char *directory, const char *catalog_path) {
    FILE *f = fopen(catalog_path, "w");
    if (f == NULL) {
        perror("Error opening catalog file");
        exit(EXIT_FAILURE);
    }

    DIR *dir = opendir(directory);
    if (dir == NULL) {
        perror("Error opening directory");
        fclose(f);
        exit(EXIT_FAILURE);
    }

    struct dirent *entry;
    char path[1024];
    snprintf(path, sizeof(path), "%s", directory);
    fprintf(f, "DIR_START %s\n", path);

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_DIR) {
            if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                fprintf(f, "%s\n", entry->d_name);
            }
        } else {
            fprintf(f, "%s\n", entry->d_name);
        }
    }

    fprintf(f, "DIR_END\n");
    closedir(dir);
    fclose(f);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <directory> <catalog_path>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *directory = argv[1];
    const char *catalog_path = argv[2];
    encode_dir_entries(directory, catalog_path);
    printf("Catalog saved to %s\n", catalog_path);

    return 0;
}
