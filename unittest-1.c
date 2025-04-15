/*
 * file:        testing.c
 * description: libcheck test skeleton for file system project
 */

#define _FILE_OFFSET_BITS 64
#define FUSE_USE_VERSION 26
#define MAX_PATH_LEN 10
#define MAX_NAME_LEN 28

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <check.h>
#include <zlib.h>
#include <fuse.h>
#include <stdlib.h>
#include <errno.h>

char **argv;
int entry;
/* this is an example of a callback function for readdir
 */
int empty_filler(void *ptr, const char *name, const struct stat *stbuf,
                 off_t off)
{
    /* FUSE passes you the entry name and a pointer to a 'struct stat' 
     * with the attributes. Ignore the 'ptr' and 'off' arguments 
     * 
     */
    if(strcmp(".", name) != 0 && strcmp("..", name) != 0) {
        strcpy(*(argv+(entry++)), name);
    }
    return 0;
}

/* note that your tests will call:
 *  fs_ops.getattr(path, struct stat *sb)
 *  fs_ops.readdir(path, NULL, filler_function, 0, NULL)
 *  fs_ops.read(path, buf, len, offset, NULL);
 *  fs_ops.statfs(path, struct statvfs *sv);
 */

extern struct fuse_operations fs_ops;
extern void block_init(char *file);
int compFunc(const void *a, const void *b);

/* Get Attr Tests */

START_TEST(root) {
    struct stat *sb = malloc(sizeof(struct stat));
    fs_ops.getattr("/", sb);
    ck_assert_int_eq(0, sb->st_uid);
    ck_assert_int_eq(0, sb->st_gid);
    ck_assert_int_eq(040777, sb->st_mode);
    ck_assert_int_eq(4096, sb->st_size);
    ck_assert_int_eq(1565283152, sb->st_ctime);
    ck_assert_int_eq(1565283167, sb->st_mtime);
    free(sb);
} END_TEST

START_TEST(file1k) {
    struct stat *sb = malloc(sizeof(struct stat));
    fs_ops.getattr("/file.1k", sb);
    ck_assert_int_eq(500, sb->st_uid);
    ck_assert_int_eq(500, sb->st_gid);
    ck_assert_int_eq(0100666, sb->st_mode);
    ck_assert_int_eq(1000, sb->st_size);
    ck_assert_int_eq(1565283152, sb->st_ctime);
    ck_assert_int_eq(1565283152, sb->st_mtime);
    free(sb);
} END_TEST

START_TEST(file10) {
    struct stat *sb = malloc(sizeof(struct stat));
    fs_ops.getattr("/file.10", sb);
    ck_assert_int_eq(500, sb->st_uid);
    ck_assert_int_eq(500, sb->st_gid);
    ck_assert_int_eq(0100666, sb->st_mode);
    ck_assert_int_eq(10, sb->st_size);
    ck_assert_int_eq(1565283152, sb->st_ctime);
    ck_assert_int_eq(1565283167, sb->st_mtime);
    free(sb);
} END_TEST

START_TEST(dir_long) {
    struct stat *sb = malloc(sizeof(struct stat));
    fs_ops.getattr("/dir-with-long-name", sb);
    ck_assert_int_eq(0, sb->st_uid);
    ck_assert_int_eq(0, sb->st_gid);
    ck_assert_int_eq(040777, sb->st_mode);
    ck_assert_int_eq(4096, sb->st_size);
    ck_assert_int_eq(1565283152, sb->st_ctime);
    ck_assert_int_eq(1565283167, sb->st_mtime);
    free(sb);
} END_TEST

START_TEST(dir_long_file12k) {
    struct stat *sb = malloc(sizeof(struct stat));
    fs_ops.getattr("/dir-with-long-name/file.12k+", sb);
    ck_assert_int_eq(0, sb->st_uid);
    ck_assert_int_eq(500, sb->st_gid);
    ck_assert_int_eq(0100666, sb->st_mode);
    ck_assert_int_eq(12289, sb->st_size);
    ck_assert_int_eq(1565283152, sb->st_ctime);
    ck_assert_int_eq(1565283167, sb->st_mtime);
    free(sb);
} END_TEST

START_TEST(dir_2) {
    struct stat *sb = malloc(sizeof(struct stat));
    fs_ops.getattr("/dir2", sb);
    ck_assert_int_eq(500, sb->st_uid);
    ck_assert_int_eq(500, sb->st_gid);
    ck_assert_int_eq(040777, sb->st_mode);
    ck_assert_int_eq(8192, sb->st_size);
    ck_assert_int_eq(1565283152, sb->st_ctime);
    ck_assert_int_eq(1565283167, sb->st_mtime);
    free(sb);
} END_TEST

START_TEST(dir_2_27byteName) {
    struct stat *sb = malloc(sizeof(struct stat));
    fs_ops.getattr("/dir2/twenty-seven-byte-file-name", sb);
    ck_assert_int_eq(500, sb->st_uid);
    ck_assert_int_eq(500, sb->st_gid);
    ck_assert_int_eq(0100666, sb->st_mode);
    ck_assert_int_eq(1000, sb->st_size);
    ck_assert_int_eq(1565283152, sb->st_ctime);
    ck_assert_int_eq(1565283167, sb->st_mtime);
    free(sb);
} END_TEST

START_TEST(dir_2_file4k) {
    struct stat *sb = malloc(sizeof(struct stat));
    fs_ops.getattr("/dir2/file.4k+", sb);
    ck_assert_int_eq(500, sb->st_uid);
    ck_assert_int_eq(500, sb->st_gid);
    ck_assert_int_eq(0100777, sb->st_mode);
    ck_assert_int_eq(4098, sb->st_size);
    ck_assert_int_eq(1565283152, sb->st_ctime);
    ck_assert_int_eq(1565283167, sb->st_mtime);
    free(sb);
} END_TEST

START_TEST(dir_3) {
    struct stat *sb = malloc(sizeof(struct stat));
    fs_ops.getattr("/dir3", sb);
    ck_assert_int_eq(0, sb->st_uid);
    ck_assert_int_eq(500, sb->st_gid);
    ck_assert_int_eq(040777, sb->st_mode);
    ck_assert_int_eq(4096, sb->st_size);
    ck_assert_int_eq(1565283152, sb->st_ctime);
    ck_assert_int_eq(1565283167, sb->st_mtime);
    free(sb);
} END_TEST

START_TEST(dir_3_subdir) {
    struct stat *sb = malloc(sizeof(struct stat));
    fs_ops.getattr("/dir3/subdir", sb);
    ck_assert_int_eq(0, sb->st_uid);
    ck_assert_int_eq(500, sb->st_gid);
    ck_assert_int_eq(040777, sb->st_mode);
    ck_assert_int_eq(4096, sb->st_size);
    ck_assert_int_eq(1565283152, sb->st_ctime);
    ck_assert_int_eq(1565283167, sb->st_mtime);
    free(sb);
} END_TEST

START_TEST(dir_3_subdir_f4k) {
    struct stat *sb = malloc(sizeof(struct stat));
    fs_ops.getattr("/dir3/subdir/file.4k-", sb);
    ck_assert_int_eq(500, sb->st_uid);
    ck_assert_int_eq(500, sb->st_gid);
    ck_assert_int_eq(0100666, sb->st_mode);
    ck_assert_int_eq(4095, sb->st_size);
    ck_assert_int_eq(1565283152, sb->st_ctime);
    ck_assert_int_eq(1565283167, sb->st_mtime);
    free(sb);
} END_TEST

START_TEST(dir_3_subdir_f8k) {
    struct stat *sb = malloc(sizeof(struct stat));
    fs_ops.getattr("/dir3/subdir/file.8k-", sb);
    ck_assert_int_eq(500, sb->st_uid);
    ck_assert_int_eq(500, sb->st_gid);
    ck_assert_int_eq(0100666, sb->st_mode);
    ck_assert_int_eq(8190, sb->st_size);
    ck_assert_int_eq(1565283152, sb->st_ctime);
    ck_assert_int_eq(1565283167, sb->st_mtime);
    free(sb);
} END_TEST

START_TEST(dir_3_subdir_f12k) {
    struct stat *sb = malloc(sizeof(struct stat));
    fs_ops.getattr("/dir3/subdir/file.12k", sb);
    ck_assert_int_eq(500, sb->st_uid);
    ck_assert_int_eq(500, sb->st_gid);
    ck_assert_int_eq(0100666, sb->st_mode);
    ck_assert_int_eq(12288, sb->st_size);
    ck_assert_int_eq(1565283152, sb->st_ctime);
    ck_assert_int_eq(1565283167, sb->st_mtime);
    free(sb);
} END_TEST

START_TEST(dir_3_f12k) {
    struct stat *sb = malloc(sizeof(struct stat));
    fs_ops.getattr("/dir3/file.12k-", sb);
    ck_assert_int_eq(0, sb->st_uid);
    ck_assert_int_eq(500, sb->st_gid);
    ck_assert_int_eq(0100777, sb->st_mode);
    ck_assert_int_eq(12287, sb->st_size);
    ck_assert_int_eq(1565283152, sb->st_ctime);
    ck_assert_int_eq(1565283167, sb->st_mtime);
    free(sb);
} END_TEST

START_TEST(dir_f8k) {
    struct stat *sb = malloc(sizeof(struct stat));
    fs_ops.getattr("/file.8k+", sb);
    ck_assert_int_eq(500, sb->st_uid);
    ck_assert_int_eq(500, sb->st_gid);
    ck_assert_int_eq(0100666, sb->st_mode);
    ck_assert_int_eq(8195, sb->st_size);
    ck_assert_int_eq(1565283152, sb->st_ctime);
    ck_assert_int_eq(1565283167, sb->st_mtime);
    free(sb);
} END_TEST

START_TEST(no_entry) {
    struct stat *sb = malloc(sizeof(struct stat));
    int err = fs_ops.getattr("/not-a-file", sb);
    ck_assert_int_eq(-ENOENT, err);
    free(sb);
} END_TEST

START_TEST(no_dir) {
    struct stat *sb = malloc(sizeof(struct stat));
    int err = fs_ops.getattr("/file.1k/file.0", sb);
    ck_assert_int_eq(-ENOTDIR, err);
    free(sb);
} END_TEST

START_TEST(no_dir_mid) {
    struct stat *sb = malloc(sizeof(struct stat));
    int err = fs_ops.getattr("/files/file.12k-", sb);
    ck_assert_int_eq(-ENOENT, err);
    free(sb);
} END_TEST

START_TEST(no_file_sub) {
    struct stat *sb = malloc(sizeof(struct stat));
    int err = fs_ops.getattr("/dir2/rouge_file", sb);
    ck_assert_int_eq(-ENOENT, err);
    free(sb);
} END_TEST

/* Read Dir Tests */

int compFunc(const void *a, const void *b) {
    return strcmp(*(char**)a, *(char**)b);
}

START_TEST(read_root) {
    //"dir2", "dir3", "dir-with-long-name", "file.10","file.1k", "file.8k+"
    argv = malloc(MAX_PATH_LEN * sizeof(char *));
    char *sorted_names[] = {"dir-with-long-name", "dir2", "dir3", "file.10", "file.1k", "file.8k+"};
    for(int i = 0; i < MAX_PATH_LEN; i++) {
        argv[i] = malloc(MAX_NAME_LEN);
    }
    entry = 0;
    int err = fs_ops.readdir("/", NULL, empty_filler, 0, NULL);
    ck_assert_int_eq(0, err);
    qsort(argv, entry, sizeof(char *), compFunc);
    for(int i = 0; i < entry; i++) {
        ck_assert_str_eq(sorted_names[i], argv[i]);
    }
    for(int i = 0; i < MAX_PATH_LEN; i++) free(argv[i]);
    free(argv);
} END_TEST

START_TEST(read_dir2) {
    //"twenty-seven-byte-file-name", "file.4k+"
    argv = malloc(MAX_PATH_LEN * sizeof(char *));
    char *sorted_names[] = {"file.4k+", "twenty-seven-byte-file-name"};
    for(int i = 0; i < MAX_PATH_LEN; i++) {
        argv[i] = malloc(MAX_NAME_LEN);
    }
    entry = 0;
    int err = fs_ops.readdir("/dir2", NULL, empty_filler, 0, NULL);
    ck_assert_int_eq(0, err);
    qsort(argv, entry, sizeof(char *), compFunc);
    for(int i = 0; i < entry; i++) {
        ck_assert_str_eq(sorted_names[i], argv[i]);
    }
    for(int i = 0; i < MAX_PATH_LEN; i++) free(argv[i]);
    free(argv);
} END_TEST

START_TEST(read_dir3) {
    //"subdir", "file.12k-"
    argv = malloc(MAX_PATH_LEN * sizeof(char *));
    char *sorted_names[] = {"file.12k-", "subdir"};
    for(int i = 0; i < MAX_PATH_LEN; i++) {
        argv[i] = malloc(MAX_NAME_LEN);
    }
    entry = 0;
    int err = fs_ops.readdir("/dir3", NULL, empty_filler, 0, NULL);
    ck_assert_int_eq(0, err);
    qsort(argv, entry, sizeof(char *), compFunc);
    for(int i = 0; i < entry; i++) {
        ck_assert_str_eq(sorted_names[i], argv[i]);
    }
    for(int i = 0; i < MAX_PATH_LEN; i++) free(argv[i]);
    free(argv);
} END_TEST

START_TEST(read_dir3_subdir) {
    //"file.4k-", "file.8k-", "file.12k"
    argv = malloc(MAX_PATH_LEN * sizeof(char *));
    char *sorted_names[] = {"file.12k", "file.4k-", "file.8k-"};
    for(int i = 0; i < MAX_PATH_LEN; i++) {
        argv[i] = malloc(MAX_NAME_LEN);
    }
    entry = 0;
    int err = fs_ops.readdir("/dir3/subdir", NULL, empty_filler, 0, NULL);
    ck_assert_int_eq(0, err);
    qsort(argv, entry, sizeof(char *), compFunc);
    for(int i = 0; i < entry; i++) {
        ck_assert_str_eq(sorted_names[i], argv[i]);
    }
    for(int i = 0; i < MAX_PATH_LEN; i++) free(argv[i]);
    free(argv);
} END_TEST

START_TEST(read_dir_long) {
    //"file.12k+"
    argv = malloc(MAX_PATH_LEN * sizeof(char *));
    char *sorted_names[] = {"file.12k+"};
    for(int i = 0; i < MAX_PATH_LEN; i++) {
        argv[i] = malloc(MAX_NAME_LEN);
    }
    entry = 0;
    int err = fs_ops.readdir("/dir-with-long-name", NULL, empty_filler, 0, NULL);
    ck_assert_int_eq(0, err);
    qsort(argv, entry, sizeof(char *), compFunc);
    for(int i = 0; i < entry; i++) {
        ck_assert_str_eq(sorted_names[i], argv[i]);
    }
    for(int i = 0; i < MAX_PATH_LEN; i++) free(argv[i]);
    free(argv);
} END_TEST

START_TEST(read_dir_give_file) {
    int err = fs_ops.readdir("/file.10", NULL, empty_filler, 0, NULL);
    ck_assert_int_eq(-ENOTDIR, err);
} END_TEST

START_TEST(read_dir_no_dir) {
    int err = fs_ops.readdir("/dir2/noexistent-dir", NULL, empty_filler, 0, NULL);
    ck_assert_int_eq(-ENOENT, err);
} END_TEST

/* Read File Tests */

START_TEST(read_file_file10) {
    char *buf = malloc(12289);
    int bRead = fs_ops.read("/file.10", buf, 10, 0, NULL);
    ck_assert_int_eq(10, bRead);
    uLong crc = crc32(0L, Z_NULL, 0);
    ck_assert_int_eq(3766980606, crc32(crc, (const Bytef *)buf, bRead));
} END_TEST

START_TEST(read_file_file1k) {
    char *buf = malloc(12289);
    int bRead = fs_ops.read("/file.1k", buf, 1000, 0, NULL);
    ck_assert_int_eq(1000, bRead);
    uLong crc = crc32(0L, Z_NULL, 0);
    ck_assert_int_eq(1726121896, crc32(crc, (const Bytef *)buf, bRead));
} END_TEST

START_TEST(read_file_file8k) {
    char *buf = malloc(12289);
    int bRead = fs_ops.read("/file.8k+", buf, 8195, 0, NULL);
    ck_assert_int_eq(8195, bRead);
    uLong crc = crc32(0L, Z_NULL, 0);
    ck_assert_int_eq(1217760297, crc32(crc, (const Bytef *)buf, bRead));
} END_TEST

START_TEST(read_dir_with_long_name_file12k) {
    char *buf = malloc(12289);
    int bRead = fs_ops.read("/dir-with-long-name/file.12k+", buf, 12289, 0, NULL);
    ck_assert_int_eq(12289, bRead);
    uLong crc = crc32(0L, Z_NULL, 0);
    ck_assert_int_eq(2781093465, crc32(crc, (const Bytef *)buf, bRead));
} END_TEST

START_TEST(dir2_twenty_seven_byte_file_name) {
    char *buf = malloc(12289);
    int bRead = fs_ops.read("/dir2/twenty-seven-byte-file-name", buf, 1000, 0, NULL);
    ck_assert_int_eq(1000, bRead);
    uLong crc = crc32(0L, Z_NULL, 0);
    ck_assert_int_eq(2902524398, crc32(crc, (const Bytef *)buf, bRead));
} END_TEST

START_TEST(dir2_file4k) {
    char *buf = malloc(12289);
    int bRead = fs_ops.read("/dir2/file.4k+", buf, 4098, 0, NULL);
    ck_assert_int_eq(4098, bRead);
    uLong crc = crc32(0L, Z_NULL, 0);
    ck_assert_int_eq(1626046637, crc32(crc, (const Bytef *)buf, bRead));
} END_TEST

START_TEST(dir3_subdir_file4k) {
    char *buf = malloc(12289);
    int bRead = fs_ops.read("/dir3/subdir/file.4k-", buf, 12289, 0, NULL);
    ck_assert_int_eq(4095, bRead);
    uLong crc = crc32(0L, Z_NULL, 0);
    ck_assert_int_eq(2991486384, crc32(crc, (const Bytef *)buf, bRead));
} END_TEST

START_TEST(dir3_subdir_file8k) {
    char *buf = malloc(12289);
    int bRead = fs_ops.read("/dir3/subdir/file.8k-", buf, 12289, 0, NULL);
    ck_assert_int_eq(8190, bRead);
    uLong crc = crc32(0L, Z_NULL, 0);
    ck_assert_int_eq(724101859, crc32(crc, (const Bytef *)buf, bRead));
} END_TEST

START_TEST(dir3_subdir_file12k) {
    char *buf = malloc(12289);
    int bRead = fs_ops.read("/dir3/subdir/file.12k", buf, 12288, 0, NULL);
    ck_assert_int_eq(12288, bRead);
    uLong crc = crc32(0L, Z_NULL, 0);
    ck_assert_int_eq(1483119748, crc32(crc, (const Bytef *)buf, bRead));
} END_TEST

START_TEST(dir3_file12k) {
    char *buf = malloc(12289);
    int bRead = fs_ops.read("/dir3/file.12k-", buf, 12287, 0, NULL);
    ck_assert_int_eq(12287, bRead);
    uLong crc = crc32(0L, Z_NULL, 0);
    ck_assert_int_eq(1203178000, crc32(crc, (const Bytef *)buf, bRead));
} END_TEST

/* Stats */

START_TEST(fs_statvfs) {
    struct statvfs *st = malloc(sizeof(struct statvfs));
    fs_ops.statfs(NULL, st);
    ck_assert_int_eq(st->f_bsize, 4096);
    ck_assert_int_eq(st->f_bfree, 355);
    ck_assert_int_eq(st->f_namemax, 27);
    ck_assert_int_eq(st->f_blocks, 400);
    free(st);
} END_TEST

START_TEST(fs_statvfs2) {
    struct statvfs *st = malloc(sizeof(struct statvfs));
    fs_ops.statfs("/dir3/file.12k-", st);
    ck_assert_int_eq(st->f_bsize, 4096);
    ck_assert_int_eq(st->f_bfree, 355);
    ck_assert_int_eq(st->f_namemax, 27);
    ck_assert_int_eq(st->f_blocks, 400);
    free(st);
} END_TEST

/* Read in chunks */

START_TEST(dir3_file12k_small_10) {
    char *buf = malloc(12289);
    int rSize = 10;
    int bRead = 0;
    for(int i = 0; i < 12289; i+=rSize) {
        bRead += fs_ops.read("/dir3/file.12k-", buf+i, rSize, i, NULL);
    }
    ck_assert_int_eq(12287, bRead);
    uLong crc = crc32(0L, Z_NULL, 0);
    ck_assert_int_eq(1203178000, crc32(crc, (const Bytef *)buf, bRead));
} END_TEST

START_TEST(dir3_file12k_small_17) {
    char *buf = malloc(12289);
    int rSize = 17;
    int bRead = 0;
    for(int i = 0; i < 12289; i+=rSize) {
        bRead += fs_ops.read("/file.1k", buf+i, rSize, i, NULL);
    }
    ck_assert_int_eq(1000, bRead);
    uLong crc = crc32(0L, Z_NULL, 0);
    ck_assert_int_eq(1726121896, crc32(crc, (const Bytef *)buf, bRead));
} END_TEST

START_TEST(dir3_file12k_small_100) {
    char *buf = malloc(12289);
    int rSize = 100;
    int bRead = 0;
    for(int i = 0; i < 12289; i+=rSize) {
        bRead += fs_ops.read("/dir-with-long-name/file.12k+", buf+i, rSize, i, NULL);
    }
    ck_assert_int_eq(12289, bRead);
    uLong crc = crc32(0L, Z_NULL, 0);
    ck_assert_int_eq(2781093465, crc32(crc, (const Bytef *)buf, bRead));
} END_TEST

START_TEST(dir3_file12k_small_1000) {
    char *buf = malloc(12289);
    int rSize = 1000;
    int bRead = 0;
    for(int i = 0; i < 12289; i+=rSize) {
        bRead += fs_ops.read("/dir2/twenty-seven-byte-file-name", buf+i, rSize, i, NULL);
    }
    ck_assert_int_eq(1000, bRead);
    uLong crc = crc32(0L, Z_NULL, 0);
    ck_assert_int_eq(2902524398, crc32(crc, (const Bytef *)buf, bRead));
} END_TEST

START_TEST(dir3_file12k_small_1024) {
    char *buf = malloc(12289);
    int rSize = 1024;
    int bRead = 0;
    for(int i = 0; i < 12289; i+=rSize) {
        bRead += fs_ops.read("/dir2/file.4k+", buf+i, rSize, i, NULL);
    }
    ck_assert_int_eq(4098, bRead);
    uLong crc = crc32(0L, Z_NULL, 0);
    ck_assert_int_eq(1626046637, crc32(crc, (const Bytef *)buf, bRead));
} END_TEST

START_TEST(dir3_file12k_small_1970) {
    char *buf = malloc(12289);
    int rSize = 1970;
    int bRead = 0;
    for(int i = 0; i < 12289; i+=rSize) {
        bRead += fs_ops.read("/dir3/subdir/file.12k", buf+i, rSize, i, NULL);
    }
    ck_assert_int_eq(12288, bRead);
    uLong crc = crc32(0L, Z_NULL, 0);
    ck_assert_int_eq(1483119748, crc32(crc, (const Bytef *)buf, bRead));
} END_TEST

START_TEST(dir3_file12k_small_3000) {
    char *buf = malloc(12289);
    int rSize = 3000;
    int bRead = 0;
    for(int i = 0; i < 12289; i+=rSize) {
        bRead += fs_ops.read("/dir3/file.12k-", buf+i, rSize, i, NULL);
    }
    ck_assert_int_eq(12287, bRead);
    uLong crc = crc32(0L, Z_NULL, 0);
    ck_assert_int_eq(1203178000, crc32(crc, (const Bytef *)buf, bRead));
} END_TEST

START_TEST(chmod_file) {
    struct stat *st = malloc(sizeof(struct stat));
    fs_ops.chmod("/dir3/file.12k-", 0100000);
    fs_ops.getattr("/dir3/file.12k-", st);
    ck_assert_int_eq(0100000, st->st_mode);
    ck_assert_int_eq(1, S_ISREG(st->st_mode));
} END_TEST

/* Renaming */

// rename file

START_TEST(rename_file_1) {
    fs_ops.rename("/dir3/file.12k-", "/dir3/file.12k");
    char *buf = malloc(12289);
    int bRead = fs_ops.read("/dir3/file.12k", buf, 12289, 0, NULL);
    ck_assert_int_eq(12287, bRead);
    uLong crc = crc32(0L, Z_NULL, 0);
    ck_assert_int_eq(1203178000, crc32(crc, (const Bytef *)buf, bRead));
} END_TEST

START_TEST(rename_file_2) {
    fs_ops.rename("/file.1k", "/file.thousand");
    char *buf = malloc(12289);
    int bRead = fs_ops.read("/file.thousand", buf, 12289, 0, NULL);
    ck_assert_int_eq(1000, bRead);
    uLong crc = crc32(0L, Z_NULL, 0);
    ck_assert_int_eq(1726121896, crc32(crc, (const Bytef *)buf, bRead));
} END_TEST

START_TEST(rename_file_3) {
    fs_ops.rename("/dir3/subdir/file.8k-", "/dir3/subdir/file.eightk-");
    char *buf = malloc(12289);
    int bRead = fs_ops.read("/dir3/subdir/file.eightk-", buf, 12289, 0, NULL);
    ck_assert_int_eq(8190, bRead);
    uLong crc = crc32(0L, Z_NULL, 0);
    ck_assert_int_eq(724101859, crc32(crc, (const Bytef *)buf, bRead));
} END_TEST

//rename dir

START_TEST(rename_dir_1) {
    fs_ops.rename("/dir3", "/dirThree");
    argv = malloc(MAX_PATH_LEN * sizeof(char *));
    char *sorted_names[] = {"file.12k-", "subdir"};
    for(int i = 0; i < MAX_PATH_LEN; i++) {
        argv[i] = malloc(MAX_NAME_LEN);
    }
    entry = 0;
    int err = fs_ops.readdir("/dirThree", NULL, empty_filler, 0, NULL);
    ck_assert_int_eq(0, err);
    qsort(argv, entry, sizeof(char *), compFunc);
    for(int i = 0; i < entry; i++) {
        ck_assert_str_eq(sorted_names[i], argv[i]);
    }
    for(int i = 0; i < MAX_PATH_LEN; i++) free(argv[i]);
    free(argv);
} END_TEST

START_TEST(rename_dir_2) {
    fs_ops.rename("/dir3/subdir", "/dir3/subdirectory");
    argv = malloc(MAX_PATH_LEN * sizeof(char *));
    char *sorted_names[] = {"file.4k-", "file.8k-", "file.12k"};
    for(int i = 0; i < MAX_PATH_LEN; i++) {
        argv[i] = malloc(MAX_NAME_LEN);
    }
    entry = 0;
    int err = fs_ops.readdir("/dir3/subdirectory", NULL, empty_filler, 0, NULL);
    ck_assert_int_eq(0, err);
    qsort(argv, entry, sizeof(char *), compFunc);
    for(int i = 0; i < entry; i++) {
        ck_assert_str_eq(sorted_names[i], argv[i]);
    }
    for(int i = 0; i < MAX_PATH_LEN; i++) free(argv[i]);
    free(argv);
} END_TEST

START_TEST(rename_dir_3) {
    fs_ops.rename("/dir-with-long-name", "/dir-short");
    argv = malloc(MAX_PATH_LEN * sizeof(char *));
    char *sorted_names[] = {"file.12k+"};
    for(int i = 0; i < MAX_PATH_LEN; i++) {
        argv[i] = malloc(MAX_NAME_LEN);
    }
    entry = 0;
    int err = fs_ops.readdir("/dir-short", NULL, empty_filler, 0, NULL);
    ck_assert_int_eq(0, err);
    qsort(argv, entry, sizeof(char *), compFunc);
    for(int i = 0; i < entry; i++) {
        ck_assert_str_eq(sorted_names[i], argv[i]);
    }
    for(int i = 0; i < MAX_PATH_LEN; i++) free(argv[i]);
    free(argv);
} END_TEST

// TODO: write more tests for renaming files and also DIRS!!!! - jeff

void get_attr_tests(TCase *tc){
    tcase_add_test(tc, root);
    tcase_add_test(tc, file1k);
    tcase_add_test(tc, file10);
    tcase_add_test(tc, dir_long);
    tcase_add_test(tc, dir_long_file12k);
    tcase_add_test(tc, dir_2);
    tcase_add_test(tc, dir_2_27byteName);
    tcase_add_test(tc, dir_2_file4k);
    tcase_add_test(tc, dir_3);
    tcase_add_test(tc, dir_3_subdir);
    tcase_add_test(tc, dir_3_subdir_f4k);
    tcase_add_test(tc, dir_3_subdir_f8k);
    tcase_add_test(tc, dir_3_subdir_f12k);
    tcase_add_test(tc, dir_3_f12k);
    tcase_add_test(tc, dir_f8k);
    tcase_add_test(tc, no_entry);
    tcase_add_test(tc, no_dir);
    tcase_add_test(tc, no_dir_mid);
    tcase_add_test(tc, no_file_sub);
}

void read_dir_tests(TCase *tc){
    tcase_add_test(tc, read_root);
    tcase_add_test(tc, read_dir2);
    tcase_add_test(tc, read_dir3);
    tcase_add_test(tc, read_dir3_subdir);
    tcase_add_test(tc, read_dir_long);
    tcase_add_test(tc, read_dir_give_file);
    tcase_add_test(tc, read_dir_no_dir);
}

void read_tests(TCase *tc) {
    //fs_read - single big read
    tcase_add_test(tc, read_file_file10);
    tcase_add_test(tc, read_file_file1k);
    tcase_add_test(tc, read_file_file8k);
    tcase_add_test(tc, read_dir_with_long_name_file12k);
    tcase_add_test(tc, dir2_twenty_seven_byte_file_name);
    tcase_add_test(tc, dir2_file4k);
    tcase_add_test(tc, dir3_subdir_file4k);
    tcase_add_test(tc, dir3_subdir_file8k);
    tcase_add_test(tc, dir3_subdir_file12k);
    tcase_add_test(tc, dir3_file12k);

    //fs_read - multiple small reads
    tcase_add_test(tc, dir3_file12k_small_10);
    tcase_add_test(tc, dir3_file12k_small_17);
    tcase_add_test(tc, dir3_file12k_small_100);
    tcase_add_test(tc, dir3_file12k_small_1000);
    tcase_add_test(tc, dir3_file12k_small_1024);
    tcase_add_test(tc, dir3_file12k_small_1970);
    tcase_add_test(tc, dir3_file12k_small_3000);


    // fs_statvfs
    tcase_add_test(tc, fs_statvfs);
    tcase_add_test(tc, fs_statvfs2);
}

void modify_tests(TCase *tc) {
    //chmod
    tcase_add_test(tc, chmod_file);

    //rename
    tcase_add_test(tc, rename_file_1);
    tcase_add_test(tc, rename_file_2);
    tcase_add_test(tc, rename_file_3);
    tcase_add_test(tc, rename_dir_1);
    tcase_add_test(tc, rename_dir_2);
    tcase_add_test(tc, rename_dir_3);
}

int main(int argc, char **argv)
{
    system("python gen-disk.py -q disk1.in test.img");
    block_init("test.img");
    fs_ops.init(NULL);
    
    Suite *s = suite_create("fs5600:read_mostly");
    TCase *getattr = tcase_create("get_attr");
    TCase *readdir = tcase_create("read_dir");
    TCase *read_t = tcase_create("read");
    TCase *modify_t = tcase_create("modify");

    get_attr_tests(getattr);
    read_dir_tests(readdir);
    read_tests(read_t);
    modify_tests(modify_t);

    suite_add_tcase(s, getattr);
    suite_add_tcase(s, readdir);
    suite_add_tcase(s, read_t);
    suite_add_tcase(s, modify_t);

    SRunner *sr = srunner_create(s);
    srunner_set_fork_status(sr, CK_NOFORK);
    
    srunner_run_all(sr, CK_VERBOSE);
    int n_failed = srunner_ntests_failed(sr);
    printf("%d tests failed\n", n_failed);
    
    srunner_free(sr);
    return (n_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
