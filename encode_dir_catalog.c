#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>

typedef struct Entry {
    char *name;
    struct Entry *next;
} Entry;

typedef struct DirEntry {
    char *path;
    Entry *entries;
    struct DirEntry *next;
} DirEntry;

Entry* create_entry(const char *name) {
    Entry *entry = (Entry *)malloc(sizeof(Entry));
    entry->name = strdup(name);
    entry->next = NULL;
    return entry;
}

void add_entry(Entry **list, Entry *entry) {
    if (*list == NULL) {
        *list = entry;
    } else {
        Entry *temp = *list;
        while (temp->next != NULL) {
            temp = temp->next;
        }
        temp->next = entry;
    }
}

DirEntry* create_dir_entry(const char *path) {
    DirEntry *dir_entry = (DirEntry *)malloc(sizeof(DirEntry));
    dir_entry->path = strdup(path);
    dir_entry->entries = NULL;
    dir_entry->next = NULL;
    return dir_entry;
}

void add_dir_entry(DirEntry **list, DirEntry *dir_entry) {
    if (*list == NULL) {
        *list = dir_entry;
    } else {
        DirEntry *temp = *list;
        while (temp->next != NULL) {
            temp = temp->next;
        }
        temp->next = dir_entry;
    }
}

void free_entries(Entry *entry) {
    while (entry) {
        Entry *next = entry->next;
        free(entry->name);
        free(entry);
        entry = next;
    }
}

void free_dir_entries(DirEntry *dir_entry) {
    while (dir_entry) {
        DirEntry *next = dir_entry->next;
        free(dir_entry->path);
        free_entries(dir_entry->entries);
        free(dir_entry);
        dir_entry = next;
    }
}

void scan_directory(const char *directory, DirEntry **dir_entries) {
    DIR *dir;
    struct dirent *entry;

    dir = opendir(directory);
    if (dir == NULL) {
        perror("opendir");
        return;
    }

    DirEntry *dir_entry = create_dir_entry(directory);
    add_dir_entry(dir_entries, dir_entry);

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char path[PATH_MAX];
        snprintf(path, sizeof(path), "%s/%s", directory, entry->d_name);

        // 로그 추가
        //printf("Scanning: %s\n", path);

        if (entry->d_type == DT_DIR) {
            add_entry(&dir_entry->entries, create_entry(entry->d_name));
            scan_directory(path, dir_entries);
        } else {
            add_entry(&dir_entry->entries, create_entry(entry->d_name));
        }
    }

    closedir(dir);
}

void write_entries(FILE *catalog, DirEntry *dir_entries) {
    for (DirEntry *dir_entry = dir_entries; dir_entry != NULL; dir_entry = dir_entry->next) {
        fprintf(catalog, "DIR_START %s\n", dir_entry->path);
        // 로그 추가
        //printf("DIR_START %s\n", dir_entry->path);

        for (Entry *entry = dir_entry->entries; entry != NULL; entry = entry->next) {
            fprintf(catalog, "%s\n", entry->name);
            // 로그 추가
            //printf("Entry: %s\n", entry->name);
        }
        fprintf(catalog, "DIR_END\n");
        // 로그 추가
        //printf("DIR_END\n");
    }
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

    DirEntry *dir_entries = NULL;
    scan_directory(directory, &dir_entries);
    write_entries(catalog, dir_entries);
    free_dir_entries(dir_entries);
    fclose(catalog);

    printf("Catalog saved to %s\n", catalog_path);
    return 0;
}
