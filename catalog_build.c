#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

#define MAX_FILES 4096

typedef struct {
    char path[PATH_MAX];
    struct stat info;
} FileInfo;

FileInfo files[MAX_FILES];
int file_count = 0;

void collect_files(const char *dirpath) {
    DIR *d = opendir(dirpath);
    if (d == NULL) {
        perror("opendir");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(d)) != NULL) {
        
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char filepath[PATH_MAX];
        snprintf(filepath, PATH_MAX, "%s/%s", dirpath, entry->d_name);

        struct stat file_stat;
        if (stat(filepath, &file_stat) == -1) {
            perror("stat");
            continue;
        }

        if (S_ISDIR(file_stat.st_mode)) {
            collect_files(filepath);
        } else {
            strcpy(files[file_count].path, filepath);
            files[file_count].info = file_stat;
            file_count++;
        }
    }
    closedir(d);
}

int compare(const void *a, const void *b) {
    FileInfo *fileA = (FileInfo *)a;
    FileInfo *fileB = (FileInfo *)b;
    return strcmp(fileA->path, fileB->path);
}

void write_file_info(const char *output_filename) {
    FILE *file = fopen(output_filename, "w");
    if (file == NULL) {
        perror("fopen");
        return;
    }
    qsort(files, file_count, sizeof(FileInfo), compare);

    for (int i = 0; i < file_count; i++) {
        fprintf(file, "%s\n", files[i].path);
        fprintf(file, "%lu %lu %u %lu %u %u %lu %ld %ld %ld %ld %ld %ld\n",
                (unsigned long)files[i].info.st_dev,
                (unsigned long)files[i].info.st_ino,
                (unsigned int)files[i].info.st_mode,
                (unsigned long)files[i].info.st_nlink,
                (unsigned int)files[i].info.st_uid,
                (unsigned int)files[i].info.st_gid,
                (unsigned long)files[i].info.st_rdev,
                (long)files[i].info.st_size,
                (long)files[i].info.st_blksize,
                (long)files[i].info.st_blocks,
                (long)files[i].info.st_atime,
                (long)files[i].info.st_mtime,
                (long)files[i].info.st_ctime);
    }

    fclose(file);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <directory> <output_file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *directory = argv[1];
    const char *output_file = argv[2];

    collect_files(directory);
    write_file_info(output_file);

    return EXIT_SUCCESS;
}

