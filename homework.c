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
    int i =0;
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

int parse2(char *path, char **argv) {
    char *token = strtok(path, "/");
    for (int i = 0; i < MAX_PATH_LEN; i++) {
        if (token == NULL) {
            return i;
        }
        argv[i] = malloc(MAX_NAME_LEN);
        strncpy(argv[i], token, MAX_NAME_LEN-1);
        token = strtok(NULL, "/");
    }
    return MAX_PATH_LEN;
}

int translate(int pathc, char **pathv) {
    printf("pathc: %d\n", pathc);
    int inum = 2;
    int inode_found = 0;
    struct fs_dirent *dir = malloc(4096);
    for(int i = 0; i < pathc; i++) {
        printf("translate looking for: %s\n", pathv[i]);
        inode_found = 0;
        if(!S_ISDIR((inodes+inum)->mode)) {
            return -ENOTDIR;
        }

        block_read(dir, *((inodes+inum)->ptrs), 1);

        for(int j = 0; j < 128; j++) {
            if(dir[j].valid && strcmp((dir+j)->name, pathv[i]) == 0) {
                //return (dir + j)->inode; this is how it was before
                inum = dir[j].inode; // i think we need to keep looking to find a nested dir
                inode_found = 1;
                break;
            }
        }
    }

    free(dir);

    if(inode_found == 1 || pathc == 0) { // so now we return the deeply nested inode here
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

int get_used_blocks() {
    int used_count = 0;
    for(int i = 0; i < super_block->disk_size; i++) {
        if(bit_test(block_bitmap, i)) {
            used_count++;
        }
    }
    return used_count;
}

int find_free() {
    //int block = 0;
    for(int i = 0; i < super_block->disk_size; i++) {
        if(!bit_test(block_bitmap, i)) {
            //block = i;
            return i; // return the first free block, no?
        }
    }
    //if(block == 0) return -ENOSPC;
    //return block;
    return -ENOSPC;
}

int dir_empty(struct fs_inode *inode) {
    for (int i = 0; i < 6; i++) {
        int block = inode->ptrs[i];
        if (block == 0) {
            continue;
        }
        struct fs_dirent *entries = malloc(sizeof(struct fs_dirent) * 128);
        block_read(entries, block, 1);

        for (int j = 0; j < 128; j++) {
            if (entries[j].valid) {
                free(entries);
                return 0;
            }
        }

        free(entries);
    }
    return 1;
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

    // prints to validate
    printf("Superblock:\n");
    printf("Magic number: 0x%X\n", super_block->magic);
    printf("Disk Size: %u\n", super_block->disk_size);

    printf("Block bitmap: ");
    for (int i = 0; i < 2; i++) { // Print first 16 bits (2 bytes)
        printf("%02X ", block_bitmap[i]);
    }
    printf("\n");

    printf("Root inode details:\n");
    printf("UID: %d\n", (inodes+2)->uid);
    printf("GID: %d\n", (inodes+2)->gid);
    printf("Mode: %o\n", (inodes+2)->mode & __S_IFMT);
    printf("Creation time: %u\n", (inodes+2)->ctime);
    printf("Modification time: %u\n", (inodes+2)->mtime);
    printf("Size: %d\n", (inodes+2)->size);
    printf("Size of array: %ld\n", sizeof((inodes+2)->ptrs));
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
    char **argv = (char **)malloc(MAX_PATH_LEN * (MAX_NAME_LEN * sizeof(char)));
    
    int pathc = parse(pathd, argv);
    int prevInum = translate(pathc-1, argv);
    if(prevInum < 2) prevInum = 2;
    int inum = get_inum(strdup(path));
    free(pathd);
    free(argv);
    if(inum < 0) return inum;
    
    printf("inum: %d\n", inum);

    if(!S_ISDIR((inodes+inum)->mode)) return -ENOTDIR;

    struct fs_dirent *dirents = malloc(128 * sizeof(struct fs_dirent));
    block_read(dirents, *((inodes+inum)->ptrs), 1);
    struct fs_inode *ino = malloc(sizeof(struct fs_inode));
    struct stat *sb = malloc(sizeof(struct stat));
    fill_stat(inodes+inum, sb, inum);
    filler(ptr, ".", sb, 0);
    fill_stat(inodes+prevInum, sb, prevInum);
    filler(ptr, "..", sb, 0);
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
    char *pathd = strdup(path);
    char **pathv = (char **)malloc(MAX_PATH_LEN * (MAX_NAME_LEN * sizeof(char)));
    int pathc = parse(pathd, pathv);
    int dirInum = translate(pathc-1, pathv);
    
    if(dirInum < 0) return dirInum;
    int nInum = get_inum(strdup(path));
    if(nInum > 0) return -EEXIST;
    struct fs_dirent *dir = malloc(128 * sizeof(struct fs_dirent));
    block_read(dir, *((inodes+dirInum)->ptrs), 1);
    struct fs_dirent *entry;
    
    int i=0;
    for(; i < 128; i++) {
        entry = dir+i;
        if(!entry->valid && *(entry->name) == '\000') {
            break;
        }
    }
    
    if(i == 128) {
        free(dir);
        free(pathv);
        return -ENOSPC;
    }

    entry->valid = 1;
    strcpy(entry->name, *(pathv+(pathc-1)));
    nInum = find_free();
    if(nInum < 0) return nInum;
    bit_set(block_bitmap, nInum);
    entry->inode = nInum;
    struct fs_inode *fsi = inodes+nInum;
    fsi->mode=mode;
    (inodes+dirInum)->mtime=time(NULL);
    block_write(fsi, nInum, 1);
    block_write(inodes+(dirInum), dirInum, 1);
    block_write(dir, *((inodes+dirInum)->ptrs), 1);
    block_write(block_bitmap, 1, 1);
    return 0;
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
    int err = fs_create(path, __S_IFDIR | mode, NULL);
    if(err < 0) return err;
    struct fs_inode *nInode = inodes+get_inum(strdup(path));
    int freeblk = find_free();
    bit_set(block_bitmap, freeblk);
    memset(nInode->ptrs, 0, 4096);
    for(int i = 0; i < 1019; i++) {
        if(*((nInode->ptrs)+i) == 0) {
            *((nInode->ptrs)+i) = freeblk;
            break;
        }
    }
    nInode->ctime=time(NULL);
    nInode->mtime=time(NULL);
    nInode->size=4096;
    block_write(nInode,get_inum(strdup(path)), 1);
    block_write(block_bitmap, 1, 1);
    return err;
}


/* unlink - delete a file
 *  success - return 0
 *  errors - path resolution, ENOENT, EISDIR
 */
int fs_unlink(const char *path)
{
    // get inode number
    char *paths = strdup(path);
    int inum_source = get_inum(paths);
    if (inum_source < 0) {
        return -ENOENT;
    }

    // get inode
    struct fs_inode *inode = inodes+inum_source;

    if ((inode->mode & __S_IFMT) == __S_IFDIR) {
        // if its a dir
        return -EISDIR;
    }

    // find parent dir
    char *pathd = strdup(path);
    char **argv = malloc(MAX_PATH_LEN * sizeof(char *));
    for (int i = 0; i < MAX_PATH_LEN; i++) {
        argv[i] = malloc(MAX_NAME_LEN);
    }

    int pathc = parse2(pathd, argv);
    free(pathd);

    int parent;
    if (pathc == 1) {
        parent = 2; // parent dir is the root
    } else {
        parent = translate(pathc - 1, argv); // get the parent dir
    }

    printf("parent: %d\n", parent);
    if (parent < 0) {
        for (int i = 0; i < MAX_PATH_LEN; i++) {
            free(argv[i]);
        }
        free(argv);
        return -ENOENT;
    } else if (parent < 2) {
        parent = 2;
    }

    // read in parent dir 
    struct fs_dirent *dirent = malloc(sizeof(struct fs_dirent) * 128);
    block_read(dirent, (inodes+parent)->ptrs[0], 1);

    printf("dir: %s\n", dirent->name);
    printf("parent: %d\n", parent);

    // do the delete
    for (int i = 0; i < 128; i++) {
        if (dirent[i].valid && dirent[i].inode == inum_source) {
            dirent[i].valid = 0;
            memset(dirent[i].name, 0, MAX_NAME_LEN);
            break;
        }
    }

    // write to mem
    block_write(dirent, (inodes+parent)->ptrs[0], 1);
    free(dirent);

    // free data blocks
    for (int i = 0; i < 6; i++) {
        int block = inode->ptrs[i];
        if (block != 0) {
            bit_clear(block_bitmap, block);
        }
    }

    // clear inode
    memset(inode, 0, sizeof(struct fs_inode));
    bit_clear(block_bitmap, inum_source);

    // write  updates
    block_write(inodes, inum_source, 1);
    block_write(block_bitmap, 1, 1);

    //free memory
    for (int i = 0; i < MAX_PATH_LEN; i++) {
        if(argv[i] != NULL) {
            free(argv[i]);
            argv[i] = NULL;
        }
    }
    free(argv);

    return 0; // success
}

/* rmdir - remove a directory
 *  success - return 0
 *  Errors - path resolution, ENOENT, ENOTDIR, ENOTEMPTY
 */
int fs_rmdir(const char *path)
{
        // get inode number
        char *paths = strdup(path);
        int inum_source = get_inum(paths);
        if (inum_source < 0) {
            return -ENOENT;
        }
    
        // get inode
        struct fs_inode *inode = inodes+inum_source;
    
        if (!((inode->mode & __S_IFMT) == __S_IFDIR)) {
            // if its not a dir
            return -ENOTDIR;
        }

        int empty = dir_empty(inode);

        if(!empty) {
            return -ENOTEMPTY;
        }
    
        // find parent dir
        char *pathd = strdup(path);
        char **argv = malloc(MAX_PATH_LEN * sizeof(char *));
        for (int i = 0; i < MAX_PATH_LEN; i++) {
            argv[i] = malloc(MAX_NAME_LEN);
        }
    
        int pathc = parse2(pathd, argv);
        free(pathd);
    
        int parent;
        if (pathc == 1) {
            parent = 2; // parent dir is the root
        } else {
            parent = translate(pathc - 1, argv); // get the parent dir
        }
    
        printf("parent: %d\n", parent);
        if (parent < 0) {
            for (int i = 0; i < MAX_PATH_LEN; i++) {
                free(argv[i]);
            }
            free(argv);
            return -ENOENT;
        } else if (parent < 2) {
            parent = 2;
        }
    
        // read in parent dir 
        struct fs_dirent *dirent = malloc(sizeof(struct fs_dirent) * 128);
        block_read(dirent, (inodes+parent)->ptrs[0], 1);
    
        printf("dir: %s\n", dirent->name);
        printf("parent: %d\n", parent);
    
        // do the delete
        for (int i = 0; i < 128; i++) {
            if (dirent[i].valid && dirent[i].inode == inum_source) {
                dirent[i].valid = 0;
                memset(dirent[i].name, 0, MAX_NAME_LEN);
                break;
            }
        }
    
        // write to mem
        block_write(dirent, (inodes+parent)->ptrs[0], 1);
        free(dirent);
    
        // free data blocks
        for (int i = 0; i < 6; i++) {
            int block = inode->ptrs[i];
            if (block != 0) {
                bit_clear(block_bitmap, block);
            }
        }
    
        // clear inode
        memset(inode, 0, sizeof(struct fs_inode));
    
        // write  updates
        block_write(inodes, 1, 1);
    
        //free memory
        for (int i = 0; i < MAX_PATH_LEN; i++) {
            if(argv[i] != NULL) {
                free(argv[i]);
                argv[i] = NULL;
            }
        }
        free(argv);
    
        return 0; // success
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
    // if src does not exist
    char *paths= strdup(src_path);
    int inum_source = get_inum(paths);
    if(inum_source < 0) {
        return -ENOENT;
    }

    // if dest does not exist
    char *pathd = strdup(dst_path);
    int inum_dst = get_inum(pathd);
    if(inum_dst > 0) {
        return -EEXIST;
    }

    char** argv_source = (char**) malloc(MAX_PATH_LEN * (MAX_NAME_LEN * sizeof(char)));
    char** argv_dest = (char**) malloc(MAX_PATH_LEN * (MAX_NAME_LEN * sizeof(char)));

    int pathc_source = parse(strdup(src_path), argv_source);
    int pathc_dest = parse(strdup(dst_path), argv_dest);

    // if src path does not equal dest path
    if(pathc_source != pathc_dest) {
        return -EINVAL;
    }

    for(int i = 0; i < pathc_source-1; i++) {
        if(strcmp(argv_source[i], argv_dest[i]) != 0) {
            return -EINVAL;
        }
    }

    // do renaming
    int parent_dir_inum = translate(pathc_source-1, argv_source);
    struct fs_dirent *dir = malloc(4096);
    block_read(dir, *((inodes+parent_dir_inum)->ptrs), 1);
    for(int i = 0; i < 128; i++) {
        if(strcmp((dir+i)->name, *(argv_source+pathc_source-1)) == 0) {
            strcpy((dir+i)->name, *(argv_dest+pathc_dest-1));
            block_write(dir, *((inodes+parent_dir_inum)->ptrs), 1);
            break;
        }
    }
    free(dir);
    free(argv_source);
    free(argv_dest);
    // TODO: change last update date
    return 0; //success
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
    // if path does not exist
    printf("chmod mode: %d\n", mode);
    char *paths= strdup(path);
    int inum_source = get_inum(paths);
    if(inum_source < 0) {
        return -ENOENT;
    }

    //chnage file permission
    struct fs_inode *inode = inodes+inum_source;
    inode->mode = mode;
    block_write(inode, inum_source, 1);

    return 0; // success
}

int fs_utime(const char *path, struct utimbuf *ut)
{
    int inum = get_inum(strdup(path));
    if(inum < 0) {
        return -EOPNOTSUPP;
    }
    struct fs_inode *inode = inodes+inum;

    inode->ctime = time(NULL);
    inode->mtime = ut->modtime;

    block_write(inode, inum, 1);

    return 0;
}

/* truncate - truncate file to exactly 'len' bytes
 * success - return 0
 * Errors - path resolution, ENOENT, EISDIR, EINVAL
 *    return EINVAL if len > 0.
 */
int fs_truncate(const char *path, off_t len)
{
    if (len != 0) {
        return -EINVAL;
    }

    // get inode number
    char *paths = strdup(path);
    int inum_source = get_inum(paths);
    if (inum_source < 0) {
        return -ENOENT;
    }

    // get inode
    struct fs_inode *inode = inodes+inum_source;

    if (inode == NULL) {
        return -ENOENT; // inode does not exist
    }

    if ((inode->mode & __S_IFMT) == __S_IFDIR) {
        // if its a dir
        return -EISDIR;
    }

    // free blocks
    for (int i = 0; i < 6; i++) {
        if (inode->ptrs[i] == 0) {
            continue;
        }
        inode->ptrs[i] = 0;
    }

    // set metadata
    inode->size = 0;
    inode->mtime = time(NULL);

    block_write(inode, inum_source, 1);

    return 0; // Success
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
        block_read(temp_buf+(i*4096), inode->ptrs[i], 1);
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
    printf("Write length: %ld\n", len);
    int inum = get_inum(strdup(path));
    if(inum < 0) return inum;

    struct fs_inode *inode = inodes+inum;

    if(S_ISDIR(inode->mode)) {
        return -EISDIR;
    }

    if(offset > inode->size) return -EINVAL;

    int numBlocks = DIV_ROUND_UP(len+offset, FS_BLOCK_SIZE);
    char *buffer = malloc(FS_BLOCK_SIZE);
    char *loop = strdup(buf);
    char *loop_inv = loop;
    int plen = len;
    

    int blockptr;
    int sizeComp = 0;
    for(int i = 0; i < 1019; i++) {
        if (inode->ptrs[i] == 0 && i >= numBlocks) {
            break;
        } else if (inode->ptrs[i] == 0 && i < numBlocks) {
            blockptr = find_free();
            bit_set(block_bitmap, blockptr);
            inode->ptrs[i] = blockptr;
        } else if (inode->ptrs[i] != 0 && i < numBlocks) {
            blockptr = inode->ptrs[i];
        } else if (inode->ptrs[i] != 0 && i >= numBlocks) {
            bit_clear(block_bitmap, blockptr);
            inodes->ptrs[i] = 0;
            continue;
        }
        int rStart = i*4096, rEnd = (i+1)*4096;
        int inRange = rEnd >= offset;
        int lpos = offset - rStart;
        int rpos = rEnd - lpos;
        if( inRange && lpos >= 0 
            && rpos < FS_BLOCK_SIZE && plen >= rpos) {
            block_read(buffer, blockptr, 1);
            memcpy(buffer+lpos, loop, rpos);
            block_write(buffer, blockptr, 1);
            loop += rpos;
            plen -= rpos;
        } else if(inRange && lpos >= 0 && plen < rpos) {
            block_read(buffer, blockptr, 1);
            memset(buffer+lpos, 0, FS_BLOCK_SIZE-lpos);
            memcpy(buffer+lpos, loop, plen);
            block_write(buffer, blockptr, 1);
            loop += plen;
            sizeComp += lpos + plen;
            plen -= plen;
            continue;
        } else if(inRange) {
            block_read(buffer, blockptr, 1);
            memcpy(buffer, loop, rpos);
            block_write(buffer, blockptr, 1);
            loop += rpos;
            plen -= rpos;
        }
        sizeComp+=4096;
    }
    inode->size = sizeComp;
    inode->mtime = time(NULL);
    block_write(inode, inum, 1);
    block_write(block_bitmap, 1, 1);
    free(loop_inv);
    return len - plen;
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

    st->f_bsize = FS_BLOCK_SIZE;
    st->f_blocks = super_block->disk_size - 2; 
    st->f_bfree =  st->f_blocks - get_used_blocks();
    st->f_bavail = st->f_bfree;
    st->f_namemax = MAX_NAME_LEN;

    st->f_files = 0;
    st->f_ffree = 0;
    st->f_favail = 0;
    st->f_flag = 0;
    st->f_fsid = 0;
    for(int i = 0; i < 6; i++) {
        st->__f_spare[i] = 0;
    }

    return 0;
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

