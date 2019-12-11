#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <stdbool.h>
#include "fs.h"


struct dinode *temp_i; // the address of #inode1
struct superblock *sb; // the address of super block
void *data_ptr; // the first data block address
int *dmap; // record the bit map in file system
int *data_use;  // record the data block in use
int *inode_use; // record the inodes in use
int *dir_inode; // record inode in the directory
void *fs_ptr; // the Base of mmap


void print_err(char *msg) {
    fprintf(stderr, "%s\n", msg);
    exit(1);
}

/**
 * Given a dinode address, calculate the number of data blocks it has, and mark the data_use array
 * @param curr
 * @param fs_ptr
 * @return
 */
int cal_num_block(struct dinode *curr, void *fs_ptr) {
    int num_b = 0;
    for (int i = 0; i < NDIRECT; i++) {
        if (curr->addrs[i] != 0) {
            num_b++;
            data_use[curr->addrs[i]] = 1;
        }
    }
    if (curr->addrs[NDIRECT] == 0) return num_b;
    data_use[curr->addrs[NDIRECT]] = 1;
    int *data_add = (int *) (fs_ptr + curr->addrs[NDIRECT] * BSIZE);
    for (int i = 0; i < NINDIRECT; i++) {
        if (*data_add != 0) {
            num_b++;
            data_use[*data_add] = 1;
        }
        data_add++;
    }
    return num_b;
}


/**
 * Given a inode (DIR), search the first element in its data block, and the int number should equal to the inode number
 * @param inode
 * @param data_b_add
 * @param fs_ptr
 * @return #inode the data block points to
 */
int check_dot(int inode, int data_b_add, void *fs_ptr) {
    short *dot = fs_ptr + (data_b_add * BSIZE);
    int temp = (int)*dot;
    if (inode != temp) {
        print_err("ERROR: current directory mismatch.");
    }
    return *dot;
}

/**
 * For a DIR inode, traverse its address and find the inode number
 */
void count_refer_node(short *ref_inode) {
    // TODO
    int inode_num = 512 / 16;
    for (int i = 0; i < inode_num; i++) {
        int temp = *ref_inode;
        if (temp != 0) {
            dir_inode[temp] = 1;
        }
        ref_inode += 8;
    }
}

void init_map() {
    char *map_ptr = (sb->bmapstart * BSIZE) + fs_ptr;
    int curr_pos = 0;
    for (int i = 0; i < sb->nblocks; i++) {
        unsigned int comp = 1 << curr_pos;
        dmap[i] = comp & (*map_ptr);
        if (curr_pos == 7) {
            curr_pos = 0;
            map_ptr++;
            continue;
        }
        curr_pos += 1;

    }
}

int main(int argc, char *argv[]) {
    if (argc == 1)
        print_err("Usage: error");
    int fsfd;
    fsfd = open(argv[1], O_RDONLY);
    if (fsfd < 0)
        print_err("image not found.");


    struct stat fs_stat;
    fstat(fsfd, &fs_stat);
    fs_ptr = mmap(NULL, fs_stat.st_size, PROT_READ, MAP_PRIVATE, fsfd, 0);

    sb = (struct superblock *) (fs_ptr + BSIZE);
    temp_i = (struct dinode *) (fs_ptr + sb->inodestart * BSIZE + sizeof(struct dinode)); // find the first inode
    int map_num = (sb->nblocks / BPB) + 1;
    data_ptr = (sb->bmapstart + map_num) * BSIZE + fs_ptr;


    dmap = malloc(map_num * BPB * sizeof(int)); // record the bit map in file system
    data_use = malloc(sb->nblocks * sizeof(int)); // record the data block in use
    inode_use = malloc(sb->ninodes * sizeof(int));
    dir_inode = malloc(sb->ninodes * sizeof(int));
    memset(dmap, 0, sizeof(int) * map_num * BPB );
    memset(data_use, 0, sizeof(int) * sb->nblocks );
    memset(inode_use, 0, sizeof(int) * sb->ninodes);
    memset(dir_inode, 0, sizeof(int) * sb->ninodes );


    init_map();

    // Begin Individual Inode Checks
    for (int i = 0; i < sb->ninodes - 1; i++) {
        // Individual Inode Checks 1
        if (temp_i->type != 0) {
            inode_use[i + 1] = 1;
            if (temp_i->type != 1 && temp_i->type != 2 && temp_i->type != 3) {
                print_err("ERROR: bad inode.");
            }
            // Individual Inode Checks 2
            int file_size = temp_i->size;
            int num_of_block = cal_num_block(temp_i, fs_ptr);
            if (num_of_block * BSIZE < file_size) {
                print_err("ERROR: bad size in inode.");
            }
            if (file_size < (num_of_block-1) * BSIZE) {
                print_err("ERROR: bad size in inode.");
            }
        }
        temp_i++;
    }
    // Begin the Directory Checks
    temp_i = (struct dinode *) (fs_ptr + sb->inodestart * BSIZE); // find the first inode
    // Directory Checks 1
    // Directory Checks 2
    for (int i = 0; i < sb->ninodes; i++) {
        if(i == ROOTINO) {
            if(temp_i->type != 1) {
                print_err("ERROR: root directory does not exist.");
            }
            struct dirent* dir = fs_ptr + (BSIZE*temp_i->addrs[0]);
            if(dir->inum != ROOTINO || (dir+1)->inum != ROOTINO){
                print_err("ERROR: root directory does not exist.");
            }
        }
        if (temp_i->type == 1) {
            int res = check_dot(i, temp_i->addrs[0], fs_ptr);
        }
        temp_i++;
    }
   
    // Begin Bitmap Checks
    for (int i = 0; i < sb->nblocks; i++) {
        // Bitmap Checks 1
        if (data_use[i] == 1 && dmap[i] == 0) {
            print_err("ERROR: bitmap marks data free but data block used by inode.");
        }
    }
    // Bit map Checks 2
    int data_begin; //
    data_begin = (data_ptr - fs_ptr) / BSIZE;
    for (int i = data_begin; i < map_num * BPB; i++) {
        if (dmap[i] != 0 && data_use[i] == 0) {
            print_err("ERROR: bitmap marks data block in use but not used.");
        }
    }
  

    // Multi-Structure Checks
    temp_i = (struct dinode *) (fs_ptr + sb->inodestart * BSIZE + sizeof(struct dinode)); // find the first inode
    for (int i = 0; i < sb->ninodes; i++) {
        if (temp_i->type == 1) {
            count_refer_node(temp_i->addrs[0] * BSIZE + fs_ptr);
        }
        temp_i++;
    }

    for (int i = 0; i < sb->ninodes; i++) {
        if (dir_inode[i] == 1 && inode_use[i] == 0)
            print_err("ERROR: inode marked free but referred to in directory.");
        if (inode_use[i] == 1 && dir_inode[i] == 0)
            print_err("ERROR: inode marked in use but not found in a directory.");
    }
//    fprintf("#%d nodes in use and #%d nodes are refered", di_num, ui_num);
    free(data_use);
    free(dmap);
    free(dir_inode);
    free(inode_use);
}
