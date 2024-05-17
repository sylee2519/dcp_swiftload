#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <libgen.h>
#include <lustre/lustreapi.h>
#include <limits.h>

void encode_stat_to_string(char* buffer, size_t size, const char* path, const struct stat* st) {
    snprintf(buffer, size,
             "%s\n"
             "st_dev:%ld; st_ino:%ld; st_mode:%o; st_nlink:%ld; st_uid:%d; st_gid:%d; st_rdev:%ld; st_size:%ld; st_blksize:%ld; st_blocks:%ld; st_atime:%ld; st_mtime:%ld; st_ctime:%ld\n",
             path,
             (long)st->st_dev,
             (long)st->st_ino,
             (unsigned int)st->st_mode,
             (long)st->st_nlink,
             (int)st->st_uid,
             (int)st->st_gid,
             (long)st->st_rdev,
             (long)st->st_size,
             (long)st->st_blksize,
             (long)st->st_blocks,
             (long)st->st_atime,
             (long)st->st_mtime,
             (long)st->st_ctime);
}

void process_directory(const char* directory, const char* catalog_path, FILE* f) {
    DIR* dir;
    struct dirent* entry;
    struct stat statbuf;
    struct stat lstatbuf;
    char path[PATH_MAX];
    char abs_path[PATH_MAX];
    char buffer[2048];

    // 1. the directory itself
    if (realpath(directory, abs_path) == NULL) {
        perror("realpath");
        return;
    }

    if (lstat(abs_path, &lstatbuf) == -1) {
        perror("lstat");
        return;
    }

    encode_stat_to_string(buffer, sizeof(buffer), abs_path, &lstatbuf);
    fprintf(f, "lstat\n%s", buffer);

    if (S_ISLNK(lstatbuf.st_mode)) {
        if (stat(abs_path, &statbuf) == -1) {
            fprintf(f, "stat\nNone\n");
        } else {
            encode_stat_to_string(buffer, sizeof(buffer), abs_path, &statbuf);
            fprintf(f, "stat\n%s", buffer);
        }
    } else {
        fprintf(f, "stat\n%s", buffer);
    }

    fprintf(f, "layout\n");
    if (!S_ISREG(lstatbuf.st_mode)) {
        fprintf(f, "None\n");
    } else {
        struct llapi_layout *layout;
        int rc[5];
        uint64_t count = 0, size, start, end, idx, interval, file_size;
        layout = llapi_layout_get_by_path(abs_path, 0);
        if (layout == NULL) {
            printf("errno: %d\n", errno);
            return;
        }

        file_size = lstatbuf.st_size;
        rc[0] = llapi_layout_comp_use(layout, 1);
        if (rc[0]) {
            printf("error: layout component iteration failed\n");
            llapi_layout_free(layout);
            return;
        }

        while (1) {
            rc[0] = llapi_layout_stripe_count_get(layout, &count);
            rc[1] = llapi_layout_stripe_size_get(layout, &size);
            rc[2] = llapi_layout_comp_extent_get(layout, &start, &end);
            if (rc[0] || rc[1] || rc[2]) {
                printf("error: cannot get stripe information\n");
                llapi_layout_free(layout);
                return;
            }

            interval = count * size;
            end = (end < file_size) ? end : file_size;
            int i;
            for (i = 0; i < count; i++) {
                rc[0] = llapi_layout_ost_index_get(layout, i, &idx);
                if (rc[0]) {
                    goto here_exit;
                }
                fprintf(f, "%s %lu %lu %lu %lu %lu %lu\n", abs_path, idx, start + i * size, end, interval, size, file_size);
            }
            rc[0] = llapi_layout_comp_use(layout, 3);
            if (rc[0] == 0)
                continue;
            if (rc[0] < 0)
                printf("error: layout component iteration failed\n");
            break;
        }
        here_exit:
        llapi_layout_free(layout);
    }
    fprintf(f, "end\n");
    
    // 2. subdir and file
    dir = opendir(directory);
    if (dir == NULL) {
        perror("opendir");
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        snprintf(path, sizeof(path), "%s/%s", directory, entry->d_name);

        if (realpath(path, abs_path) == NULL) {
            perror("realpath");
            continue;
        }

        if (lstat(abs_path, &lstatbuf) == -1) {
            perror("lstat");
            continue;
        }

        encode_stat_to_string(buffer, sizeof(buffer), abs_path, &lstatbuf);
        fprintf(f, "lstat\n%s", buffer);

        if (S_ISLNK(lstatbuf.st_mode)) {
            if (stat(abs_path, &statbuf) == -1) {
                fprintf(f, "stat\nNone\n");
            } else {
                encode_stat_to_string(buffer, sizeof(buffer), abs_path, &statbuf);
                fprintf(f, "stat\n%s", buffer);
            }
        } else {
            fprintf(f, "stat\n%s", buffer);
        }

        fprintf(f, "layout\n");
        if (!S_ISREG(lstatbuf.st_mode)) {
            fprintf(f, "None\n");
            if (S_ISDIR(lstatbuf.st_mode)) {
                process_directory(abs_path, catalog_path, f);
            }
            continue;
        }

        struct llapi_layout *layout;
        int rc[5];
        uint64_t count = 0, size, start, end, idx, interval, file_size;
        layout = llapi_layout_get_by_path(abs_path, 0);
        if (layout == NULL) {
            printf("errno: %d\n", errno);
            continue;
        }

        file_size = lstatbuf.st_size;
        rc[0] = llapi_layout_comp_use(layout, 1);
        if (rc[0]) {
            printf("error: layout component iteration failed\n");
            continue;
        }

        while (1) {
            rc[0] = llapi_layout_stripe_count_get(layout, &count);
            rc[1] = llapi_layout_stripe_size_get(layout, &size);
            rc[2] = llapi_layout_comp_extent_get(layout, &start, &end);
            if (rc[0] || rc[1] || rc[2]) {
                printf("error: cannot get stripe information\n");
                continue;
            }

            interval = count * size;
            end = (end < file_size) ? end : file_size;
            int i;
            for (i = 0; i < count; i++) {
                rc[0] = llapi_layout_ost_index_get(layout, i, &idx);
                if (rc[0]) {
		            goto here_exit;
                }
                fprintf(f, "%s %lu %lu %lu %lu %lu %lu\n", abs_path, idx, start + i * size, end, interval, size, file_size);
            }
            rc[0] = llapi_layout_comp_use(layout, 3);
            if (rc[0] == 0)
                continue;
            if (rc[0] < 0)
                printf("error: layout component iteration failed\n");
            break;
        }
        here_exit:
        llapi_layout_free(layout);
        fprintf(f, "end\n");
    }

    closedir(dir);
}


int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <directory> <catalog_path>\n", argv[0]);
        return 1;
    }
    FILE *f;
       f = fopen(argv[2], "w");
    if (f == NULL) {
        perror("fopen");
        return 0;
    } 

    process_directory(argv[1], argv[2], f);
    printf("Catalog saved to %s\n", argv[2]);

    fclose(f);
    return 0;
}

