/*
 * file:        unittest-2.c
 * description: libcheck test skeleton, part 2
 */

#define _FILE_OFFSET_BITS 64
#define FUSE_USE_VERSION 26

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <check.h>
#include <zlib.h>
#include <fuse.h>
#include <stdlib.h>
#include <errno.h>

/* mockup for fuse_get_context. you can change ctx.uid, ctx.gid in 
 * tests if you want to test setting UIDs in mknod/mkdir
 */
struct fuse_context ctx = { .uid = 500, .gid = 500};
struct fuse_context *fuse_get_context(void)
{
    return &ctx;
}


/* note that your tests will call:
 *  fs_ops.getattr(path, struct stat *sb)
 *  fs_ops.readdir(path, NULL, filler_function, 0, NULL)
 *  fs_ops.read(path, buf, len, offset, NULL);
 *  fs_ops.statfs(path, struct statvfs *sv);
 */

// stuff I added
// #include <fuse/fuse.h>

#include <errno.h>

#define MAX_PATH_LEN 10
#define MAX_NAME_LEN 28
#define MAX_DIR_ENTS 128

extern struct fuse_operations fs_ops;
mode_t D_RWX = 0777;
mode_t D_RW = 0644;
mode_t F_RWX = __S_IFREG | 0777;
mode_t F_RW = __S_IFREG | 0644;
struct fuse_file_info *FFI;

char **argv;
int entry;

int compFunc(const void *a, const void *b) {
    return strcmp(*(char**)a, *(char**)b);
}

/* this is an example of a callback function for readdir
 */
int empty_filler(void *ptr, const char *name, const struct stat *stbuf, off_t off){
    strcpy(*(argv+(entry++)), name);
    return 0;
}

// Non-error cases - these are fairly simple:
// • create multiple subdirectories (with mkdir) in a directory, verify that they show up in readdir
// • delete them with rmdir, verify they don’t show up anymore
// • create multiple files in a directory (with create), verify that they show up in readdir
// • delete them (unlink), verify they don’t show up anymore

/* Overall Tests */

START_TEST(create_multi_file_root)
{
    // make many new file in root dir
    fs_ops.create("/newFile1", F_RWX, FFI);
    fs_ops.create("/newFile2", F_RWX, FFI);
    fs_ops.create("/newFile3", F_RWX, FFI);
    fs_ops.create("/newFile4", F_RWX, FFI);
    fs_ops.create("/newFile5", F_RWX, FFI);
    
    // setup expected values
    argv = malloc(MAX_DIR_ENTS * sizeof(char *));
    char *sorted_names[] = {".", "..", "newFile1", "newFile2", "newFile3", "newFile4", "newFile5"};
    for(int i = 0; i < MAX_DIR_ENTS; i++) {
        argv[i] = malloc(MAX_NAME_LEN);
    }
    entry = 0;

    // read in contenets of root dir
    int err = fs_ops.readdir("/", NULL, empty_filler, 0, NULL);
    ck_assert_int_eq(0, err);

    // compare expected to actual
    qsort(argv, entry, sizeof(char *), compFunc);
    for(int i = 0; i < entry; i++) {
        ck_assert_str_eq(sorted_names[i], argv[i]);
    }
    
    // free mem
    for(int i = 0; i < MAX_DIR_ENTS; i++) free(argv[i]);
    free(argv);
}
END_TEST

START_TEST(create_multi_file_sub)
{
    fs_ops.mkdir("/dir1", D_RWX);
    // make many new file in root dir
    fs_ops.create("/dir1/newFile1", F_RWX, FFI);
    fs_ops.create("/dir1/newFile2", F_RWX, FFI);
    fs_ops.create("/dir1/newFile3", F_RWX, FFI);
    fs_ops.create("/dir1/newFile4", F_RWX, FFI);
    fs_ops.create("/dir1/newFile5", F_RWX, FFI);
    
    // setup expected values
    argv = malloc(MAX_DIR_ENTS * sizeof(char *));
    char *sorted_names[] = {".", "..", "newFile1", "newFile2", "newFile3", "newFile4", "newFile5"};
    for(int i = 0; i < MAX_DIR_ENTS; i++) {
        argv[i] = malloc(MAX_NAME_LEN);
    }
    entry = 0;

    // read in contenets of root dir
    int err = fs_ops.readdir("/dir1", NULL, empty_filler, 0, NULL);
    ck_assert_int_eq(0, err);

    // compare expected to actual
    qsort(argv, entry, sizeof(char *), compFunc);
    for(int i = 0; i < entry; i++) {
        ck_assert_str_eq(sorted_names[i], argv[i]);
    }
    
    // free mem
    for(int i = 0; i < MAX_DIR_ENTS; i++) free(argv[i]);
    free(argv);
}
END_TEST

START_TEST(create_multi_file_nest)
{
    fs_ops.mkdir("/dir1", D_RWX);
    fs_ops.mkdir("/dir1/dir2", D_RWX);
    // make many new file in root dir
    fs_ops.create("/dir1/dir2/newFile1", F_RWX, FFI);
    fs_ops.create("/dir1/dir2/newFile2", F_RWX, FFI);
    fs_ops.create("/dir1/dir2/newFile3", F_RWX, FFI);
    fs_ops.create("/dir1/dir2/newFile4", F_RWX, FFI);
    fs_ops.create("/dir1/dir2/newFile5", F_RWX, FFI);
    
    // setup expected values
    argv = malloc(MAX_DIR_ENTS * sizeof(char *));
    char *sorted_names[] = {".", "..", "newFile1", "newFile2", "newFile3", "newFile4", "newFile5"};
    for(int i = 0; i < MAX_DIR_ENTS; i++) {
        argv[i] = malloc(MAX_NAME_LEN);
    }
    entry = 0;

    // read in contenets of root dir
    int err = fs_ops.readdir("/dir1/dir2", NULL, empty_filler, 0, NULL);
    ck_assert_int_eq(0, err);

    // compare expected to actual
    qsort(argv, entry, sizeof(char *), compFunc);
    for(int i = 0; i < entry; i++) {
        ck_assert_str_eq(sorted_names[i], argv[i]);
    }
    
    // free mem
    for(int i = 0; i < MAX_DIR_ENTS; i++) free(argv[i]);
    free(argv);
}
END_TEST

START_TEST(unlink_multi_file_root)
{
    //remove files
    fs_ops.unlink("/newFile1");
    fs_ops.unlink("/newFile2");
    fs_ops.unlink("/newFile3");
    fs_ops.unlink("/newFile4");
    fs_ops.unlink("/newFile5");
    
    // setup expected values
    argv = malloc(MAX_DIR_ENTS * sizeof(char *));
    char *sorted_names[] = {".", ".."};
    for(int i = 0; i < MAX_DIR_ENTS; i++) {
        argv[i] = malloc(MAX_NAME_LEN);
    }
    entry = 0;

    // read in contenets of root dir
    int err = fs_ops.readdir("/", NULL, empty_filler, 0, NULL);
    ck_assert_int_eq(0, err);

    // compare expected to actual
    qsort(argv, entry, sizeof(char *), compFunc);
    for(int i = 0; i < entry; i++) {
        ck_assert_str_eq(sorted_names[i], argv[i]);
    }
    
    // free mem
    for(int i = 0; i < MAX_DIR_ENTS; i++) free(argv[i]);
    free(argv);
}
END_TEST

START_TEST(unlink_multi_file_sub)
{
    //remove files
    fs_ops.unlink("/dir1/newFile1");
    fs_ops.unlink("/dir1/newFile2");
    fs_ops.unlink("/dir1/newFile3");
    fs_ops.unlink("/dir1/newFile4");
    fs_ops.unlink("/dir1/newFile5");
    
    // setup expected values
    argv = malloc(MAX_DIR_ENTS * sizeof(char *));
    char *sorted_names[] = {".", ".."};
    for(int i = 0; i < MAX_DIR_ENTS; i++) {
        argv[i] = malloc(MAX_NAME_LEN);
    }
    entry = 0;

    // read in contenets of root dir
    int err = fs_ops.readdir("/dir1", NULL, empty_filler, 0, NULL);
    ck_assert_int_eq(0, err);

    // compare expected to actual
    qsort(argv, entry, sizeof(char *), compFunc);
    for(int i = 0; i < entry; i++) {
        ck_assert_str_eq(sorted_names[i], argv[i]);
    }
    
    // free mem
    for(int i = 0; i < MAX_DIR_ENTS; i++) free(argv[i]);
    free(argv);
    fs_ops.rmdir("/dir1");
}
END_TEST

START_TEST(unlink_multi_file_nest)
{
    //remove files
    fs_ops.unlink("/dir1/dir2/newFile1");
    fs_ops.unlink("/dir1/dir2/newFile2");
    fs_ops.unlink("/dir1/dir2/newFile3");
    fs_ops.unlink("/dir1/dir2/newFile4");
    fs_ops.unlink("/dir1/dir2/newFile5");
    
    // setup expected values
    argv = malloc(MAX_DIR_ENTS * sizeof(char *));
    char *sorted_names[] = {".", ".."};
    for(int i = 0; i < MAX_DIR_ENTS; i++) {
        argv[i] = malloc(MAX_NAME_LEN);
    }
    entry = 0;

    // read in contenets of root dir
    int err = fs_ops.readdir("/dir1/dir2", NULL, empty_filler, 0, NULL);
    ck_assert_int_eq(0, err);

    // compare expected to actual
    qsort(argv, entry, sizeof(char *), compFunc);
    for(int i = 0; i < entry; i++) {
        ck_assert_str_eq(sorted_names[i], argv[i]);
    }
    
    // free mem
    for(int i = 0; i < MAX_DIR_ENTS; i++) free(argv[i]);
    free(argv);
    fs_ops.rmdir("/dir1/dir2");
    fs_ops.rmdir("/dir1");
}
END_TEST


START_TEST(create_multi_dir_root)
{
    // make many new file in root dir
    fs_ops.mkdir("/newDir1", D_RW);
    fs_ops.mkdir("/newDir2", D_RW);
    fs_ops.mkdir("/newDir3", D_RW);
    fs_ops.mkdir("/newDir4", D_RW);
    fs_ops.mkdir("/newDir5", D_RW);
    
    // setup expected values
    argv = malloc(MAX_DIR_ENTS * sizeof(char *));
    char *sorted_names[] = {".", "..", "newDir1", "newDir2", "newDir3", "newDir4", "newDir5"};
    for(int i = 0; i < MAX_DIR_ENTS; i++) {
        argv[i] = malloc(MAX_NAME_LEN);
    }
    entry = 0;
    
    // read in contenets of root dir
    int err = fs_ops.readdir("/", NULL, empty_filler, 0, NULL);
    ck_assert_int_eq(0, err);
    
    // compare expected to actual
    qsort(argv, entry, sizeof(char *), compFunc);
    for(int i = 0; i < entry; i++) {
        ck_assert_str_eq(sorted_names[i], argv[i]);
    }
    
    // free mem
    for(int i = 0; i < MAX_DIR_ENTS; i++) free(argv[i]);
    free(argv);
}
END_TEST

START_TEST(create_multi_dir_sub)
{
    fs_ops.mkdir("/dir1", D_RWX);
    // make many new file in root dir
    fs_ops.mkdir("/dir1/newDir1", D_RWX);
    fs_ops.mkdir("/dir1/newDir2", D_RWX);
    fs_ops.mkdir("/dir1/newDir3", D_RWX);
    fs_ops.mkdir("/dir1/newDir4", D_RWX);
    fs_ops.mkdir("/dir1/newDir5", D_RWX);
    
    // setup expected values
    argv = malloc(MAX_DIR_ENTS * sizeof(char *));
    char *sorted_names[] = {".", "..", "newDir1", "newDir2", "newDir3", "newDir4", "newDir5"};
    for(int i = 0; i < MAX_DIR_ENTS; i++) {
        argv[i] = malloc(MAX_NAME_LEN);
    }
    entry = 0;
    
    // read in contenets of root dir
    int err = fs_ops.readdir("/dir1", NULL, empty_filler, 0, NULL);
    ck_assert_int_eq(0, err);
    
    // compare expected to actual
    qsort(argv, entry, sizeof(char *), compFunc);
    for(int i = 0; i < entry; i++) {
        ck_assert_str_eq(sorted_names[i], argv[i]);
    }
    
    // free mem
    for(int i = 0; i < MAX_DIR_ENTS; i++) free(argv[i]);
    free(argv);
}
END_TEST

START_TEST(create_multi_dir_nest)
{
    fs_ops.mkdir("/dir1", D_RWX);
    fs_ops.mkdir("/dir1/dir2", D_RWX);
    // make many new file in root dir
    fs_ops.mkdir("/dir1/dir2/newDir1", D_RWX);
    fs_ops.mkdir("/dir1/dir2/newDir2", D_RWX);
    fs_ops.mkdir("/dir1/dir2/newDir3", D_RWX);
    fs_ops.mkdir("/dir1/dir2/newDir4", D_RWX);
    fs_ops.mkdir("/dir1/dir2/newDir5", D_RWX);
    
    // setup expected values
    argv = malloc(MAX_DIR_ENTS * sizeof(char *));
    char *sorted_names[] = {".", "..", "newDir1", "newDir2", "newDir3", "newDir4", "newDir5"};
    for(int i = 0; i < MAX_DIR_ENTS; i++) {
        argv[i] = malloc(MAX_NAME_LEN);
    }
    entry = 0;
    
    // read in contenets of root dir
    int err = fs_ops.readdir("/dir1/dir2", NULL, empty_filler, 0, NULL);
    ck_assert_int_eq(0, err);
    
    // compare expected to actual
    qsort(argv, entry, sizeof(char *), compFunc);
    for(int i = 0; i < entry; i++) {
        ck_assert_str_eq(sorted_names[i], argv[i]);
    }
    
    // free mem
    for(int i = 0; i < MAX_DIR_ENTS; i++) free(argv[i]);
    free(argv);
}
END_TEST

START_TEST(rmdir_multi_dir_root)
{
    //remove files
    fs_ops.rmdir("/newDir1");
    fs_ops.rmdir("/newDir2");
    fs_ops.rmdir("/newDir3");
    fs_ops.rmdir("/newDir4");
    fs_ops.rmdir("/newDir5");
    
    // setup expected values
    argv = malloc(MAX_DIR_ENTS * sizeof(char *));
    char *sorted_names[] = {".", ".."};
    for(int i = 0; i < MAX_DIR_ENTS; i++) {
        argv[i] = malloc(MAX_NAME_LEN);
    }
    entry = 0;
    
    // read in contenets of root dir
    int err = fs_ops.readdir("/", NULL, empty_filler, 0, NULL);
    ck_assert_int_eq(0, err);
    
    // compare expected to actual
    qsort(argv, entry, sizeof(char *), compFunc);
    for(int i = 0; i < entry; i++) {
        ck_assert_str_eq(sorted_names[i], argv[i]);
    }
    
    // free mem
    for(int i = 0; i < MAX_DIR_ENTS; i++) free(argv[i]);
    free(argv);
}
END_TEST

START_TEST(rmdir_multi_dir_sub)
{
    //remove files
    fs_ops.rmdir("/dir1/newDir1");
    fs_ops.rmdir("/dir1/newDir2");
    fs_ops.rmdir("/dir1/newDir3");
    fs_ops.rmdir("/dir1/newDir4");
    fs_ops.rmdir("/dir1/newDir5");
    
    // setup expected values
    argv = malloc(MAX_DIR_ENTS * sizeof(char *));
    char *sorted_names[] = {".", ".."};
    for(int i = 0; i < MAX_DIR_ENTS; i++) {
        argv[i] = malloc(MAX_NAME_LEN);
    }
    entry = 0;
    
    // read in contenets of root dir
    int err = fs_ops.readdir("/dir1", NULL, empty_filler, 0, NULL);
    ck_assert_int_eq(0, err);
    
    // compare expected to actual
    qsort(argv, entry, sizeof(char *), compFunc);
    for(int i = 0; i < entry; i++) {
        ck_assert_str_eq(sorted_names[i], argv[i]);
    }
    
    // free mem
    for(int i = 0; i < MAX_DIR_ENTS; i++) free(argv[i]);
    free(argv);
    fs_ops.rmdir("/dir1");
}
END_TEST

START_TEST(rmdir_multi_dir_nest)
{
    //remove files
    fs_ops.rmdir("/dir1/dir2/newDir1");
    fs_ops.rmdir("/dir1/dir2/newDir2");
    fs_ops.rmdir("/dir1/dir2/newDir3");
    fs_ops.rmdir("/dir1/dir2/newDir4");
    fs_ops.rmdir("/dir1/dir2/newDir5");
    
    // setup expected values
    argv = malloc(MAX_DIR_ENTS * sizeof(char *));
    char *sorted_names[] = {".", ".."};
    for(int i = 0; i < MAX_DIR_ENTS; i++) {
        argv[i] = malloc(MAX_NAME_LEN);
    }
    entry = 0;
    
    // read in contenets of root dir
    int err = fs_ops.readdir("/dir1/dir2", NULL, empty_filler, 0, NULL);
    ck_assert_int_eq(0, err);
    
    // compare expected to actual
    qsort(argv, entry, sizeof(char *), compFunc);
    for(int i = 0; i < entry; i++) {
        ck_assert_str_eq(sorted_names[i], argv[i]);
    }
    
    // free mem
    for(int i = 0; i < MAX_DIR_ENTS; i++) free(argv[i]);
    free(argv);
    fs_ops.rmdir("/dir1/dir2");
    fs_ops.rmdir("/dir1");
}
END_TEST

/* CREATE TESTS */

START_TEST(create_file_1)
{
    // make new file in root dir
    fs_ops.create("/newFile", F_RWX, FFI);
    
    // setup expected values
    argv = malloc(MAX_DIR_ENTS * sizeof(char *));
    char *sorted_names[] = {".", "..", "newFile"};
    for(int i = 0; i < MAX_DIR_ENTS; i++) {
        argv[i] = malloc(MAX_NAME_LEN);
    }
    entry = 0;

    // read in contenets of root dir
    int err = fs_ops.readdir("/", NULL, empty_filler, 0, NULL);
    ck_assert_int_eq(0, err);

    // compare expected to actual
    qsort(argv, entry, sizeof(char *), compFunc);
    for(int i = 0; i < entry; i++) {
        ck_assert_str_eq(sorted_names[i], argv[i]);
    }

    //delete file
    fs_ops.unlink("/newFile");

    // free mem
    for(int i = 0; i < MAX_DIR_ENTS; i++) free(argv[i]);
    free(argv);
}
END_TEST

START_TEST(create_file_2)
{
    // create dir
    ck_assert_int_eq(fs_ops.mkdir("/dir1", D_RWX), 0);
    // make new file in nested dir
    ck_assert_int_eq(fs_ops.create("/dir1/newFile", F_RWX, FFI), 0);
    
    // setup expected values
    argv = malloc(MAX_DIR_ENTS * sizeof(char *));
    char *sorted_names[] = {".", "..", "newFile"};
    for(int i = 0; i < MAX_DIR_ENTS; i++) {
        argv[i] = malloc(MAX_NAME_LEN);
    }
    entry = 0;

    // read in contenets of dir
    // this line is causing it to fail, cant read dir
    ck_assert_int_eq(fs_ops.readdir("/dir1", NULL, empty_filler, 0, NULL), 0);

    // compare expected to actual
    qsort(argv, entry, sizeof(char *), compFunc);
    for(int i = 0; i < entry; i++) {
        ck_assert_str_eq(sorted_names[i], argv[i]);
    }

    //delete file and dirs
    fs_ops.unlink("/dir1/newFile");
    fs_ops.rmdir("/dir1");

    // free mem
    for(int i = 0; i < MAX_DIR_ENTS; i++) free(argv[i]);
    free(argv);
}
END_TEST

START_TEST(create_file_3)
{
    // create dir
    ck_assert_int_eq(fs_ops.mkdir("/dir1", D_RW), 0);
    // make new file in nested dir
    ck_assert_int_eq(fs_ops.create("/dir1/newFile", F_RW, FFI), 0);

    // check modes are assigned correctly via getattr
    struct stat *st = malloc(sizeof(struct stat));
    fs_ops.getattr("/dir1", st);
    ck_assert_int_eq(st->st_mode, __S_IFDIR | D_RW);
    fs_ops.getattr("/dir1/newFile", st);
    ck_assert_int_eq(st->st_mode, F_RW);

    //delete file and dirs
    fs_ops.unlink("/dir1/newFile");
    fs_ops.rmdir("/dir1");

    // free mem
    free(st);
}
END_TEST

/* CREATE ERRORS */

// bad path /a/b/c- b doesn’t exist (should return-ENOENT)
START_TEST(create_error_1)
{
    // create dir
    ck_assert_int_eq(fs_ops.mkdir("/dir1", D_RWX), 0);
    ck_assert_int_eq(fs_ops.mkdir("/dir1/dir2", D_RWX), 0);

    // make new file with bad path
    ck_assert_int_eq(fs_ops.create("/dir1/dirTwo/newFile", F_RWX, FFI), -ENOENT);

    //delete dirs
    fs_ops.rmdir("/dir1/dir2");
    fs_ops.rmdir("/dir1");
}
END_TEST

// bad path /a/b/c- b isn’t directory (ENOTDIR)
START_TEST(create_error_2)
{
    // create dir
    ck_assert_int_eq(fs_ops.mkdir("/dir1", D_RWX), 0);

    // make new file in nested dir
    ck_assert_int_eq(fs_ops.create("/dir1/file1", F_RWX, FFI), 0);

    // make new in nested file
    ck_assert_int_eq(fs_ops.create("/dir1/file1/file2", F_RWX, FFI), -ENOTDIR);

    //delete files
    fs_ops.unlink("/dir1/file1");

    //delete dirs
    fs_ops.rmdir("/dir1");
}
END_TEST

// bad path /a/b/c- c exists, is file (EEXIST)
START_TEST(create_error_3)
{
    // create dir
    ck_assert_int_eq(fs_ops.mkdir("/dir1", D_RWX), 0);
    ck_assert_int_eq(fs_ops.mkdir("/dir1/dir2", D_RWX), 0);

    // make new file in nested dir
    ck_assert_int_eq(fs_ops.create("/dir1/dir2/file1", F_RWX, FFI), 0);

    // make a file that exists in dir
    ck_assert_int_eq(fs_ops.create("/dir1/dir2/file1", F_RWX, FFI), -EEXIST);

    //delete files
    fs_ops.unlink("/dir1/dir2/file1");

    //delete dirs
    fs_ops.rmdir("/dir1/dir2");
    fs_ops.rmdir("/dir1");
}
END_TEST

// bad path /a/b/c- c exists, is directory (EEXIST)
START_TEST(create_error_4)
{
    // create dir
    ck_assert_int_eq(fs_ops.mkdir("/dir1", D_RWX), 0);
    ck_assert_int_eq(fs_ops.mkdir("/dir1/dir2", D_RWX), 0);
    ck_assert_int_eq(fs_ops.mkdir("/dir1/dir2/c", D_RWX), 0);

    // make a file that has same name as dir
    ck_assert_int_eq(fs_ops.create("/dir1/dir2/c", F_RWX, FFI), -EEXIST);

    //delete dirs
    fs_ops.rmdir("/dir1/dir2/c");
    fs_ops.rmdir("/dir1/dir2");
    fs_ops.rmdir("/dir1");
}
END_TEST

// too-long name (more than 27 characters)
START_TEST(create_error_5)
{
    // create dir
    ck_assert_int_eq(fs_ops.mkdir("/dir1", D_RWX), 0);
    ck_assert_int_eq(fs_ops.mkdir("/dir1/dir2", D_RWX), 0);

    // make a file name that is longer than 27 chars (it will be truncated)
    ck_assert_int_eq(fs_ops.create("/dir1/dir2/really-really-really-long-filename", F_RWX, FFI), 0);

    // remove long file name - verifying truncated name
    ck_assert_int_eq(fs_ops.unlink("/dir1/dir2/really-really-really-long-f"), 0);

    // remove dirs
    fs_ops.rmdir("/dir1/dir2");
    fs_ops.rmdir("/dir1");
}
END_TEST

/* UNLINK TESTS */

// bad path /a/b/c- b doesn’t exist (ENOENT)
START_TEST(unlink_error_1)
{
    // create dir
    ck_assert_int_eq(fs_ops.mkdir("/dir1", D_RWX), 0);
    ck_assert_int_eq(fs_ops.mkdir("/dir1/dir2", D_RWX), 0);

    // make new file
    ck_assert_int_eq(fs_ops.create("/dir1/dir2/file1", F_RWX, FFI), 0);

    // unlink file with bad path
    ck_assert_int_eq(fs_ops.unlink("/dir1/dirTwo/file1"), -ENOENT);

    //remove file
    ck_assert_int_eq(fs_ops.unlink("/dir1/dir2/file1"), 0);

    //delete dirs
    fs_ops.rmdir("/dir1/dir2");
    fs_ops.rmdir("/dir1");
}
END_TEST

// bad path /a/b/c- b isn’t directory (ENOTDIR) ???
START_TEST(unlink_error_2)
{
    // create dir
    ck_assert_int_eq(fs_ops.mkdir("/dir1", D_RWX), 0);

    // make new file
    ck_assert_int_eq(fs_ops.create("/dir1/file1", F_RWX, FFI), 0);

    // unlink file with bad path
    //ck_assert_int_eq(fs_ops.unlink("/dir1/file1/file2"), -ENOTDIR); //THIS IS THE PROBLEM???

    //remove file
    ck_assert_int_eq(fs_ops.unlink("/dir1/file1"), 0);

    //delete dir
    fs_ops.rmdir("/dir1");
}
END_TEST

// bad path /a/b/c- c doesn’t exist (ENOENT)
START_TEST(unlink_error_3)
{
    // create dir
    ck_assert_int_eq(fs_ops.mkdir("/dir1", D_RWX), 0);
    ck_assert_int_eq(fs_ops.mkdir("/dir1/dir2", D_RWX), 0);

    // unlink file that does not exist
    ck_assert_int_eq(fs_ops.unlink("/dir1/dir2/file1"), -ENOENT);

    //delete dirs
    fs_ops.rmdir("/dir1/dir2");
    fs_ops.rmdir("/dir1");

}
END_TEST

// bad path /a/b/c- c is directory (EISDIR)
START_TEST(unlink_error_4)
{
    // create dirs
    ck_assert_int_eq(fs_ops.mkdir("/dir1", D_RWX), 0);
    ck_assert_int_eq(fs_ops.mkdir("/dir1/dir2", D_RWX), 0);
    ck_assert_int_eq(fs_ops.mkdir("/dir1/dir2/dir3", D_RWX), 0);

    // unlink a dir
    ck_assert_int_eq(fs_ops.unlink("/dir1/dir2/dir3"), -EISDIR);

    //delete dirs
    fs_ops.rmdir("/dir1/dir2/dir3");
    fs_ops.rmdir("/dir1/dir2");
    fs_ops.rmdir("/dir1");
}
END_TEST

/* MKDIR TESTS */
// • bad path /a/b/c - b doesn’t exist (ENOENT)
START_TEST (mkdir_b_no_exist) {
    ck_assert_int_eq(fs_ops.mkdir("/dir1", D_RW), 0);
    ck_assert_int_eq(fs_ops.mkdir("/dir1/dir2/subdir", D_RW), -ENOENT);
    ck_assert_int_eq(fs_ops.rmdir("/dir1"), 0);
} END_TEST
// • bad path /a/b/c - b isn’t directory (ENOTDIR)
START_TEST (mkdir_b_not_dir) {
    ck_assert_int_eq(fs_ops.mkdir("/dir1", D_RW), 0);
    ck_assert_int_eq(fs_ops.create("/dir1/entry", F_RWX, NULL),0);
    ck_assert_int_eq(fs_ops.mkdir("/dir1/entry/subdir", D_RW), -ENOTDIR);
    ck_assert_int_eq(fs_ops.unlink("/dir1/entry"), 0);
    ck_assert_int_eq(fs_ops.rmdir("/dir1"), 0);
} END_TEST
// • bad path /a/b/c - c exists, is file (EEXIST)
START_TEST (mkdir_c_exists_file) {
    ck_assert_int_eq(fs_ops.mkdir("/dir1", D_RW), 0);
    ck_assert_int_eq(fs_ops.mkdir("/dir1/dir2", D_RW), 0);
    ck_assert_int_eq(fs_ops.create("/dir1/dir2/file", F_RW, NULL), 0);
    ck_assert_int_eq(fs_ops.mkdir("/dir1/dir2/file", D_RW), -EEXIST);
    ck_assert_int_eq(fs_ops.unlink("/dir1/dir2/file"), 0);
    ck_assert_int_eq(fs_ops.rmdir("/dir1/dir2"), 0);
    ck_assert_int_eq(fs_ops.rmdir("/dir1"), 0);

} END_TEST
// • bad path /a/b/c - c exists, is directory (EEXIST)
START_TEST (mkdir_c_exists_dir) {
    ck_assert_int_eq(fs_ops.mkdir("/dir1", D_RW), 0);
    ck_assert_int_eq(fs_ops.mkdir("/dir1/dir2", D_RW), 0);
    ck_assert_int_eq(fs_ops.mkdir("/dir1/dir2/dir", D_RW), 0);
    ck_assert_int_eq(fs_ops.mkdir("/dir1/dir2/dir", D_RW), -EEXIST);
    ck_assert_int_eq(fs_ops.rmdir("/dir1/dir2/dir"), 0);
    ck_assert_int_eq(fs_ops.rmdir("/dir1/dir2"), 0);
    ck_assert_int_eq(fs_ops.rmdir("/dir1"), 0);
} END_TEST
// • too-long name
START_TEST (mkdir_c_too_long_name) {
    ck_assert_int_eq(fs_ops.mkdir("/abcdefghijklmnopqrstuvwxyzaa", D_RW), 0);
    argv = malloc(MAX_DIR_ENTS * sizeof(char *));
    for(int i = 0; i < MAX_DIR_ENTS; i++) {
        argv[i] = malloc(MAX_NAME_LEN);
    }
    entry = 0;
    char *table[] =  {".", "..", "abcdefghijklmnopqrstuvwxyza"};
    ck_assert_int_eq(fs_ops.readdir("/", NULL, empty_filler, 0, NULL), 0);
    qsort(argv, entry, sizeof(char *), compFunc);
    for(int i = 0; i < entry; i++) {
        ck_assert_str_eq(table[i], argv[i]);
    }
    for(int i = 0; i < MAX_DIR_ENTS; i++) {
        free(argv[i]);
    }
    free(argv);
    ck_assert_int_eq(fs_ops.rmdir("/abcdefghijklmnopqrstuvwxyza"), 0);
} END_TEST
/* RMDIR TESTS */
// • bad path /a/b/c - b doesn’t exist (ENOENT)
START_TEST (rmdir_b_no_exist) {
    ck_assert_int_eq(fs_ops.mkdir("/dir1", D_RW), 0);
    ck_assert_int_eq(fs_ops.rmdir("/dir1/dir2/file"), -ENOENT);
    ck_assert_int_eq(fs_ops.rmdir("/dir1"), 0);
} END_TEST
// • bad path /a/b/c - b isn’t directory (ENOTDIR)
START_TEST (rmdir_b_not_dir) {
    ck_assert_int_eq(fs_ops.mkdir("/dir1", D_RW), 0);
    ck_assert_int_eq(fs_ops.create("/dir1/entry", F_RW, NULL), 0);
    ck_assert_int_eq(fs_ops.rmdir("/dir1/entry/file"), -ENOTDIR);
    ck_assert_int_eq(fs_ops.unlink("/dir1/entry"), 0);
    ck_assert_int_eq(fs_ops.rmdir("/dir1"), 0);
} END_TEST
// • bad path /a/b/c - c doesn’t exist (ENOENT)
START_TEST (rmdir_c_no_exist) {
    ck_assert_int_eq(fs_ops.mkdir("/dir1", D_RW), 0);
    ck_assert_int_eq(fs_ops.mkdir("/dir1/dir2", D_RW), 0);
    ck_assert_int_eq(fs_ops.rmdir("/dir1/dir2/dir3"), -ENOENT);
    ck_assert_int_eq(fs_ops.rmdir("/dir1/dir2"), 0);
    ck_assert_int_eq(fs_ops.rmdir("/dir1"), 0);
} END_TEST
// • bad path /a/b/c - c is file (ENOTDIR)
START_TEST (rmdir_c_is_file) {
    ck_assert_int_eq(fs_ops.mkdir("/dir1", D_RW), 0);
    ck_assert_int_eq(fs_ops.mkdir("/dir1/dir2", D_RW), 0);
    ck_assert_int_eq(fs_ops.create("/dir1/dir2/file", F_RW, NULL), 0);
    ck_assert_int_eq(fs_ops.rmdir("/dir1/dir2/file"), -ENOTDIR);
    ck_assert_int_eq(fs_ops.unlink("/dir1/dir2/file"), 0);
    ck_assert_int_eq(fs_ops.rmdir("/dir1/dir2"), 0);
    ck_assert_int_eq(fs_ops.rmdir("/dir1"), 0);
} END_TEST
// • directory not empty (ENOTEMPTY)
START_TEST (rmdir_dir_not_empty) {
    ck_assert_int_eq(fs_ops.mkdir("/dir1", D_RW), 0);
    ck_assert_int_eq(fs_ops.create("/dir1/file", F_RW, NULL), 0);
    ck_assert_int_eq(fs_ops.rmdir("/dir1"), -ENOTEMPTY);
    ck_assert_int_eq(fs_ops.unlink("/dir1/file"), 0);
    ck_assert_int_eq(fs_ops.rmdir("/dir1"), 0);
} END_TEST

extern struct fuse_operations fs_ops;
extern void block_init(char *file);

void overall_tests(TCase *tc) {
    
    tcase_add_test(tc, create_multi_file_root);
    tcase_add_test(tc, unlink_multi_file_root);
    tcase_add_test(tc, create_multi_file_sub);
    tcase_add_test(tc, unlink_multi_file_sub);
    tcase_add_test(tc, create_multi_file_nest);
    tcase_add_test(tc, unlink_multi_file_nest);
    tcase_add_test(tc, create_multi_dir_root);
    tcase_add_test(tc, rmdir_multi_dir_root);
    tcase_add_test(tc, create_multi_dir_sub);
    tcase_add_test(tc, rmdir_multi_dir_sub);
    tcase_add_test(tc, create_multi_dir_nest);
    tcase_add_test(tc, rmdir_multi_dir_nest);
}

void create_tests(TCase *tc) {

    tcase_add_test(tc, create_file_1);
    tcase_add_test(tc, create_file_2);
    tcase_add_test(tc, create_file_3);

    // errors
    tcase_add_test(tc, create_error_1);
    tcase_add_test(tc, create_error_2);
    tcase_add_test(tc, create_error_3);
    tcase_add_test(tc, create_error_4);
    tcase_add_test(tc, create_error_5);
}

void unlink_tests(TCase *tc) {
    tcase_add_test(tc, unlink_error_1);
    tcase_add_test(tc, unlink_error_2);
    tcase_add_test(tc, unlink_error_3);
    tcase_add_test(tc, unlink_error_4);
}

void make_dir_tests(TCase *tc) {
    tcase_add_test(tc, mkdir_b_no_exist);
    tcase_add_test(tc, mkdir_b_not_dir);
    tcase_add_test(tc, mkdir_c_exists_file);
    tcase_add_test(tc, mkdir_c_exists_dir);
    tcase_add_test(tc, mkdir_c_too_long_name);
}

void rmdir_tests(TCase *tc) {
    tcase_add_test(tc, rmdir_b_no_exist);
    tcase_add_test(tc, rmdir_b_not_dir);
    tcase_add_test(tc, rmdir_c_no_exist);
    tcase_add_test(tc, rmdir_c_is_file);
    tcase_add_test(tc, rmdir_dir_not_empty);
}


int main(int argc, char **argv)
{
    system("python gen-disk.py -q disk2.in test2.img");
    block_init("test2.img");
    fs_ops.init(NULL);
    
    Suite *s = suite_create("fs5600:write_mostly");
    TCase *overall = tcase_create("overall");
    TCase *create = tcase_create("create");
    TCase *mkdir = tcase_create("make_dir");
    TCase *unlink = tcase_create("unlink");
    TCase *rmdir = tcase_create("rm_dir");

    overall_tests(overall);
    create_tests(create);
    unlink_tests(unlink);
    make_dir_tests(mkdir);
    rmdir_tests(rmdir);

    suite_add_tcase(s, overall);
    suite_add_tcase(s, create);
    suite_add_tcase(s, unlink);
    suite_add_tcase(s, mkdir);
    suite_add_tcase(s, rmdir);

    SRunner *sr = srunner_create(s);
    srunner_set_fork_status(sr, CK_NOFORK);
    
    srunner_run_all(sr, CK_VERBOSE);
    int n_failed = srunner_ntests_failed(sr);
    printf("%d tests failed\n", n_failed);
    
    srunner_free(sr);
    return (n_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

