/*
 * file:        testing.c
 * description: libcheck test skeleton for file system project
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


/* change test name and make it do something useful */
// START_TEST(a_test)
// {
//     ck_assert_int_eq(1, 1);
// }
// END_TEST

// START_TEST(fs_getattr)
// {
//     struct stat stat;

//     char *path = "/this/is/a/file";
//     unsigned expected_cksum = 1234567;
    
//     int rv = fs_ops.getattr(path, &stat);
//     ck_assert(rv == 0);

//     char *buf = malloc(stat.st_size);
//     ck_assert(buf != NULL);

//     rv = fs_ops.read(path, buf, 0, stat.st_size, 0);
//     ck_assert(rv == stat.st_size);

//     unsigned cksum = crc32(0, buf, stat.st_size);
//     free(buf);
    
//     ck_assert(cksum == expected_cksum);
// }
// END_TEST

/* this is an example of a callback function for readdir
 */
int empty_filler(void *ptr, const char *name, const struct stat *stbuf,
                 off_t off)
{
    /* FUSE passes you the entry name and a pointer to a 'struct stat' 
     * with the attributes. Ignore the 'ptr' and 'off' arguments 
     * 
     */
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
}
END_TEST

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
}
END_TEST

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
}
END_TEST

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
}
END_TEST

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
}
END_TEST

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
}
END_TEST

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
}
END_TEST

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
}
END_TEST

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
}
END_TEST

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
}
END_TEST

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
}
END_TEST

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
}
END_TEST

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
}
END_TEST

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
}
END_TEST

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
}
END_TEST

START_TEST(no_entry) {
    struct stat *sb = malloc(sizeof(struct stat));
    int err = fs_ops.getattr("/not-a-file", sb);
    ck_assert_int_eq(-ENOENT, err);
    free(sb);
}
END_TEST

START_TEST(no_dir) {
    struct stat *sb = malloc(sizeof(struct stat));
    int err = fs_ops.getattr("/file.1k/file.0", sb);
    ck_assert_int_eq(-ENOTDIR, err);
    free(sb);
}
END_TEST

START_TEST(no_dir_mid) {
    struct stat *sb = malloc(sizeof(struct stat));
    int err = fs_ops.getattr("/files/file.12k-", sb);
    ck_assert_int_eq(-ENOENT, err);
    free(sb);
}
END_TEST

START_TEST(no_file_sub) {
    struct stat *sb = malloc(sizeof(struct stat));
    int err = fs_ops.getattr("/dir2/rouge_file", sb);
    ck_assert_int_eq(-ENOENT, err);
    free(sb);
}
END_TEST

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

int main(int argc, char **argv)
{
    system("python gen-disk.py -q disk1.in test.img");
    block_init("test.img");
    fs_ops.init(NULL);
    
    Suite *s = suite_create("fs5600:read_mostly");
    TCase *getattr = tcase_create("get_attr");

    get_attr_tests(getattr);

    suite_add_tcase(s, getattr);
    SRunner *sr = srunner_create(s);
    srunner_set_fork_status(sr, CK_NOFORK);
    
    srunner_run_all(sr, CK_VERBOSE);
    int n_failed = srunner_ntests_failed(sr);
    printf("%d tests failed\n", n_failed);
    
    srunner_free(sr);
    return (n_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
