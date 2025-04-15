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
#include <fuse/fuse.h>

#define MAX_PATH_LEN 10
#define MAX_NAME_LEN 28

extern struct fuse_operations fs_ops;
mode_t RWX = 0777;
struct fuse_file_info *FFI;

char **argv;
int entry;

int compFunc(const void *a, const void *b) {
    return strcmp(*(char**)a, *(char**)b);
}

/* this is an example of a callback function for readdir
 */
int empty_filler(void *ptr, const char *name, const struct stat *stbuf, off_t off){
    if (strcmp(".", name) != 0 && strcmp("..", name) != 0) {
        strcpy(*(argv+(entry++)), name);
    }
    return 0;
}

/* CREATE TESTS */

START_TEST(create_file_1)
{
    // make new file in root dir
    fs_ops.create("/newFile", RWX, FFI);
    
    // setup expected values
    argv = malloc(MAX_PATH_LEN * sizeof(char *));
    char *sorted_names[] = {"newFile"};
    for(int i = 0; i < MAX_PATH_LEN; i++) {
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
    for(int i = 0; i < MAX_PATH_LEN; i++) free(argv[i]);
    free(argv);
}
END_TEST

START_TEST(create_file_2)
{
    // make many new file in root dir
    fs_ops.create("/newFile1", RWX, FFI);
    fs_ops.create("/newFile2", RWX, FFI);
    fs_ops.create("/newFile3", RWX, FFI);
    fs_ops.create("/newFile4", RWX, FFI);
    fs_ops.create("/newFile5", RWX, FFI);
    
    // setup expected values
    argv = malloc(MAX_PATH_LEN * sizeof(char *));
    char *sorted_names[] = {"newFile1", "newFile2", "newFile3", "newFile4", "newFile5"};
    for(int i = 0; i < MAX_PATH_LEN; i++) {
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

    //remove files
    fs_ops.unlink("/newFile1");
    fs_ops.unlink("/newFile2");
    fs_ops.unlink("/newFile3");
    fs_ops.unlink("/newFile4");
    fs_ops.unlink("/newFile5");
    
    // free mem
    for(int i = 0; i < MAX_PATH_LEN; i++) free(argv[i]);
    free(argv);
}
END_TEST

START_TEST(create_file_3)
{
    // create dir
    ck_assert_int_eq(fs_ops.mkdir("/dir1", RWX), 0);
    // make new file in nested dir
    ck_assert_int_eq(fs_ops.create("/dir1/newFile", RWX, FFI), 0);
    
    // setup expected values
    argv = malloc(MAX_PATH_LEN * sizeof(char *));
    char *sorted_names[] = {"newFile"};
    for(int i = 0; i < MAX_PATH_LEN; i++) {
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
    for(int i = 0; i < MAX_PATH_LEN; i++) free(argv[i]);
    free(argv);
}
END_TEST

START_TEST(create_file_4)
{
    // create dir
    ck_assert_int_eq(fs_ops.mkdir("/dir1", RWX), 0);
    // make new file in nested dir
    ck_assert_int_eq(fs_ops.create("/dir1/newFile", __S_IFREG | RWX, FFI), 0);

    struct stat *st = malloc(sizeof(struct stat));
    fs_ops.getattr("/dir1", st);
    ck_assert_int_eq(st->st_mode, (__S_IFDIR | RWX));
    fs_ops.getattr("/dir1/newFile", st);
    ck_assert_int_eq(st->st_mode, (__S_IFREG | RWX));

    //delete file and dirs
    fs_ops.unlink("/dir1/newFile");
    fs_ops.rmdir("/dir1");

    // free mem
    free(st);
}
END_TEST

/* MKDIR TESTS */

/* UNLINK TESTS */

/* RMDIR TESTS */

extern struct fuse_operations fs_ops;
extern void block_init(char *file);

void create_tests(TCase *tc) {

    tcase_add_test(tc, create_file_1);
    tcase_add_test(tc, create_file_2);
    tcase_add_test(tc, create_file_3);
    tcase_add_test(tc, create_file_4);
}

void make_dir_tests(TCase *tc) {

    //tcase_add_test(tc, a_test);
}

void unlink_tests(TCase *tc) {

    //tcase_add_test(tc, a_test);
}

void rmdir_tests(TCase *tc) {

    //tcase_add_test(tc, a_test);
}


int main(int argc, char **argv)
{
    system("python gen-disk.py -q disk2.in test2.img");
    block_init("test2.img");
    fs_ops.init(NULL);
    
    Suite *s = suite_create("fs5600");
    TCase *tc = tcase_create("write_mostly");
    TCase *mkdir = tcase_create("make_dir");
    TCase *unlink = tcase_create("unlink");
    TCase *rmdir = tcase_create("rm_dir");

    create_tests(tc);
    //make_dir_tests(mkdir);
    //unlink_tests(unlink);
    //rmdir_tests(rmdir);

    suite_add_tcase(s, tc);
    //suite_add_tcase(s, mkdir);
    //suite_add_tcase(s, unlink);
    //suite_add_tcase(s, rmdir);

    SRunner *sr = srunner_create(s);
    srunner_set_fork_status(sr, CK_NOFORK);
    
    srunner_run_all(sr, CK_VERBOSE);
    int n_failed = srunner_ntests_failed(sr);
    printf("%d tests failed\n", n_failed);
    
    srunner_free(sr);
    return (n_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

