#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>

void encode_dir_entries(const char *directory, FILE *catalog) {
    DIR *dir;
    struct dirent *entry;

    if (!(dir = opendir(directory))) {
        perror("opendir");
        return;
    }

    fprintf(catalog, "DIR_START %s\n", directory);

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_DIR) {
            char path[1024];
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                continue;
            }
            snprintf(path, sizeof(path), "%s/%s", directory, entry->d_name);
            fprintf(catalog, "%s\n", entry->d_name);
            encode_dir_entries(path, catalog);
        } else {
            fprintf(catalog, "%s\n", entry->d_name);
        }
    }

    fprintf(catalog, "DIR_END\n");
    closedir(dir);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <directory> <catalog_path>\n", argv[0]);
        return 1;
    }

    const char *directory = argv[1];
    const char *catalog_path = argv[2];

    FILE *catalog = fopen(catalog_path, "w");
    if (!catalog) {
        perror("fopen");
        return 1;
    }

    encode_dir_entries(directory, catalog);
    fclose(catalog);

    printf("Catalog saved to %s\n", catalog_path);
    return 0;
}
