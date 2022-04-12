#include "disk.h"
#include "fs.h"

#include <stdio.h>
#include <string.h>
#include <math.h>

#define min(a,b) (((a) < (b)) ? (a) : (b))

#ifdef DEBUG
#define DEBUG_PRINT(fmt, args...)    fprintf(stderr, fmt, ## args)
#else
#define DEBUG_PRINT(fmt, args...)    /* Don't do anything in release builds */
#endif

// Debug file system -----------------------------------------------------------

void fs_debug(Disk *disk)
{
    if (disk == 0)
        return;

    Block block;

    // Read Superblock
    disk_read(disk, 0, block.Data);

    uint32_t magic_num = block.Super.MagicNumber;
    uint32_t num_blocks = block.Super.Blocks;
    uint32_t num_inodeBlocks = block.Super.InodeBlocks;
    uint32_t num_inodes = block.Super.Inodes;

    if (magic_num != MAGIC_NUMBER)
    {
        printf("Magic number is invalid: %c\n", magic_num);
        return;
    }

    printf("SuperBlock:\n");
    printf("    magic number is valid\n");
    printf("    %u blocks\n", num_blocks);
    printf("    %u inode blocks\n", num_inodeBlocks);
    printf("    %u inodes\n", num_inodes);

    uint32_t expected_num_inodeBlocks = round((float)num_blocks / 10);

    if (expected_num_inodeBlocks != num_inodeBlocks)
    {
        printf("SuperBlock declairs %u InodeBlocks but expect %u InodeBlocks!\n", \
                                    num_inodeBlocks, expected_num_inodeBlocks);
    }

    uint32_t expect_num_inodes = num_inodeBlocks * INODES_PER_BLOCK;
    if (expect_num_inodes != num_inodes)
    {
        printf("SuperBlock declairs %u Inodes but expect %u Inodes!\n", \
                                    num_inodes, expect_num_inodes);
    }

    // FIXME: Read Inode blocks
    for (unsigned int i=0; i<num_inodeBlocks; i++){
        // printf("\ninode block %d\n", i);
        disk_read(disk, i+1, block.Data);
        
        for (unsigned int j=0; j<INODES_PER_BLOCK; j++){
            Inode this_inode = block.Inodes[j];
            if (this_inode.Valid == 0){
                continue;
            }

            printf("Inode %d:\n", i*INODES_PER_BLOCK+j);
            printf("    size: %d bytes\n", this_inode.Size);
            
            printf("    direct blocks:");
            for (unsigned int k=0; k<POINTERS_PER_INODE; k++){
                if (this_inode.Direct[k]){
                    printf(" %d", this_inode.Direct[k]);
                }
            }
            printf("\n");

            uint32_t this_indirect = this_inode.Indirect;
            if (this_indirect){
                Block indirect_block;
                printf("    indirect block: %d\n", this_indirect);
                printf("    indirect data blocks:");
                disk_read(disk, this_indirect, indirect_block.Data);
                for (unsigned int k=0; k<POINTERS_PER_BLOCK; k++){
                    if (indirect_block.Pointers[k]){
                        printf(" %d", indirect_block.Pointers[k]);
                    }
                }
                printf("\n");
            }
        }
    }
}

// Format file system ----------------------------------------------------------

bool fs_format(Disk *disk)
{
    // Write superblock
    Block new_super;
    new_super.Super.MagicNumber = MAGIC_NUMBER;
    new_super.Super.Blocks      = disk_size(disk);
    new_super.Super.InodeBlocks = round((float)new_super.Super.Blocks / 10);
    new_super.Super.Inodes      = new_super.Super.InodeBlocks * INODES_PER_BLOCK;
    
    disk_write(disk, 0, new_super.Data);

    // Clear all other blocks
    // for (unsigned int i=1; i<new_super.Super.Blocks; i++){
    //     disk_
    // }
    return true;
    return false;
}

// FileSystem constructor 
FileSystem *new_fs()
{
    FileSystem *fs = malloc(sizeof(FileSystem));
    return fs;
}

// FileSystem destructor 
void free_fs(FileSystem *fs)
{
    // FIXME: free resources and allocated memory in FileSystem

    free(fs);
}

// Mount file system -----------------------------------------------------------

bool fs_mount(FileSystem *fs, Disk *disk)
{
    // Read superblock

    // Set device and mount

    // Copy metadata

    // Allocate free block bitmap

    return false;
}

// Create inode ----------------------------------------------------------------

ssize_t fs_create(FileSystem *fs)
{
    // Locate free inode in inode table

    // Record inode if found

    return -1;
}

// Optional: the following two helper functions may be useful. 

// bool find_inode(FileSystem *fs, size_t inumber, Inode *inode)
// {
//     return true;
// }

// bool store_inode(FileSystem *fs, size_t inumber, Inode *inode)
// {
//     return true;
// }

// Remove inode ----------------------------------------------------------------

bool fs_remove(FileSystem *fs, size_t inumber)
{
    // Load inode information

    // Free direct blocks

    // Free indirect blocks

    // Clear inode in inode table

    return false;
}

// Inode stat ------------------------------------------------------------------

ssize_t fs_stat(FileSystem *fs, size_t inumber)
{
    // Load inode information
    return 0;
}

// Read from inode -------------------------------------------------------------

ssize_t fs_read(FileSystem *fs, size_t inumber, char *data, size_t length, size_t offset)
{
    // Load inode information

    // Adjust length

    // Read block and copy to data
    
    return 0;
}

// Optional: the following helper function may be useful. 

// ssize_t fs_allocate_block(FileSystem *fs)
// {
//     return -1;
// }

// Write to inode --------------------------------------------------------------

ssize_t fs_write(FileSystem *fs, size_t inumber, char *data, size_t length, size_t offset)
{
    // Load inode
    
    // Write block and copy to data

    return 0;
}
