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
START_TEST(a_test)
{
    ck_assert_int_eq(1, 1);
}
END_TEST

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

int main(int argc, char **argv)
{
    block_init("test.img");
    fs_ops.init(NULL);
    
    Suite *s = suite_create("fs5600");
    TCase *tc = tcase_create("read_mostly");

    tcase_add_test(tc, a_test); /* see START_TEST above */
    /* add more tests here */

    suite_add_tcase(s, tc);
    SRunner *sr = srunner_create(s);
    srunner_set_fork_status(sr, CK_NOFORK);
    
    srunner_run_all(sr, CK_VERBOSE);
    int n_failed = srunner_ntests_failed(sr);
    printf("%d tests failed\n", n_failed);
    
    srunner_free(sr);
    return (n_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
