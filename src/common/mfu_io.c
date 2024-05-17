#define _GNU_SOURCE

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <fcntl.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>
#include <assert.h>
#include <libgen.h>

#define LOG_FILE "./logfile.log"
#include <stdarg.h>

#include "mfu.h"
#include "mfu_errors.h"

#define MFU_IO_TRIES  (5)
#define MFU_IO_USLEEP (100)

static int mpi_rank;

static catalog_dir_t* catalog_dirs = NULL;
static size_t catalog_dir_count = 0;
static int catalog_dir_loaded = 0;

catalog_entry_t* catalog_entries = NULL;
size_t catalog_entry_count = 0;
int catalog_loaded = 0;

void log_message(const char* format, ...) {
    FILE* log_file = fopen(LOG_FILE, "a");
    if (log_file != NULL) {
        va_list args;
        va_start(args, format);
        vfprintf(log_file, format, args);
        va_end(args);
        fclose(log_file);
    }
}

int compare_stats(const struct stat* buf1, const struct stat* buf2) {
    return buf1->st_size == buf2->st_size &&
           buf1->st_mtime == buf2->st_mtime &&
           buf1->st_mode == buf2->st_mode &&
           buf1->st_uid == buf2->st_uid &&
           buf1->st_gid == buf2->st_gid &&
           buf1->st_nlink == buf2->st_nlink &&
           buf1->st_ino == buf2->st_ino &&
           buf1->st_dev == buf2->st_dev &&
           buf1->st_rdev == buf2->st_rdev;
}

void load_catalog_if_needed() {
    if (!catalog_loaded) {
        const char* catalog_path = "catalog.txt"; // TODO: catalog path 설정
        catalog_entries = load_catalog(catalog_path, &catalog_entry_count);
        if (catalog_entries != NULL) {
            catalog_loaded = 1;
        }
    }
}

void load_catalog_dir_if_needed() {
    if (!catalog_dir_loaded) {
        const char* catalog_path = "catalog_dir.txt"; // TODO: catalog path 설정
        FILE* file = fopen(catalog_path, "r");
        if (file == NULL) {
            perror("fopen");
            return;
        }

        size_t count = 0;
        char line[LINE_MAX];
        while (fgets(line, sizeof(line), file)) {
            count++;
        }

        fseek(file, 0, SEEK_SET);
        catalog_dirs = malloc((count) * sizeof(catalog_dir_t));
        if (catalog_dirs == NULL) {
            perror("malloc");
            fclose(file);
            return;
        }

        size_t index = 0;
        while (fgets(line, sizeof(line), file)) {
//            strncpy(catalog_dirs[index].dir_name, line, PATH_MAX);
//            catalog_dirs[index].dir_name[strcspn(catalog_dirs[index].dir_name, "\n")] = '\0';

            sscanf(line, "%s", catalog_dirs[index].dir_name);

//            if (fgets(line, sizeof(line), file) == NULL) {
//                break;
//            }

            catalog_dirs[index].entries = malloc(sizeof(char*));
            catalog_dirs[index].entries[0] = strdup(line);
            catalog_dirs[index].entries[0][strcspn(catalog_dirs[index].entries[0], "\n")] = '\0';
            catalog_dirs[index].entry_count = 1;
            catalog_dirs[index].current_entry = 0;

            index++;
        }

        fclose(file);
        catalog_dir_count = count;
        catalog_loaded = 1;
    }
}



catalog_entry_t* load_catalog(const char* catalog_path, size_t* out_count) {
    FILE* file = fopen(catalog_path, "r");
    if (file == NULL) {
        perror("fopen");
        return NULL;
    }

    size_t count = 0;
    char line[LINE_MAX];
    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "lstat", 5) == 0) {
            count++;
        }
    }

    fseek(file, 0, SEEK_SET);
    catalog_entry_t* entries = malloc(count * sizeof(catalog_entry_t));
    if (entries == NULL) {
        perror("malloc");
        fclose(file);
        return NULL;
    }

    size_t index = 0;
    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "lstat", 5) == 0) {
            fgets(line, sizeof(line), file);  // Read the file path line
            sscanf(line, "%[^\n]", entries[index].path);

            fgets(line, sizeof(line), file);  // Read the lstat info line
            sscanf(line,
                   "st_dev:%lu;st_ino:%lu;st_mode:%u;st_nlink:%lu;"
                   "st_uid:%u;st_gid:%u;st_rdev:%lu;st_size:%ld;"
                   "st_blksize:%ld;st_blocks:%ld;st_atime:%ld;"
                   "st_mtime:%ld;st_ctime:%ld",
                   &entries[index].lstat.st_dev,
                   &entries[index].lstat.st_ino,
                   &entries[index].lstat.st_mode,
                   &entries[index].lstat.st_nlink,
                   &entries[index].lstat.st_uid,
                   &entries[index].lstat.st_gid,
                   &entries[index].lstat.st_rdev,
                   &entries[index].lstat.st_size,
                   &entries[index].lstat.st_blksize,
                   &entries[index].lstat.st_blocks,
                   &entries[index].lstat.st_atime,
                   &entries[index].lstat.st_mtime,
                   &entries[index].lstat.st_ctime);

            fgets(line, sizeof(line), file);  // Read the "stat" line
            fgets(line, sizeof(line), file);  // Read the stat info or "None" line
            sscanf(line, "%[^\n]", entries[index].path);
            fgets(line, sizeof(line), file);  // Read the stat info or "None" line
            if (strncmp(line, "None", 4) == 0) {
                entries[index].has_stat = 0;
                memcpy(&entries[index].stat, &entries[index].lstat, sizeof(struct stat));
            } else {
                entries[index].has_stat = 1;
                sscanf(line,
                       "st_dev:%lu;st_ino:%lu;st_mode:%u;st_nlink:%lu;"
                       "st_uid:%u;st_gid:%u;st_rdev:%lu;st_size:%ld;"
                       "st_blksize:%ld;st_blocks:%ld;st_atime:%ld;"
                       "st_mtime:%ld;st_ctime:%ld",
                       &entries[index].stat.st_dev,
                       &entries[index].stat.st_ino,
                       &entries[index].stat.st_mode,
                       &entries[index].stat.st_nlink,
                       &entries[index].stat.st_uid,
                       &entries[index].stat.st_gid,
                       &entries[index].stat.st_rdev,
                       &entries[index].stat.st_size,
                       &entries[index].stat.st_blksize,
                       &entries[index].stat.st_blocks,
                       &entries[index].stat.st_atime,
                       &entries[index].stat.st_mtime,
                       &entries[index].stat.st_ctime);
            }

            fgets(line, sizeof(line), file);  // Reading "layout" line
            fgets(line, sizeof(line), file);  // Reading actual layout or "None"
            if (strncmp(line, "None", 4) == 0) {
                entries[index].has_layout = 0;
            } else {
                int cnt = 0;
                entries[index].layout = NULL;
                entries[index].has_layout = 1;
                while (strncmp(line, "end", 3) != 0) {
                    obj_task* new_task = (obj_task*)malloc(sizeof(obj_task));
                    if (!new_task) {
                        perror("malloc");
                        break;
                    }
                    sscanf(line,
                           "%s %d %lu %lu %lu %lu %lu",
                           new_task->path,
                           &new_task->ost_idx,
                           &new_task->start,
                           &new_task->end,
                           &new_task->interval,
                           &new_task->stripe_size,
                           &new_task->file_size);
                    cnt++;
                    new_task->next = entries[index].layout;
                    entries[index].layout = new_task;
                    if (!fgets(line, sizeof(line), file)) {
                        break;
                    }
                }
                entries[index].task_num = cnt;
            }

            index++;
        }
    }

    fclose(file);
    *out_count = count;

// #ifdef DEBUG
//     printf("Before sorting:\n");
//     for (size_t i = 0; i < count; i++) {
//         printf("%s\n", entries[i].path);
//     }
// #endif

    qsort(entries, count, sizeof(catalog_entry_t), compare_catalog_entry);

// #ifdef DEBUG
//     printf("After sorting:\n");
//     for (size_t i = 0; i < count; i++) {
//         printf("%s\n", entries[i].path);
//     }
// #endif

    return entries;
}


int compare_catalog_entry(const void* a, const void* b) {
    return strcmp(((catalog_entry_t*)a)->path, ((catalog_entry_t*)b)->path);
}

catalog_entry_t* find_entry_in_catalog(catalog_entry_t* entries, size_t count, const char* path) {
    catalog_entry_t key;
    strncpy(key.path, path, PATH_MAX);
#ifdef DEBUG
    printf("Searching for: %s\n", key.path);
#endif
    catalog_entry_t* result = bsearch(&key, entries, count, sizeof(catalog_entry_t), compare_catalog_entry);
#ifdef DEBUG
    if (result) {
        printf("Found entry for: %s\n", result->path);
    } else {
        printf("Entry not found for: %s\n", key.path);
    }
#endif
    return result;
}


catalog_dir_t* find_dir_in_catalog(const char* path) {
    for (size_t i = 0; i < catalog_dir_count; i++) {
        if (strcmp(catalog_dirs[i].dir_name, path) == 0) {
            return &catalog_dirs[i];
        }
    }
    return NULL;
}

/* calls access, and retries a few times if we get EIO or EINTR */
int mfu_file_access(const char* path, int amode, mfu_file_t* mfu_file)
{
    if (mfu_file->type == POSIX) {
        int rc = mfu_access(path, amode);
        return rc;
    } else if (mfu_file->type == DFS) {
        int rc = daos_access(path, amode, mfu_file);
        return rc;
    } else {
        MFU_ABORT(-1, "File type not known: %s type=%d",
                  path, mfu_file->type);
    }
}

int mfu_access(const char* path, int amode)
{
    int rc;
    int tries = MFU_IO_TRIES;
retry:
    errno = 0;
    rc = access(path, amode);
    if (rc != 0) {
        if (errno == EINTR || errno == EIO) {
            tries--;
            if (tries > 0) {
                /* sleep a bit before consecutive tries */
                usleep(MFU_IO_USLEEP);
                goto retry;
            }
        }
    }
    return rc;
}

int daos_access(const char* path, int amode, mfu_file_t* mfu_file)
{
#ifdef DAOS_SUPPORT
    int rc = dfs_sys_access(mfu_file->dfs_sys, path, amode, 0);
    return mfu_errno2rc(rc);
#else
    return mfu_errno2rc(ENOSYS);
#endif
}

/* calls faccessat, and retries a few times if we get EIO or EINTR */
int mfu_file_faccessat(int dirfd, const char* path, int amode, int flags, mfu_file_t* mfu_file)
{
    if (mfu_file->type == POSIX) {
        int rc = mfu_faccessat(dirfd, path, amode, flags);
        return rc;
    } else if (mfu_file->type == DFS) {
        int rc = daos_faccessat(dirfd, path, amode, flags, mfu_file);
        return rc;
    } else {
        MFU_ABORT(-1, "File type not known: %s type=%d",
                  path, mfu_file->type);
    } 
}

int mfu_faccessat(int dirfd, const char* path, int amode, int flags)
{
    int rc;
    int tries = MFU_IO_TRIES;
retry:
    errno = 0;
    rc = faccessat(dirfd, path, amode, flags);
    if (rc != 0) {
        if (errno == EINTR || errno == EIO) {
            tries--;
            if (tries > 0) {
                /* sleep a bit before consecutive tries */
                usleep(MFU_IO_USLEEP);
                goto retry;
            }
        }
    }
    return rc;
}

/* Emulates faccessat for a DAOS path */
int daos_faccessat(int dirfd, const char* path, int amode, int flags, mfu_file_t* mfu_file)
{
#ifdef DAOS_SUPPORT
    /* Only current working directory supported at this time */
    if (dirfd != AT_FDCWD) {
        return mfu_errno2rc(ENOTSUP);
    }

    int access_flags = (flags & AT_SYMLINK_NOFOLLOW) ? O_NOFOLLOW : 0;
    int rc = dfs_sys_access(mfu_file->dfs_sys, path, amode, access_flags);
    return mfu_errno2rc(rc);
#else
    return mfu_errno2rc(ENOSYS);
#endif
}

/* calls lchown, and retries a few times if we get EIO or EINTR */
int mfu_file_lchown(const char* path, uid_t owner, gid_t group, mfu_file_t* mfu_file)
{
    if (mfu_file->type == POSIX) {
        int rc = mfu_lchown(path, owner, group);
        return rc;
    } else if (mfu_file->type == DFS) {
        int rc = daos_lchown(path, owner, group, mfu_file);
        return rc;
    } else {
        MFU_ABORT(-1, "File type not known: %s type=%d",
                  path, mfu_file->type);
    }    
}

int mfu_lchown(const char* path, uid_t owner, gid_t group)
{
    int rc;
    int tries = MFU_IO_TRIES;
retry:
    errno = 0;
    rc = lchown(path, owner, group);
    if (rc != 0) {
        if (errno == EINTR || errno == EIO) {
            tries--;
            if (tries > 0) {
                /* sleep a bit before consecutive tries */
                usleep(MFU_IO_USLEEP);
                goto retry;
            }
        }
    }
    return rc;
}

int daos_lchown(const char* path, uid_t owner, gid_t group, mfu_file_t* mfu_file)
{
    /* At this time, DFS does not support updating the uid or gid.
     * These are set at the container level, not file level */
    return mfu_errno2rc(0);
}

int daos_chmod(const char *path, mode_t mode, mfu_file_t* mfu_file)
{
#ifdef DAOS_SUPPORT
    int rc = dfs_sys_chmod(mfu_file->dfs_sys, path, mode);
    return mfu_errno2rc(rc);
#else
    return mfu_errno2rc(ENOSYS);
#endif
}

int mfu_chmod(const char* path, mode_t mode)
{
    int rc;
    int tries = MFU_IO_TRIES;
retry:
    errno = 0;
    rc = chmod(path, mode);
    if (rc != 0) {
        if (errno == EINTR || errno == EIO) {
            tries--;
            if (tries > 0) {
                /* sleep a bit before consecutive tries */
                usleep(MFU_IO_USLEEP);
                goto retry;
            }
        }
    }
    return rc;
}

/* calls chmod, and retries a few times if we get EIO or EINTR */
int mfu_file_chmod(const char* path, mode_t mode, mfu_file_t* mfu_file)
{
    if (mfu_file->type == POSIX) {
        int rc = mfu_chmod(path, mode);
        return rc;
    } else if (mfu_file->type == DFS) {
        int rc = daos_chmod(path, mode, mfu_file);
        return rc;
    } else {
        MFU_ABORT(-1, "File type not known: %s type=%d",
                  path, mfu_file->type);
    }
}

/* calls utimensat, and retries a few times if we get EIO or EINTR */
int mfu_file_utimensat(int dirfd, const char* pathname, const struct timespec times[2], int flags,
                       mfu_file_t* mfu_file)
{
    if (mfu_file->type == POSIX) {
        int rc = mfu_utimensat(dirfd, pathname, times, flags);
        return rc;
    } else if (mfu_file->type == DFS) {
        int rc = daos_utimensat(dirfd, pathname, times, flags, mfu_file);
        return rc;
    } else {
        MFU_ABORT(-1, "File type not known: %s type=%d",
                  pathname, mfu_file->type);
    }
}

int mfu_utimensat(int dirfd, const char* pathname, const struct timespec times[2], int flags)
{
    int rc;
    int tries = MFU_IO_TRIES;
retry:
    errno = 0;
    rc = utimensat(dirfd, pathname, times, flags);
    if (rc != 0) {
        if (errno == EINTR || errno == EIO) {
            tries--;
            if (tries > 0) {
                /* sleep a bit before consecutive tries */
                usleep(MFU_IO_USLEEP);
                goto retry;
            }
        }
    }
    return rc;
}

/* Emulates utimensat by calling dfs_osetattr */
int daos_utimensat(int dirfd, const char* pathname, const struct timespec times[2], int flags,
                   mfu_file_t* mfu_file)
{
#ifdef DAOS_SUPPORT
    /* Only current working directory supported at this time */
    if (dirfd != AT_FDCWD) {
        return mfu_errno2rc(ENOTSUP);
    }

    int time_flags = (flags & AT_SYMLINK_NOFOLLOW) ? O_NOFOLLOW : 0;
    int rc = dfs_sys_utimens(mfu_file->dfs_sys, pathname, times, time_flags);
    return mfu_errno2rc(rc);
#else
    return mfu_errno2rc(ENOSYS);
#endif
}

int daos_stat(const char* path, struct stat* buf, mfu_file_t* mfu_file)
{
#ifdef DAOS_SUPPORT
    int rc = dfs_sys_stat(mfu_file->dfs_sys, path, 0, buf);
    return mfu_errno2rc(rc);
#else
    return mfu_errno2rc(ENOSYS);
#endif
}


int mfu_stat(const char* path, struct stat* buf) {
    load_catalog_if_needed();

    if (catalog_loaded) {
        catalog_entry_t* entry = find_entry_in_catalog(catalog_entries, catalog_entry_count, path);
        if (entry != NULL) {
#ifdef DEBUG
            log_message("mfu_stat: Found %s in catalog\n", path);
#endif
            if (entry->has_stat) {
                memcpy(buf, &entry->stat, sizeof(struct stat));
            } else {
                memcpy(buf, &entry->lstat, sizeof(struct stat));
            }
#ifdef DEBUG
            log_message("mfu_stat: catalog stat data: st_size=%ld, st_mtime=%ld, st_mode=%o, st_uid=%u, st_gid=%u, st_nlink=%lu, st_ino=%lu, st_dev=%lu, st_rdev=%lu\n",
                buf->st_size, buf->st_mtime, buf->st_mode, buf->st_uid, buf->st_gid, buf->st_nlink, buf->st_ino, buf->st_dev, buf->st_rdev);
#endif

            struct stat actual_stat;
            if (stat(path, &actual_stat) == 0) {
#ifdef DEBUG
                log_message("mfu_stat: actual stat data: st_size=%ld, st_mtime=%ld, st_mode=%o, st_uid=%u, st_gid=%u, st_nlink=%lu, st_ino=%lu, st_dev=%lu, st_rdev=%lu\n",
                    actual_stat.st_size, actual_stat.st_mtime, actual_stat.st_mode, actual_stat.st_uid, actual_stat.st_gid, actual_stat.st_nlink, actual_stat.st_ino, actual_stat.st_dev, actual_stat.st_rdev);
#endif
                if (!compare_stats(buf, &actual_stat)) {
#ifdef DEBUG
                    log_message("mfu_stat: catalog and actual stat data mismatch for %s\n", path);
#endif
                }
            }
            return 0;
        }
        else {
#ifdef DEBUG
            log_message("mfu_stat: %s not found in catalog\n", path);
#endif
        }
    }

    int rc;
    int tries = MFU_IO_TRIES;
retry:
    errno = 0;
    rc = stat(path, buf);
    if (rc != 0) {
        if (errno == EINTR || errno == EIO) {
            tries--;
            if (tries > 0) {
                /* sleep a bit before consecutive tries */
                usleep(MFU_IO_USLEEP);
                goto retry;
            }
        }
    }
    return rc;
}

/* calls stat, and retries a few times if we get EIO or EINTR */
int mfu_file_stat(const char* path, struct stat* buf, mfu_file_t* mfu_file)
{
    if (mfu_file->type == POSIX) {
        int rc = mfu_stat(path, buf);
        return rc;
    } else if (mfu_file->type == DFS) {
        int rc = daos_stat(path, buf, mfu_file);
        return rc;
    } else {
        MFU_ABORT(-1, "File type not known: %s type=%d",
                  path, mfu_file->type);
    }
}

/* lstat a DAOS path */
int daos_lstat(const char* path, struct stat* buf, mfu_file_t* mfu_file)
{
#ifdef DAOS_SUPPORT
    int rc =  dfs_sys_stat(mfu_file->dfs_sys, path, O_NOFOLLOW, buf);
    return mfu_errno2rc(rc);
#else
    return mfu_errno2rc(ENOSYS);
#endif
}

int mfu_lstat(const char* path, struct stat* buf) {
    load_catalog_if_needed();

    if (catalog_loaded) {
        catalog_entry_t* entry = find_entry_in_catalog(catalog_entries, catalog_entry_count, path);
        if (entry != NULL) {
#ifdef DEBUG
            log_message("mfu_lstat: Found %s in catalog\n", path);
#endif
            memcpy(buf, &entry->lstat, sizeof(struct stat));
#ifdef DEBUG
            log_message("mfu_lstat: catalog lstat data: st_size=%ld, st_mtime=%ld, st_mode=%o, st_uid=%u, st_gid=%u, st_nlink=%lu, st_ino=%lu, st_dev=%lu, st_rdev=%lu\n",
                buf->st_size, buf->st_mtime, buf->st_mode, buf->st_uid, buf->st_gid, buf->st_nlink, buf->st_ino, buf->st_dev, buf->st_rdev);
#endif

            struct stat actual_lstat;
            if (lstat(path, &actual_lstat) == 0) {
#ifdef DEBUG
                log_message("mfu_lstat: actual lstat data: st_size=%ld, st_mtime=%ld, st_mode=%o, st_uid=%u, st_gid=%u, st_nlink=%lu, st_ino=%lu, st_dev=%lu, st_rdev=%lu\n",
                    actual_lstat.st_size, actual_lstat.st_mtime, actual_lstat.st_mode, actual_lstat.st_uid, actual_lstat.st_gid, actual_lstat.st_nlink, actual_lstat.st_ino, actual_lstat.st_dev, actual_lstat.st_rdev);
#endif
                if (!compare_stats(buf, &actual_lstat)) {
#ifdef DEBUG
                    log_message("mfu_lstat: catalog and actual lstat data mismatch for %s\n", path);
#endif
                }
            }
            return 0;
        }
        else {
#ifdef DEBUG
            log_message("mfu_lstat: %s not found in catalog\n", path);
#endif
        }
    }

    int rc;
    int tries = MFU_IO_TRIES;
retry:
    errno = 0;
    rc = lstat(path, buf);
    if (rc != 0) {
        if (errno == EINTR || errno == EIO) {
            tries--;
            if (tries > 0) {
                /* sleep a bit before consecutive tries */
                usleep(MFU_IO_USLEEP);
                goto retry;
            }
        }
    }
    return rc;
}

/* calls lstat, and retries a few times if we get EIO or EINTR */
int mfu_file_lstat(const char* path, struct stat* buf, mfu_file_t* mfu_file)
{
    if (mfu_file->type == POSIX) {
        int rc = mfu_lstat(path, buf);
        return rc;
    } else if (mfu_file->type == DFS) {
        int rc = daos_lstat(path, buf, mfu_file);
        return rc;
    } else {
        MFU_ABORT(-1, "File type not known: %s type=%d",
                  path, mfu_file->type);
    }
}

/* calls lstat64, and retries a few times if we get EIO or EINTR */
int mfu_lstat64(const char* path, struct stat64* buf)
{
    int rc;
    int tries = MFU_IO_TRIES;
retry:
    errno = 0;
    rc = lstat64(path, buf);
    if (rc != 0) {
        if (errno == EINTR || errno == EIO) {
            tries--;
            if (tries > 0) {
                /* sleep a bit before consecutive tries */
                usleep(MFU_IO_USLEEP);
                goto retry;
            }
        }
    }
    return rc;
}

int daos_mknod(const char* path, mode_t mode, mfu_file_t* mfu_file)
{
#ifdef DAOS_SUPPORT
    int rc = dfs_sys_mknod(mfu_file->dfs_sys, path, mode, 0, 0);
    return mfu_errno2rc(rc);
#else
    return mfu_errno2rc(ENOSYS);
#endif
}

int mfu_mknod(const char* path, mode_t mode, dev_t dev)
{
    int rc;
    int tries = MFU_IO_TRIES;
retry:
    errno = 0;
    rc = mknod(path, mode, dev);
    if (rc < 0) {
        if (errno == EINTR || errno == EIO) {
            tries--;
            if (tries > 0) {
                /* sleep a bit before consecutive tries */
                usleep(MFU_IO_USLEEP);
                goto retry;
            }
        }
    }
    return rc;
}

/* call mknod, retry a few times on EINTR or EIO */
int mfu_file_mknod(const char* path, mode_t mode, dev_t dev, mfu_file_t* mfu_file)
{
    if (mfu_file->type == POSIX) {
        int rc = mfu_mknod(path, mode, dev);
        return rc;
    } else if (mfu_file->type == DFS) {
        int rc = daos_mknod(path, mode, mfu_file);
        return rc;
    } else {
        MFU_ABORT(-1, "File type not known: %s type=%d",
                  path, mfu_file->type);
    }
}

int mfu_file_remove(const char* path, mfu_file_t* mfu_file)
{
    if (mfu_file->type == POSIX) {
        int rc = mfu_remove(path);
        return rc;
    } else if (mfu_file->type == DFS) {
        int rc = daos_remove(path, mfu_file);
        return rc;
    } else {
        MFU_ABORT(-1, "File type not known: %s type=%d",
                  path, mfu_file->type);
    }
}

/* call remove, retry a few times on EINTR or EIO */
int mfu_remove(const char* path)
{
    int rc;
    int tries = MFU_IO_TRIES;
retry:
    errno = 0;
    rc = remove(path);
    if (rc < 0) {
        if (errno == EINTR || errno == EIO) {
            tries--;
            if (tries > 0) {
                /* sleep a bit before consecutive tries */
                usleep(MFU_IO_USLEEP);
                goto retry;
            }
        }
    }
    return rc;
}

int daos_remove(const char* path, mfu_file_t* mfu_file)
{
#ifdef DAOS_SUPPORT
    int rc = dfs_sys_remove(mfu_file->dfs_sys, path, false, NULL);
    return mfu_errno2rc(rc);
#else
    return mfu_errno2rc(ENOSYS);
#endif
}

/* calls realpath */
char* mfu_file_realpath(const char* path, char* resolved_path, mfu_file_t* mfu_file)
{
    if (mfu_file->type == POSIX) {
        char* p = mfu_realpath(path, resolved_path);
        return p;
    } else if (mfu_file->type == DFS) {
        char* p = daos_realpath(path, resolved_path, mfu_file);
        return p;
    } else {
        MFU_ABORT(-1, "File type not known: %s type=%d",
                  path, mfu_file->type);
    }
}

char* mfu_realpath(const char* path, char* resolved_path)
{
    char* p = realpath(path, resolved_path);
    return p;
}

char* daos_realpath(const char* path, char* resolved_path, mfu_file_t* mfu_file)
{
#ifdef DAOS_SUPPORT
    /* There is currently not a reasonable way to do this */
    return NULL;
#else
    errno = ENOSYS;
    return NULL;
#endif
}

/*****************************
 * Links
 ****************************/

ssize_t daos_readlink(const char* path, char* buf, size_t bufsize, mfu_file_t* mfu_file)
{
#ifdef DAOS_SUPPORT
    daos_size_t got_size = bufsize;
    int rc = dfs_sys_readlink(mfu_file->dfs_sys, path, buf, &got_size);
    if (rc != 0) {
        errno = rc;
    }
    return (ssize_t) got_size;
#else
    return (ssize_t) mfu_errno2rc(ENOSYS);
#endif
}

/* call readlink, retry a few times on EINTR or EIO */
ssize_t mfu_readlink(const char* path, char* buf, size_t bufsize)
{
    ssize_t rc;
    int tries = MFU_IO_TRIES;
retry:
    errno = 0;
    rc = readlink(path, buf, bufsize);
    if (rc < 0) {
        if (errno == EINTR || errno == EIO) {
            tries--;
            if (tries > 0) {
                /* sleep a bit before consecutive tries */
                usleep(MFU_IO_USLEEP);
                goto retry;
            }
        }
    }
    return rc;
}

ssize_t mfu_file_readlink(const char* path, char* buf, size_t bufsize, mfu_file_t* mfu_file)
{
    int rc;

    if (mfu_file->type == POSIX) {
        rc = mfu_readlink(path, buf, bufsize);
    } else if (mfu_file->type == DFS) {
        rc = daos_readlink(path, buf, bufsize, mfu_file);
    } else {
        MFU_ABORT(-1, "File type not known: %s type=%d",
                  path, mfu_file->type);
    }

    return rc;
}

/* emulates symlink for a DAOS symlink */
int daos_symlink(const char* oldpath, const char* newpath, mfu_file_t* mfu_file)
{
#ifdef DAOS_SUPPORT
    int rc = dfs_sys_symlink(mfu_file->dfs_sys, oldpath, newpath);
    return mfu_errno2rc(rc);
#else
    return mfu_errno2rc(ENOSYS);
#endif
}

/* call symlink, retry a few times on EINTR or EIO */
int mfu_symlink(const char* oldpath, const char* newpath)
{
    int rc;
    int tries = MFU_IO_TRIES;
retry:
    errno = 0;
    rc = symlink(oldpath, newpath);
    if (rc < 0) {
        if (errno == EINTR || errno == EIO) {
            tries--;
            if (tries > 0) {
                /* sleep a bit before consecutive tries */
                usleep(MFU_IO_USLEEP);
                goto retry;
            }
        }
    }
    return rc;
}

int mfu_file_symlink(const char* oldpath, const char* newpath, mfu_file_t* mfu_file)
{
    int rc;

    if (mfu_file->type == POSIX) {
        rc = mfu_symlink(oldpath, newpath);
    } else if (mfu_file->type == DFS) {
        rc = daos_symlink(oldpath, newpath, mfu_file);
    } else {
        MFU_ABORT(-1, "File type not known: %s type=%d",
                  oldpath, mfu_file->type);
    }

    return rc;
}

/* call hardlink, retry a few times on EINTR or EIO */
int mfu_hardlink(const char* oldpath, const char* newpath)
{
    int rc;
    int tries = MFU_IO_TRIES;
retry:
    errno = 0;
    rc = link(oldpath, newpath);
    if (rc < 0) {
        if (errno == EINTR || errno == EIO) {
            tries--;
            if (tries > 0) {
                /* sleep a bit before consecutive tries */
                usleep(MFU_IO_USLEEP);
                goto retry;
            }
        }
    }
    return rc;
}

/*****************************
 * Files
 ****************************/
int daos_open(const char* file, int flags, mode_t mode, mfu_file_t* mfu_file)
{
#ifdef DAOS_SUPPORT
    int rc = dfs_sys_open(mfu_file->dfs_sys, file, mode, flags, 0, 0, NULL, &(mfu_file->obj));
    return mfu_errno2rc(rc);
#else
    return mfu_errno2rc(ENOSYS);
#endif
}

/* open file with specified flags and mode, retry open a few times on failure */
int mfu_open(const char* file, int flags, ...)
{
    /* extract the mode (see man 2 open) */
    int mode_set = 0;
    mode_t mode = 0;
    if (flags & O_CREAT) {
        va_list ap;
        va_start(ap, flags);
        mode = va_arg(ap, mode_t);
        va_end(ap);
        mode_set = 1;
    }

    /* attempt to open file */
    int fd = -1;
    errno = 0;
    if (mode_set) {
        fd = open(file, flags, mode);
    }
    else {
        fd = open(file, flags);
    }

    /* if open failed, try a few more times */
    if (fd < 0) {
        /* try again */
        int tries = MFU_IO_TRIES;
        while (tries && fd < 0) {
            /* sleep a bit before consecutive tries */
            usleep(MFU_IO_USLEEP);

            /* open again */
            errno = 0;
            if (mode_set) {
                fd = open(file, flags, mode);
            }
            else {
                fd = open(file, flags);
            }
            tries--;
        }

         /* if we still don't have a valid file, consider it an error */
         if (fd < 0) {
             /* we could abort, but probably don't want to here */
         }
    }
    return fd;
}

/* Open a file.
 * Return 0 on success, -1 on error */
int mfu_file_open(const char* file, int flags, mfu_file_t* mfu_file, ...)
{
    /* extract the mode (see man 2 open) */
    int mode_set = 0;
    mode_t mode  = 0;
    if (flags & O_CREAT) {
        va_list ap;
        va_start(ap, mfu_file);
        mode = va_arg(ap, mode_t);
        va_end(ap);
        mode_set = 1;
    }

    int rc = 0;

    if (mfu_file->type == POSIX) {
        if (mode_set) {
            mfu_file->fd = mfu_open(file, flags, mode);
        } else {
            mfu_file->fd = mfu_open(file, flags);
        }
        if (mfu_file->fd < 0) {
            rc = -1;
        }
    } else if (mfu_file->type == DFS) {
        daos_open(file, flags, mode, mfu_file);
#ifdef DAOS_SUPPORT
        if (mfu_file->obj == NULL) {
            rc = -1;
        }
#endif
    } else {
        MFU_ABORT(-1, "File type not known: %s type=%d",
                  file, mfu_file->type);
    }

    return rc;
}

/* release an open object */
int daos_close(const char* file, mfu_file_t* mfu_file)
{
#ifdef DAOS_SUPPORT
    int rc = dfs_sys_close(mfu_file->obj);
    if (rc == 0) {
        mfu_file->obj = NULL;
    }
    return mfu_errno2rc(rc);
#else
    return mfu_errno2rc(ENOSYS);
#endif
}

/* close file */
int mfu_close(const char* file, int fd)
{
    int tries = MFU_IO_TRIES;
retry:
    errno = 0;
    int rc = close(fd);
    if (rc != 0) {
        if (errno == EINTR || errno == EIO) {
            tries--;
            if (tries > 0) {
                /* sleep a bit before consecutive tries */
                usleep(MFU_IO_USLEEP);
                goto retry;
            }
        }
    }
    return rc;
}

int mfu_file_close(const char* file, mfu_file_t* mfu_file)
{
    if (mfu_file->type == POSIX) {
        int rc = mfu_close(file, mfu_file->fd);
        if (rc == 0) {
            mfu_file->fd = -1;
        }
        return rc;
    } else if (mfu_file->type == DFS) {
        int rc = daos_close(file, mfu_file);
        return rc;
    } else {
        MFU_ABORT(-1, "File type not known: %s type=%d",
                  file, mfu_file->type);
    }
}

int daos_lseek(const char* file, mfu_file_t* mfu_file, off_t pos, int whence)
{
#ifdef DAOS_SUPPORT
    if (whence == SEEK_SET) {
        mfu_file->offset = (daos_off_t)pos;
    } else {
        MFU_ABORT(-1, "daos_lseek whence type not known: %d", whence);
    }
    return 0;
#else
    return mfu_errno2rc(ENOSYS);
#endif
}

/* seek file descriptor to specified position */
off_t mfu_lseek(const char* file, int fd, off_t pos, int whence)
{
    int tries = MFU_IO_TRIES;
retry:
    errno = 0;
    off_t rc = lseek(fd, pos, whence);
    if (rc == (off_t)-1) {
        if (errno == EINTR || errno == EIO) {
            tries--;
            if (tries > 0) {
                /* sleep a bit before consecutive tries */
                usleep(MFU_IO_USLEEP);
                goto retry;
            }
        }
    }
    return rc;
}

off_t mfu_file_lseek(const char* file, mfu_file_t* mfu_file, off_t pos, int whence)
{
    if (mfu_file->type == POSIX) {
        off_t rc = mfu_lseek(file, mfu_file->fd, pos, whence);
        return rc;
    } else if (mfu_file->type == DFS) {
        off_t rc = daos_lseek(file, mfu_file, pos, whence);
        return rc;
    } else {
        MFU_ABORT(-1, "File type not known: %s type=%d",
                  file, mfu_file->type);
    }
}

/* reliable read from file descriptor (retries, if necessary, until hard error) */
ssize_t mfu_file_read(const char* file, void* buf, size_t size, mfu_file_t* mfu_file)
{
    if (mfu_file->type == POSIX) {
        ssize_t got_size = mfu_read(file, mfu_file->fd, buf, size);
        return got_size;
    } else if (mfu_file->type == DFS) {
        ssize_t got_size = daos_read(file, buf, size, mfu_file);
        return got_size;
    } else {
        MFU_ABORT(-1, "File type not known: %s type=%d",
                  file, mfu_file->type);
    }
}

ssize_t daos_read(const char* file, void* buf, size_t size, mfu_file_t* mfu_file)
{
#ifdef DAOS_SUPPORT
    daos_size_t got_size = size;
    int rc = dfs_sys_read(mfu_file->dfs_sys, mfu_file->obj, buf, mfu_file->offset, &got_size, NULL);
    if (rc != 0) {
        errno = rc;
        return -1;
    } else {
        /* update file pointer with number of bytes read */
        mfu_file->offset += (daos_off_t)got_size;
    }
    return (ssize_t)got_size;
#else
    return (ssize_t)mfu_errno2rc(ENOSYS);
#endif
}

ssize_t mfu_read(const char* file, int fd, void* buf, size_t size)
{
    int tries = MFU_IO_TRIES;
    ssize_t n = 0;
    while ((size_t)n < size) {
        errno = 0;
        ssize_t rc = read(fd, (char*) buf + n, size - (size_t)n);
        if (rc > 0) {
            /* read some data */
            n += rc;
            tries = MFU_IO_TRIES;

            /* return, even if we got a short read */
            return n;
        }
        else if (rc == 0) {
            /* EOF */
            return n;
        }
        else {   /* (rc < 0) */
            /* something worth printing an error about */
            tries--;
            if (tries <= 0) {
                /* too many failed retries, give up */
                MFU_ABORT(-1, "Failed to read file %s errno=%d (%s)",
                            file, errno, strerror(errno)
                           );
            }

            /* sleep a bit before consecutive tries */
            usleep(MFU_IO_USLEEP);
        }
    }
    return n;
}

/* reliable write to file descriptor (retries, if necessary, until hard error) */
ssize_t mfu_file_write(const char* file, const void* buf, size_t size, mfu_file_t* mfu_file)
{
    if (mfu_file->type == POSIX) {
        ssize_t num_bytes_written = mfu_write(file, mfu_file->fd, buf, size);
        return num_bytes_written;
    } else if (mfu_file->type == DFS) {
        ssize_t num_bytes_written = daos_write(file, buf, size, mfu_file);
        return num_bytes_written;
    } else {
        MFU_ABORT(-1, "File type not known: %s type=%d",
                  file, mfu_file->type);
    }
}

ssize_t mfu_write(const char* file, int fd, const void* buf, size_t size)
{
    int tries = MFU_IO_TRIES;
    ssize_t n = 0;
    while ((size_t)n < size) {
        errno = 0;
        ssize_t rc = write(fd, (const char*) buf + n, size - (size_t)n);
        if (rc > 0) {
            /* wrote some data */
            n += rc;
            tries = MFU_IO_TRIES;
        }
        else if (rc == 0) {
            /* something bad happened, print an error and abort */
            MFU_ABORT(-1, "Failed to write file %s errno=%d (%s)",
                        file, errno, strerror(errno)
                       );
        }
        else {   /* (rc < 0) */
            /* something worth printing an error about */
            tries--;
            if (tries <= 0) {
                /* too many failed retries, give up */
                MFU_ABORT(-1, "Failed to write file %s errno=%d (%s)",
                            file, errno, strerror(errno)
                           );
            }

            /* sleep a bit before consecutive tries */
            usleep(MFU_IO_USLEEP);
        }
    }
    return n;
}

ssize_t daos_write(const char* file, const void* buf, size_t size, mfu_file_t* mfu_file)
{
#ifdef DAOS_SUPPORT
    daos_size_t write_size = size;
    int rc = dfs_sys_write(mfu_file->dfs_sys, mfu_file->obj, buf, mfu_file->offset, &write_size, NULL);
    if (rc != 0) {
        errno = rc;
        return -1;
    } else {
        /* update file pointer with number of bytes written */
        mfu_file->offset += write_size;
    }
    return (ssize_t)write_size;
#else
    return (ssize_t)mfu_errno2rc(ENOSYS);
#endif
}

/* reliable pread from file descriptor (retries, if necessary, until hard error) */
ssize_t mfu_file_pread(const char* file, void* buf, size_t size, off_t offset, mfu_file_t* mfu_file)
{
    if (mfu_file->type == POSIX) {
        ssize_t rc = mfu_pread(file, mfu_file->fd, buf, size, offset);
        return rc;
    } else if (mfu_file->type == DFS) {
        ssize_t rc = daos_pread(file, buf, size, offset, mfu_file);
        return rc;
    } else {
        MFU_ABORT(-1, "File type not known: %s type=%d",
            file, mfu_file->type);
    }
}

ssize_t daos_pread(const char* file, void* buf, size_t size, off_t offset, mfu_file_t* mfu_file)
{
#ifdef DAOS_SUPPORT
    daos_size_t got_size = size;
    int rc = dfs_sys_read(mfu_file->dfs_sys, mfu_file->obj, buf, offset, &got_size, NULL);
    if (rc != 0) {
        errno = rc;
        /* return -1 if dfs_sys_read encounters error */
        return -1;
    }
    return (ssize_t)got_size;
#else
    return (ssize_t)mfu_errno2rc(ENOSYS);
#endif
}

ssize_t mfu_pread(const char* file, int fd, void* buf, size_t size, off_t offset)
{
    int tries = MFU_IO_TRIES;
    while (1) {
        ssize_t rc = pread(fd, (char*) buf, size, offset);
        if (rc > 0) {
            /* read some data */
            return rc;
        }
        else if (rc == 0) {
            /* EOF */
            return rc;
        }
        else {   /* (rc < 0) */
            /* something worth printing an error about */
            tries--;
            if (tries <= 0) {
                /* too many failed retries, give up */
                MFU_ABORT(-1, "Failed to read file %s errno=%d (%s)",
                    file, errno, strerror(errno));
            }

            /* sleep a bit before consecutive tries */
            usleep(MFU_IO_USLEEP);
        }
    }
}

ssize_t mfu_file_pwrite(const char* file, const void* buf, size_t size, off_t offset, mfu_file_t* mfu_file)
{
    if (mfu_file->type == POSIX) {
        ssize_t rc = mfu_pwrite(file, mfu_file->fd, buf, size, offset);
        return rc;
    } else if (mfu_file->type == DFS) {
        ssize_t rc = daos_pwrite(file, buf, size, offset, mfu_file);
        return rc;
    } else {
        MFU_ABORT(-1, "File type not known: %s type=%d",
            file, mfu_file->type);
    }
}

ssize_t mfu_pwrite(const char* file, int fd, const void* buf, size_t size, off_t offset)
{
    int tries = MFU_IO_TRIES;
    while (1) {
        ssize_t rc = pwrite(fd, (const char*) buf, size, offset);
        if (rc > 0) {
            /* wrote some data */
            return rc;
        }
        else if (rc == 0) {
            /* didn't write anything, but not an error either */
            return rc;
        }
        else { /* (rc < 0) */
            /* something worth printing an error about */
            tries--;
            if (tries <= 0) {
                /* too many failed retries, give up */
                MFU_ABORT(-1, "Failed to write file %s errno=%d (%s)",
                    file, errno, strerror(errno));
            }

            /* sleep a bit before consecutive tries */
            usleep(MFU_IO_USLEEP);
        }
    }
}

ssize_t daos_pwrite(const char* file, const void* buf, size_t size, off_t offset, mfu_file_t* mfu_file)
{
#ifdef DAOS_SUPPORT
    daos_size_t write_size = size;
    int rc = dfs_sys_write(mfu_file->dfs_sys, mfu_file->obj, buf, offset, &write_size, NULL);
    if (rc != 0) {
        errno = rc;
        /* report -1 if dfs_sys_write encounters error */
        return -1;
    }
    return (ssize_t)write_size;
#else
    return (ssize_t)mfu_errno2rc(ENOSYS);
#endif
}

/* truncate a file */
int mfu_file_truncate(const char* file, off_t length, mfu_file_t* mfu_file)
{
    if (mfu_file->type == POSIX) {
        int rc = mfu_truncate(file, length);
        return rc;
    } else if (mfu_file->type == DFS) {
        int rc = daos_truncate(file, length, mfu_file);
        return rc;
    } else {
        MFU_ABORT(-1, "File type not known, type=%d",
                  mfu_file->type);
    }
}

/* truncate a file */
int mfu_truncate(const char* file, off_t length)
{
    int rc;
    int tries = MFU_IO_TRIES;
retry:
    errno = 0;
    rc = truncate(file, length);
    if (rc != 0) {
        if (errno == EINTR || errno == EIO) {
            tries--;
            if (tries > 0) {
                /* sleep a bit before consecutive tries */
                usleep(MFU_IO_USLEEP);
                goto retry;
            }
        }
    }
    return rc;
}

int daos_truncate(const char* file, off_t length, mfu_file_t* mfu_file)
{
#ifdef DAOS_SUPPORT
    int rc = dfs_sys_punch(mfu_file->dfs_sys, file, length, DFS_MAX_FSIZE);
    return mfu_errno2rc(rc);
#else
    return mfu_errno2rc(ENOSYS);
#endif
}

int daos_ftruncate(mfu_file_t* mfu_file, off_t length)
{
#ifdef DAOS_SUPPORT
    int rc = dfs_punch(mfu_file->dfs, mfu_file->obj, length, DFS_MAX_FSIZE);
    return mfu_errno2rc(rc);
#else
    return mfu_errno2rc(ENOSYS);
#endif
}

int mfu_ftruncate(int fd, off_t length)
{
    int rc;
    int tries = MFU_IO_TRIES;
retry:
    errno = 0;
    rc = ftruncate(fd, length);
    if (rc != 0) {
        if (errno == EINTR || errno == EIO) {
            tries--;
            if (tries > 0) {
                /* sleep a bit before consecutive tries */
                usleep(MFU_IO_USLEEP);
                goto retry;
            }
        }
    }
    return rc;
}

/* ftruncate a file */
int mfu_file_ftruncate(mfu_file_t* mfu_file, off_t length)
{
    if (mfu_file->type == POSIX) {
        int rc = mfu_ftruncate(mfu_file->fd, length);
        return rc;
    } else if (mfu_file->type == DFS) {
        int rc = daos_ftruncate(mfu_file, length);
        return rc;
    } else {
        MFU_ABORT(-1, "File type not known, type=%d",
                  mfu_file->type);
    }
}

/* unlink a file */
int mfu_file_unlink(const char* file, mfu_file_t* mfu_file)
{
    if (mfu_file->type == POSIX) {
        int rc = mfu_unlink(file);
        return rc;
    } else if (mfu_file->type == DFS) {
        int rc = daos_unlink(file, mfu_file);
        return rc;
    } else {
        MFU_ABORT(-1, "File type not known, type=%d",
                  mfu_file->type);
    } 
}

int daos_unlink(const char* file, mfu_file_t* mfu_file)
{
#ifdef DAOS_SUPPORT
    int rc = dfs_sys_remove(mfu_file->dfs_sys, file, false, NULL);
    return mfu_errno2rc(rc);
#else
    return mfu_errno2rc(ENOSYS);
#endif
}

/* delete a file */
int mfu_unlink(const char* file)
{
    int rc;
    int tries = MFU_IO_TRIES;
retry:
    errno = 0;
    rc = unlink(file);
    if (rc != 0) {
        if (errno == EINTR || errno == EIO) {
            tries--;
            if (tries > 0) {
                /* sleep a bit before consecutive tries */
                usleep(MFU_IO_USLEEP);
                goto retry;
            }
        }
    }
    return rc;
}

/* force flush of written data */
int mfu_fsync(const char* file, int fd)
{
    int rc;
    int tries = MFU_IO_TRIES;
retry:
    errno = 0;
    rc = fsync(fd);
    if (rc < 0) {
        if (errno == EINTR || errno == EIO) {
            tries--;
            if (tries > 0) {
                /* sleep a bit before consecutive tries */
                usleep(MFU_IO_USLEEP);
                goto retry;
            }
        }
    }
    return rc;
}

/*****************************
 * Directories
 ****************************/

/* get current working directory, abort if fail or buffer too small */
void mfu_getcwd(char* buf, size_t size)
{
    errno = 0;
    char* p = getcwd(buf, size);
    if (p == NULL) {
        MFU_ABORT(-1, "Failed to get current working directory errno=%d (%s)",
                    errno, strerror(errno)
                   );
    }
}

int daos_mkdir(const char* dir, mode_t mode, mfu_file_t* mfu_file)
{
#ifdef DAOS_SUPPORT
    int rc = dfs_sys_mkdir(mfu_file->dfs_sys, dir, mode, 0);
    return mfu_errno2rc(rc);
#else
    return mfu_errno2rc(ENOSYS);
#endif
}

int mfu_mkdir(const char* dir, mode_t mode)
{
    int rc;
    int tries = MFU_IO_TRIES;
retry:
    errno = 0;
    rc = mkdir(dir, mode);
    if (rc < 0) {
        if (errno == EINTR || errno == EIO) {
            tries--;
            if (tries > 0) {
                /* sleep a bit before consecutive tries */
                usleep(MFU_IO_USLEEP);
                goto retry;
            }
        }
    }
    return rc;
}

/* create directory, retry a few times on EINTR or EIO */
int mfu_file_mkdir(const char* dir, mode_t mode, mfu_file_t* mfu_file)
{
    if (mfu_file->type == POSIX) {
        int rc = mfu_mkdir(dir, mode);
        return rc;
    } else if (mfu_file->type == DFS) {
        int rc = daos_mkdir(dir, mode, mfu_file);
        return rc;
    } else {
        MFU_ABORT(-1, "File type not known: %s type=%d",
                  dir, mfu_file->type);
    }
}

int mfu_file_rmdir(const char* dir, mfu_file_t* mfu_file)
{
    if (mfu_file->type == POSIX) {
        int rc = mfu_rmdir(dir);
        return rc;
    } else if (mfu_file->type == DFS) {
        int rc = daos_rmdir(dir, mfu_file);
        return rc;
    } else {
        MFU_ABORT(-1, "File type not known: %s type=%d",
                  dir, mfu_file->type);
    }
}

/* remove directory, retry a few times on EINTR or EIO */
int mfu_rmdir(const char* dir)
{
    int rc;
    int tries = MFU_IO_TRIES;
retry:
    errno = 0;
    rc = rmdir(dir);
    if (rc < 0) {
        if (errno == EINTR || errno == EIO) {
            tries--;
            if (tries > 0) {
                /* sleep a bit before consecutive tries */
                usleep(MFU_IO_USLEEP);
                goto retry;
            }
        }
    }
    return rc;
}

int daos_rmdir(const char* dir, mfu_file_t* mfu_file)
{
#ifdef DAOS_SUPPORT
    int rc = dfs_sys_remove_type(mfu_file->dfs_sys, dir, false, S_IFDIR, NULL);
    return mfu_errno2rc(rc);
#else
    return mfu_errno2rc(ENOSYS);
#endif
}

/* open directory. The entry itself is not cached in mfu_file->dir_hash */
DIR* daos_opendir(const char* dir, mfu_file_t* mfu_file)
{
#ifdef DAOS_SUPPORT
    DIR* dirp = NULL;
    int rc = dfs_sys_opendir(mfu_file->dfs_sys, dir, 0, &dirp);
    if (rc != 0) {
        errno = rc;
        dirp = NULL;
    }
    return dirp;
#else
    errno = ENOSYS;
    return NULL;
#endif
}

/* open directory, retry a few times on EINTR or EIO */
DIR* mfu_opendir(const char* dir)
{
/*
    load_catalog_dir_if_needed();

    catalog_dir_t* mfu_dir = find_dir_in_catalog(dir);
    if (mfu_dir != NULL) {
#ifdef DEBUG
        printf("mfu_opendir: Found directory %s in catalog\n", dir);
#endif
        return (DIR*)mfu_dir;
    }
*/

    printf("opendir %s\n", dir);

    DIR* dirp;
    int tries = MFU_IO_TRIES;
retry:
    errno = 0;
    dirp = opendir(dir);
    if (dirp == NULL) {
        if (errno == EINTR || errno == EIO) {
            tries--;
            if (tries > 0) {
                /* sleep a bit before consecutive tries */
                usleep(MFU_IO_USLEEP);
                goto retry;
            }
        }
    }
    return dirp;
}

/* open directory, retry a few times on EINTR or EIO */
DIR* mfu_file_opendir(const char* dir, mfu_file_t* mfu_file)
{
    if (mfu_file->type == POSIX) {
        DIR* dirp = mfu_opendir(dir);
        return dirp;
    } else if (mfu_file->type == DFS) {
        DIR* dirp = daos_opendir(dir, mfu_file);
        return dirp;
    } else {
        MFU_ABORT(-1, "File type not known: %s type=%d",
                  dir, mfu_file->type);
    }
}

/* close dir. This is not cached in mfu_file->dir_hash */
int daos_closedir(DIR* dirp, mfu_file_t* mfu_file)
{
#ifdef DAOS_SUPPORT
    int rc = dfs_sys_closedir(dirp);
    return mfu_errno2rc(rc);
#else
    return mfu_errno2rc(ENOSYS);
#endif
}

/* close directory, retry a few times on EINTR or EIO */
int mfu_closedir(DIR* dirp)
{
/*    if ((catalog_dir_t*)dirp >= (catalog_dir_t*)catalog_dirs && (catalog_dir_t*)dirp < (catalog_dir_t*)catalog_dirs + catalog_dir_count) {
#ifdef DEBUG
        printf("mfu_closedir: Closing catalog directory\n");
#endif
        return 0;
    }
*/
    int rc;
    int tries = MFU_IO_TRIES;
retry:
    errno = 0;
    rc = closedir(dirp);
    if (rc < 0) {
        if (errno == EINTR || errno == EIO) {
            tries--;
            if (tries > 0) {
                /* sleep a bit before consecutive tries */
                usleep(MFU_IO_USLEEP);
                goto retry;
            }
        }
    }
    return rc;
}

int mfu_file_closedir(DIR* dirp, mfu_file_t* mfu_file)
{
    if (mfu_file->type == POSIX) {
        int rc = mfu_closedir(dirp);
        return rc;
    } else if (mfu_file->type == DFS) {
        int rc = daos_closedir(dirp, mfu_file);
        return rc;
    } else {
        MFU_ABORT(-1, "File type not known, type=%d",
                  mfu_file->type);
    }
}

struct dirent* daos_readdir(DIR* dirp, mfu_file_t* mfu_file)
{
#ifdef DAOS_SUPPORT
    struct dirent* dirent = NULL;
    int rc = dfs_sys_readdir(mfu_file->dfs_sys, dirp, &dirent);
    if (rc != 0) {
        errno = rc;
        dirent = NULL;
    }
    return dirent;
#else
    errno = ENOSYS;
    return NULL;
#endif
}

/* read directory entry, retry a few times on ENOENT, EIO, or EINTR */
struct dirent* mfu_readdir(DIR* dirp)
{
/*    if ((catalog_dir_t*)dirp >= (catalog_dir_t*)catalog_dirs && (catalog_dir_t*)dirp < (catalog_dir_t*)catalog_dirs + catalog_dir_count) {
        catalog_dir_t* mfu_dir = (catalog_dir_t*)dirp;
        if (mfu_dir->current_entry < mfu_dir->entry_count) {
#ifdef DEBUG
            printf("mfu_readdir: Reading entry %d in catalog directory %s\n", mfu_dir->current_entry, mfu_dir->dir_name);
#endif
            static struct dirent entry;
            memset(&entry, 0, sizeof(entry));
            strncpy(entry.d_name, mfu_dir->entries[mfu_dir->current_entry], sizeof(entry.d_name) - 1);
            mfu_dir->current_entry++;
            return &entry;
        }
        return NULL;
    }
*/
    /* read next directory entry, retry a few times */
    struct dirent* entry;
    int tries = MFU_IO_TRIES;
retry:
    errno = 0;
    entry = readdir(dirp);
    if (entry == NULL) {
        if (errno == EINTR || errno == EIO || errno == ENOENT) {
            tries--;
            if (tries > 0) {
                /* sleep a bit before consecutive tries */
                usleep(MFU_IO_USLEEP);
                goto retry;
            }
        }
    }
    return entry;
}

struct dirent* mfu_file_readdir(DIR* dirp, mfu_file_t* mfu_file)
{
    if (mfu_file->type == POSIX) {
        struct dirent* entry = mfu_readdir(dirp);
        return entry;
    } else if (mfu_file->type == DFS) {
        struct dirent* entry = daos_readdir(dirp, mfu_file);
        return entry;
    } else {
        MFU_ABORT(-1, "File type not known, type=%d",
                  mfu_file->type);
    }
}

/* list xattrs (link interrogation) */
ssize_t mfu_file_llistxattr(const char* path, char* list, size_t size, mfu_file_t* mfu_file)
{
    if (mfu_file->type == POSIX) {
        ssize_t rc = mfu_llistxattr(path, list, size);
        return rc;
    } else if (mfu_file->type == DFS) {
        ssize_t rc = daos_llistxattr(path, list, size, mfu_file);
        return rc;
    } else {
        MFU_ABORT(-1, "File type not known, type=%d",
                  mfu_file->type);
    }
}

ssize_t mfu_llistxattr(const char* path, char* list, size_t size)
{
    ssize_t rc = llistxattr(path, list, size);
    return rc;
}

/* DAOS wrapper for dfs_listxattr that adjusts return
 * codes and errno to be similar to POSIX llistxattr */
ssize_t daos_llistxattr(const char* path, char* list, size_t size, mfu_file_t* mfu_file)
{
#ifdef DAOS_SUPPORT
    daos_size_t got_size = size;
    int rc = dfs_sys_listxattr(mfu_file->dfs_sys, path, list, &got_size, O_NOFOLLOW);
    if (rc != 0) {
        errno = rc;
    }
    return (ssize_t) got_size;
#else
    return (ssize_t) mfu_errno2rc(ENOSYS);
#endif
}

/* list xattrs (link dereference) */
ssize_t mfu_file_listxattr(const char* path, char* list, size_t size, mfu_file_t* mfu_file)
{
    if (mfu_file->type == POSIX) {
        ssize_t rc = mfu_listxattr(path, list, size);
        return rc;
    } else if (mfu_file->type == DFS) {
        ssize_t rc = daos_listxattr(path, list, size, mfu_file);
        return rc;
    } else {
        MFU_ABORT(-1, "File type not known, type=%d",
                  mfu_file->type);
    }
}

ssize_t mfu_listxattr(const char* path, char* list, size_t size)
{
    ssize_t rc = listxattr(path, list, size);
    return rc;
}

/* DAOS wrapper for dfs_listxattr that adjusts return
 * codes and errno to be similar to POSIX listxattr */
ssize_t daos_listxattr(const char* path, char* list, size_t size, mfu_file_t* mfu_file)
{
#ifdef DAOS_SUPPORT
    daos_size_t got_size = size;
    int rc = dfs_sys_listxattr(mfu_file->dfs_sys, path, list, &got_size, 0);
    if (rc != 0) {
        errno = rc;
    }
    return (ssize_t) got_size;
#else
    return (ssize_t) mfu_errno2rc(ENOSYS);
#endif
}

/* get xattrs (link interrogation) */
ssize_t mfu_file_lgetxattr(const char* path, const char* name, void* value, size_t size, mfu_file_t* mfu_file)
{
   if (mfu_file->type == POSIX) {
        ssize_t rc = mfu_lgetxattr(path, name, value, size);
        return rc;
    } else if (mfu_file->type == DFS) {
        ssize_t rc = daos_lgetxattr(path, name, value, size, mfu_file);
        return rc;
    } else {
        MFU_ABORT(-1, "File type not known, type=%d",
                  mfu_file->type);
    } 
}

ssize_t mfu_lgetxattr(const char* path, const char* name, void* value, size_t size)
{
    ssize_t rc = lgetxattr(path, name, value, size);
    return rc;
}

ssize_t daos_lgetxattr(const char* path, const char* name, void* value, size_t size, mfu_file_t* mfu_file)
{
#ifdef DAOS_SUPPORT
    daos_size_t got_size = size;
    int rc = dfs_sys_getxattr(mfu_file->dfs_sys, path, name, value, &got_size, O_NOFOLLOW);
    if (rc != 0) {
        errno = rc;
    }
    return (ssize_t) got_size;
#else
    return (ssize_t) mfu_errno2rc(ENOSYS);
#endif
}

/* get xattrs (link dereference) */
ssize_t mfu_file_getxattr(const char* path, const char* name, void* value, size_t size, mfu_file_t* mfu_file)
{
   if (mfu_file->type == POSIX) {
        ssize_t rc = mfu_getxattr(path, name, value, size);
        return rc;
    } else if (mfu_file->type == DFS) {
        ssize_t rc = daos_getxattr(path, name, value, size, mfu_file);
        return rc;
    } else {
        MFU_ABORT(-1, "File type not known, type=%d",
                  mfu_file->type);
    } 
}

ssize_t mfu_getxattr(const char* path, const char* name, void* value, size_t size)
{
    ssize_t rc = getxattr(path, name, value, size);
    return rc;
}

ssize_t daos_getxattr(const char* path, const char* name, void* value, size_t size, mfu_file_t* mfu_file)
{
#ifdef DAOS_SUPPORT
    daos_size_t got_size = size;
    int rc = dfs_sys_getxattr(mfu_file->dfs_sys, path, name, value, &got_size, 0);
    if (rc != 0) {
        errno = rc;
    }
    return (ssize_t) got_size;
#else
    return (ssize_t) mfu_errno2rc(ENOSYS);
#endif
}

/* set xattrs (link interrogation) */
int mfu_file_lsetxattr(const char* path, const char* name, const void* value, size_t size, int flags,
                       mfu_file_t* mfu_file)
{
    if (mfu_file->type == POSIX) {
        int rc = mfu_lsetxattr(path, name, value, size, flags);
        return rc;
    } else if (mfu_file->type == DFS) {
        int rc = daos_lsetxattr(path, name, value, size, flags, mfu_file);
        return rc;
    } else {
        MFU_ABORT(-1, "File type not known, type=%d",
                  mfu_file->type);
    }
}

int mfu_lsetxattr(const char* path, const char* name, const void* value, size_t size, int flags)
{
    int rc = lsetxattr(path, name, value, size, flags);
    return rc;
}

int daos_lsetxattr(const char* path, const char* name, const void* value, size_t size, int flags,
                   mfu_file_t* mfu_file)
{
#ifdef DAOS_SUPPORT
    int rc = dfs_sys_setxattr(mfu_file->dfs_sys, path, name, value, size, flags, O_NOFOLLOW);
    return mfu_errno2rc(rc);
#else
    return mfu_errno2rc(ENOSYS);
#endif
}
