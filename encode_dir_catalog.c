#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>

typedef struct Entry {
    char *name;
    struct Entry *next;
    struct Entry *sub_entries;
    int is_dir;
} Entry;

Entry* create_entry(const char *name, int is_dir) {
    Entry *entry = (Entry *)malloc(sizeof(Entry));
    entry->name = strdup(name);
    entry->next = NULL;
    entry->sub_entries = NULL;
    entry->is_dir = is_dir;
    return entry;
}

void add_entry(Entry **list, Entry *entry) {
    entry->next = *list;
    *list = entry;
}

void free_entries(Entry *entry) {
    while (entry) {
        Entry *next = entry->next;
        free(entry->name);
        if (entry->is_dir) {
            free_entries(entry->sub_entries);
        }
        free(entry);
        entry = next;
    }
}

void scan_directory(const char *directory, Entry **entries) {
    DIR *dir;
    struct dirent *entry;

    if (!(dir = opendir(directory))) {
        perror("opendir");
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char path[1024];
        snprintf(path, sizeof(path), "%s/%s", directory, entry->d_name);

        if (entry->d_type == DT_DIR) {
            Entry *dir_entry = create_entry(entry->d_name, 1);
            scan_directory(path, &dir_entry->sub_entries);
            add_entry(entries, dir_entry);
        } else {
            Entry *file_entry = create_entry(entry->d_name, 0);
            add_entry(entries, file_entry);
        }
    }

    closedir(dir);
}

void write_entries(FILE *catalog, const char *directory, Entry *entries) {
    fprintf(catalog, "DIR_START %s\n", directory);

    for (Entry *entry = entries; entry != NULL; entry = entry->next) {
        if (entry->is_dir) {
            fprintf(catalog, "%s\n", entry->name);
            char path[1024];
            snprintf(path, sizeof(path), "%s/%s", directory, entry->name);
            write_entries(catalog, path, entry->sub_entries);
        } else {
            fprintf(catalog, "%s\n", entry->name);
        }
    }

    fprintf(catalog, "DIR_END\n");
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

    Entry *entries = NULL;
    scan_directory(directory, &entries);
    write_entries(catalog, directory, entries);
    free_entries(entries);
    fclose(catalog);

    printf("Catalog saved to %s\n", catalog_path);
    return 0;
}
