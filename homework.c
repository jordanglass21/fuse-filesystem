/*
 * file: homework.c
 * description: skeleton file for CS 5600 system
 *
 * CS 5600, Computer Systems, Northeastern
 */

#define FUSE_USE_VERSION 27
#define _FILE_OFFSET_BITS 64

#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <fuse.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <sys/stat.h>

#include "fs5600.h"

/* if you don't understand why you can't use these system calls here, 
 * you need to read the assignment description another time
 */
#define stat(a,b) error do not use stat()
#define open(a,b) error do not use open()
#define read(a,b,c) error do not use read()
#define write(a,b,c) error do not use write()

#define INODE_SIZE 128 // is this right?

/* disk access. All access is in terms of 4KB blocks; read and
 * write functions return 0 (success) or -EIO.
 */
extern int block_read(void *buf, int lba, int nblks);
extern int block_write(void *buf, int lba, int nblks);

/* bitmap functions
 */
void bit_set(unsigned char *map, int i)
{
    map[i/8] |= (1 << (i%8));
}
void bit_clear(unsigned char *map, int i)
{
    map[i/8] &= ~(1 << (i%8));
}
int bit_test(unsigned char *map, int i)
{
    return map[i/8] & (1 << (i%8));
}

/* Global Variables and other definitions */

#define MAX_PATH_LEN 10
#define MAX_NAME_LEN 27

struct fs_super *super_block;

unsigned char *block_bitmap;

struct fs_inode *inodes;

/* Helper functions */
// Create function that reads the root dir
// pass in until valid == 0 and name empty
/*func:
    read inode -> global inode map for reading in???
    do mode check
    if mode == dir:
        read dir
        recursively call func passing in inode
*/
void process_init_read_in(struct fs_dirent * dir) {
    struct fs_dirent *entry = NULL;
    for(int i = 0; i < 128; i++){
        entry = dir + i;
        if(entry->valid){
            printf("i: %d\n", i);
            printf("valid: %d\n", entry->valid);
            printf("inode #: %d\n", entry->inode);
            printf("name: %s\n", entry->name);
            
            struct fs_inode *n_inode = inodes+(entry->inode);

            block_read(n_inode, entry->inode, 1);
            // int inodes_per_block = FS_BLOCK_SIZE / INODE_SIZE;
            // int data_region = 2 + (entry->inode / inodes_per_block);
            // block_read(n_inode, data_region, 1); // i think this is how we calculate the datablocks we read in?

            if(S_ISDIR(n_inode->mode)) {
                struct fs_dirent *n_dir = malloc(4096);
                block_read(n_dir, *(n_inode->ptrs), 1);
                process_init_read_in(n_dir);
                free(n_dir);
            }
        }
    }
}

int parse(char *path, char **argv) {
    int i;
    for (i = 0; i < MAX_PATH_LEN; i++) {
        if ((argv[i] = strtok(path, "/")) == NULL)
            break;
        if (strlen(argv[i]) > MAX_NAME_LEN)
            argv[i][MAX_NAME_LEN] = '\0';
        else
            argv[i][strlen(argv[i])] = '\0'; 
        path = NULL;
    }
    return i;
}

int translate(int pathc, char **pathv) {
    int inum = 2;
    int inode_found;
    struct fs_dirent *dir = malloc(4096);
    for(int i = 0; i < pathc; i++) {
        inode_found = 0;
        if(!S_ISDIR((inodes+inum)->mode)) {
            return -ENOTDIR;
        }

        block_read(dir, *((inodes+inum)->ptrs), 1);

        for(int j = 0; j < 128; j++) {
            if(strcmp((dir+j)->name, *(pathv+i)) == 0) {
                //return (dir + j)->inode; this is how it was before
                inum = (dir + j)->inode; // i think we need to keep looking to find a nested dir
                inode_found = 1;
                break;
            }
        }
    }

    free(dir);

    if(inode_found == 1) { // so now we return the deeply nested inode here
        return inum;
    }

    return -ENOENT;
}

int get_inum(char *pathd) {
    char **argv = (char **)malloc(MAX_PATH_LEN * (MAX_NAME_LEN * sizeof(char)));
    int pathc = parse(pathd, argv);
    int inum;
    if(pathc == 0) {
        inum = 2;
    } else {
        inum = translate(pathc, argv);
    }
    free(argv);
    free(pathd);
    return inum;
}

void fill_stat(struct fs_inode *iq, struct stat *sb, int inum) {
    sb->st_ino=inum;
    sb->st_atime=iq->mtime;
    sb->st_mtime=iq->mtime;
    sb->st_ctime=iq->ctime;
    sb->st_nlink=1;
    sb->st_uid=iq->uid;
    sb->st_gid=iq->gid;
    sb->st_size=iq->size;
    sb->st_mode=iq->mode;
}

/* init - this is called once by the FUSE framework at startup. Ignore
 * the 'conn' argument.
 * recommended actions:
 *   - read superblock
 *   - allocate memory, read bitmaps and inodes
 */
void* fs_init(struct fuse_conn_info *conn)
{
    // allocate memory for superblock
    super_block = malloc(sizeof(struct fs_super));

    // READ SUPERBLOCK
    block_read(super_block, 0, 1);

    // validate the magic number (5600)
    if (super_block->magic != 0x30303635) {
        printf("Superblock magic number invalid");
        free(super_block);
        return NULL;
    }

    // allocate memory for the block bitmap
    block_bitmap = malloc(4096);

    // READ BITMAP
    block_read(block_bitmap, 1, 1);

    inodes = malloc(super_block->disk_size * sizeof(struct fs_inode));

    // READ ROOT DIR INODE
    block_read(inodes+2, 2, 1);

    // int data_block_start = 3;
    // int data_block_end = 5; // how many data blocks are there???


    // prints to validate
    printf("Superblock details:\n");
    printf("Magic number: 0x%X\n", super_block->magic);
    printf("Disk size: %u\n", super_block->disk_size);

    printf("Block Bitmap: ");
    for (int i = 0; i < 2; i++) { // Print first 16 bits (2 bytes)
        printf("%02X ", block_bitmap[i]);
    }
    printf("\n");

    printf("Root Inode details:\n");
    printf("UID: %d\n", (inodes+2)->uid);
    printf("GID: %d\n", (inodes+2)->gid);
    printf("Mode: %o\n", (inodes+2)->mode & __S_IFMT);
    printf("Creation time: %u\n", (inodes+2)->ctime);
    printf("Modification time: %u\n", (inodes+2)->mtime);
    printf("Size: %d\n", (inodes+2)->size);
    printf("Size of arr: %ld\n", sizeof((inodes+2)->ptrs));
    printf("ptr: %d\n", *((inodes+2)->ptrs));
    
    struct fs_dirent *dirents = malloc(128 * sizeof(struct fs_dirent));
    block_read(dirents, *((inodes+2)->ptrs), 1);

    process_init_read_in(dirents);
    free(dirents);
    return super_block;
}

/* Note on path translation errors:
 * In addition to the method-specific errors listed below, almost
 * every method can return one of the following errors if it fails to
 * locate a file or directory corresponding to a specified path.
 *
 * ENOENT - a component of the path doesn't exist.
 * ENOTDIR - an intermediate component of the path (e.g. 'b' in
 *           /a/b/c) is not a directory
 */

/* note on splitting the 'path' variable:
 * the value passed in by the FUSE framework is declared as 'const',
 * which means you can't modify it. The standard mechanisms for
 * splitting strings in C (strtok, strsep) modify the string in place,
 * so you have to copy the string and then free the copy when you're
 * done. One way of doing this:
 *
 *    char *_path = strdup(path);
 *    int inum = translate(_path);
 *    free(_path);
 */



/* getattr - get file or directory attributes. For a description of
 *  the fields in 'struct stat', see 'man lstat'.
 *
 * Note - for several fields in 'struct stat' there is no corresponding
 *  information in our file system:
 *    st_nlink - always set it to 1
 *    st_atime, st_ctime - set to same value as st_mtime
 *
 * success - return 0
 * errors - path translation, ENOENT
 * hint - factor out inode-to-struct stat conversion - you'll use it
 *        again in readdir
 */
int fs_getattr(const char *path, struct stat *sb)
{
    /* your code here */
    char *pathd = strdup(path);
    int inum = get_inum(pathd);
    if(inum < 0) return inum;
    printf("inum: %d\n", inum);
    fill_stat(inodes+inum, sb, inum);
    return 0;
}

/* readdir - get directory contents.
 *
 * call the 'filler' function once for each valid entry in the 
 * directory, as follows:
 *     filler(buf, <name>, <statbuf>, 0)
 * where <statbuf> is a pointer to a struct stat
 * success - return 0
 * errors - path resolution, ENOTDIR, ENOENT
 * 
 * hint - check the testing instructions if you don't understand how
 *        to call the filler function
 */
int fs_readdir(const char *path, void *ptr, fuse_fill_dir_t filler,
		       off_t offset, struct fuse_file_info *fi)
{
    /* your code here */
    char *pathd = strdup(path);
    int inum = get_inum(pathd);
    if(inum < 0) return inum;
    
    printf("inum: %d\n", inum);

    if(!S_ISDIR((inodes+inum)->mode)) return -ENOTDIR;

    struct fs_dirent *dirents = malloc(128 * sizeof(struct fs_dirent));
    block_read(dirents, *((inodes+inum)->ptrs), 1);
    struct fs_inode *ino = malloc(sizeof(struct fs_inode));
    struct stat *sb = malloc(sizeof(struct stat));

    for(int i = 0; i < 128; i++) {
        struct fs_dirent *ent = dirents+i;
        if(ent->valid) {
            block_read(ino, ent->inode, 1);
            fill_stat(inodes+(ent->inode), sb, ent->inode);
            filler(ptr, ent->name, sb, 0);
        }
    }
    return 0;
}

/* create - create a new file with specified permissions
 *
 * success - return 0
 * errors - path resolution, EEXIST
 *          in particular, for create("/a/b/c") to succeed,
 *          "/a/b" must exist, and "/a/b/c" must not.
 *
 * Note that 'mode' will already have the S_IFREG bit set, so you can
 * just use it directly. Ignore the third parameter.
 *
 * If a file or directory of this name already exists, return -EEXIST.
 * If there are already 128 entries in the directory (i.e. it's filled an
 * entire block), you are free to return -ENOSPC instead of expanding it.
 */
int fs_create(const char *path, mode_t mode, struct fuse_file_info *fi)
{
    /* your code here */
    return -EOPNOTSUPP;
}

/* mkdir - create a directory with the given mode.
 *
 * WARNING: unlike fs_create, @mode only has the permission bits. You
 * have to OR it with S_IFDIR before setting the inode 'mode' field.
 *
 * success - return 0
 * Errors - path resolution, EEXIST
 * Conditions for EEXIST are the same as for create. 
 */ 
int fs_mkdir(const char *path, mode_t mode)
{
    /* your code here */
    return -EOPNOTSUPP;
}


/* unlink - delete a file
 *  success - return 0
 *  errors - path resolution, ENOENT, EISDIR
 */
int fs_unlink(const char *path)
{
    /* your code here */
    return -EOPNOTSUPP;
}

/* rmdir - remove a directory
 *  success - return 0
 *  Errors - path resolution, ENOENT, ENOTDIR, ENOTEMPTY
 */
int fs_rmdir(const char *path)
{
    /* your code here */
    return -EOPNOTSUPP;
}

/* rename - rename a file or directory
 * success - return 0
 * Errors - path resolution, ENOENT, EINVAL, EEXIST
 *
 * ENOENT - source does not exist
 * EEXIST - destination already exists
 * EINVAL - source and destination are not in the same directory
 *
 * Note that this is a simplified version of the UNIX rename
 * functionality - see 'man 2 rename' for full semantics. In
 * particular, the full version can move across directories, replace a
 * destination file, and replace an empty directory with a full one.
 */
int fs_rename(const char *src_path, const char *dst_path)
{
    /* your code here */
    return -EOPNOTSUPP;
}

/* chmod - change file permissions
 * utime - change access and modification times
 *         (for definition of 'struct utimebuf', see 'man utime')
 *
 * success - return 0
 * Errors - path resolution, ENOENT.
 */
int fs_chmod(const char *path, mode_t mode)
{
    /* your code here */
    return -EOPNOTSUPP;
}

int fs_utime(const char *path, struct utimbuf *ut)
{
    /* your code here */
    return -EOPNOTSUPP;
}

/* truncate - truncate file to exactly 'len' bytes
 * success - return 0
 * Errors - path resolution, ENOENT, EISDIR, EINVAL
 *    return EINVAL if len > 0.
 */
int fs_truncate(const char *path, off_t len)
{
    /* you can cheat by only implementing this for the case of len==0,
     * and an error otherwise.
     */
    if (len != 0)
	return -EINVAL;		/* invalid argument */

    /* your code here */
    return -EOPNOTSUPP;
}


/* read - read data from an open file.
 * success: should return exactly the number of bytes requested, except:
 *   - if offset >= file len, return 0
 *   - if offset+len > file len, return #bytes from offset to end
 *   - on error, return <0
 * Errors - path resolution, ENOENT, EISDIR
 */
int fs_read(const char *path, char *buf, size_t len, off_t offset,
	    struct fuse_file_info *fi)
{

    int inum = get_inum(strdup(path));
    struct fs_inode *inode = inodes+inum;
    if(inum < 0) {
        return inum;
    }
    if(S_ISDIR((inode)->mode)) {
        return -EISDIR;
    }

    int file_len = inode->size;

    if(offset >= file_len) {
        return 0;
    }

    char *temp_buf = malloc(FS_BLOCK_SIZE*DIV_ROUND_UP(inode->size, 4096));
    for(int i=0; i < 1019; i++) {
        if (inode->ptrs[i] == 0) {
            break;
        }
        block_read(temp_buf, inode->ptrs[i], 1);
    }

    
    if(offset+len > file_len) {
        memcpy(buf, temp_buf + offset, inode->size - offset);
        free(temp_buf);
        return (inode->size) - offset;
    } else {
        memcpy(buf, temp_buf + offset, len);
    }
    free(temp_buf);
    return len;

    // return 0;
}

/* write - write data to a file
 * success - return number of bytes written. (this will be the same as
 *           the number requested, or else it's an error)
 * Errors - path resolution, ENOENT, EISDIR
 *  return EINVAL if 'offset' is greater than current file length.
 *  (POSIX semantics support the creation of files with "holes" in them, 
 *   but we don't)
 */
int fs_write(const char *path, const char *buf, size_t len,
	     off_t offset, struct fuse_file_info *fi)
{
    /* your code here */
    return -EOPNOTSUPP;
}

/* statfs - get file system statistics
 * see 'man 2 statfs' for description of 'struct statvfs'.
 * Errors - none. Needs to work.
 */
int fs_statfs(const char *path, struct statvfs *st)
{
    /* needs to return the following fields (set others to zero):
     *   f_bsize = BLOCK_SIZE
     *   f_blocks = total image - (superblock + block map)
     *   f_bfree = f_blocks - blocks used
     *   f_bavail = f_bfree
     *   f_namemax = <whatever your max namelength is>
     *
     * it's OK to calculate this dynamically on the rare occasions
     * when this function is called.
     */
    /* your code here */
    return -EOPNOTSUPP;
}

/* operations vector. Please don't rename it, or else you'll break things
 */
struct fuse_operations fs_ops = {
    .init = fs_init,            /* read-mostly operations */
    .getattr = fs_getattr,
    .readdir = fs_readdir,
    .rename = fs_rename,
    .chmod = fs_chmod,
    .read = fs_read,
    .statfs = fs_statfs,

    .create = fs_create,        /* write operations */
    .mkdir = fs_mkdir,
    .unlink = fs_unlink,
    .rmdir = fs_rmdir,
    .utime = fs_utime,
    .truncate = fs_truncate,
    .write = fs_write,
};

