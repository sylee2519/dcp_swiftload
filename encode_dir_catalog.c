#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>
#include <errno.h>

void encode_dir_entries(const char* directory, const char* catalog_path) {
    FILE* file = fopen(catalog_path, "w");
    if (file == NULL) {
        perror("fopen");
        return;
    }

    char path[PATH_MAX];
    DIR* dir;
    struct dirent* entry;

    // Stack for directory entries
    struct dir_stack {
        char dir_name[PATH_MAX];
        struct dir_stack* next;
    };
    struct dir_stack* stack = NULL;

    // Push the initial directory onto the stack
    struct dir_stack* initial_dir = (struct dir_stack*)malloc(sizeof(struct dir_stack));
    strncpy(initial_dir->dir_name, directory, PATH_MAX);
    initial_dir->next = stack;
    stack = initial_dir;

    while (stack != NULL) {
        struct dir_stack* current = stack;
        stack = stack->next;

        dir = opendir(current->dir_name);
        if (dir == NULL) {
            perror("opendir");
            free(current);
            continue;
        }

        fprintf(file, "DIR_START %s\n", current->dir_name);
        while ((entry = readdir(dir)) != NULL) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                continue;
            }

            snprintf(path, PATH_MAX, "%s/%s", current->dir_name, entry->d_name);
            struct stat path_stat;
            if (stat(path, &path_stat) == -1) {
                perror("stat");
                continue;
            }

            if (S_ISDIR(path_stat.st_mode)) {
                // Push directory onto stack
                struct dir_stack* new_dir = (struct dir_stack*)malloc(sizeof(struct dir_stack));
                strncpy(new_dir->dir_name, path, PATH_MAX);
                new_dir->next = stack;
                stack = new_dir;
            }

            fprintf(file, "%s\n", entry->d_name);
        }

        fprintf(file, "DIR_END\n");
        closedir(dir);
        free(current);
    }

    fclose(file);
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <directory> <catalog_path>\n", argv[0]);
        return 1;
    }

    encode_dir_entries(argv[1], argv[2]);
    printf("Catalog saved to %s\n", argv[2]);

    return 0;
}
