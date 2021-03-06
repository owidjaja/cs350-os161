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
            Inode *this_inode = &block.Inodes[j];
            if (this_inode->Valid == 0){
                continue;
            }

            printf("Inode %d:\n", i*INODES_PER_BLOCK+j);
            printf("    size: %d bytes\n", this_inode->Size);
            
            printf("    direct blocks:");
            for (unsigned int k=0; k<POINTERS_PER_INODE; k++){
                if (this_inode->Direct[k]){
                    printf(" %d", this_inode->Direct[k]);
                }
            }
            printf("\n");

            uint32_t this_indirect = this_inode->Indirect;
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
    if (disk_mounted(disk)){
        return false;
    }

    // Write superblock
    Block new_super;
    new_super.Super.MagicNumber = MAGIC_NUMBER;
    new_super.Super.Blocks      = disk_size(disk);
    new_super.Super.InodeBlocks = round((float)new_super.Super.Blocks / 10);
    new_super.Super.Inodes      = new_super.Super.InodeBlocks * INODES_PER_BLOCK;
    
    disk_write(disk, 0, new_super.Data);

    // Reset inode table
    for (unsigned int i=0; i<new_super.Super.InodeBlocks; i++){
        // printf("\non inode block %d\n", i);
        Block new_inodeblock;

        for (unsigned int j=0; j<INODES_PER_BLOCK; j++){
            // printf("on inode num %d\n", j);
            Inode* this_inode    = &new_inodeblock.Inodes[j];
            this_inode->Valid    = 0;
            this_inode->Size     = 0;
            this_inode->Indirect = 0;
            for (unsigned int k=0; k<POINTERS_PER_INODE; k++){
                this_inode->Direct[k] = 0;
            }
        }
        
        disk_write(disk, i+1, new_inodeblock.Data);
    }

    // Other blocks deliberately left as is

    return true;
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
    free(fs->block_bitmap);
    free(fs);
}

// Mount file system -----------------------------------------------------------

bool fs_mount(FileSystem *fs, Disk *disk)
{
    if (disk==0 || disk_mounted(disk)){
        return false;
    }

    Block block;
    
    // Read superblock
    disk_read(disk, 0, block.Data);
    uint32_t magic_num = block.Super.MagicNumber;
    if (magic_num != MAGIC_NUMBER){
        // printf("Magic number is invalid: %c\n", magic_num);
        return false;
    }
    if (block.Super.InodeBlocks != round((float)block.Super.Blocks / 10)){
        return false;
    }
    if (block.Super.Inodes != block.Super.InodeBlocks*INODES_PER_BLOCK){
        return false;
    }

    // Set device and mount
    fs->disk = disk;
    disk_mount(disk);

    // Copy metadata
    fs->super = block.Super;
    // printf("super mounted with numblocks: %d\n", fs->super.Blocks);

    // Allocate free block bitmap
    fs->block_bitmap = malloc(fs->super.Blocks * sizeof(int));
    
    for (unsigned int i=0; i<block.Super.InodeBlocks; i++){
        disk_read(disk, i+1, block.Data);
        
        for (unsigned int j=0; j<INODES_PER_BLOCK; j++){
            Inode this_inode = block.Inodes[j];
            if (this_inode.Valid == 0){
                continue;
            }
            for (unsigned int k=0; k<POINTERS_PER_INODE; k++){
                if (this_inode.Direct[k]){
                    fs->block_bitmap[this_inode.Direct[k]] = 1;
                }
            }

            uint32_t this_indirect = this_inode.Indirect;
            if (this_indirect){
                Block indirect_block;
                disk_read(disk, this_indirect, indirect_block.Data);
                for (unsigned int k=0; k<POINTERS_PER_BLOCK; k++){
                    if (indirect_block.Pointers[k]){
                        fs->block_bitmap[indirect_block.Pointers[k]] = 1;
                    }
                }
            }
        }
    }

    return true;
}

// Create inode ----------------------------------------------------------------

ssize_t fs_create(FileSystem *fs)
{
    Block block;

    // Locate free inode in inode table
    for (unsigned int i=0; i<fs->super.InodeBlocks; i++){
        disk_read(fs->disk, i+1, block.Data);
        
        for (unsigned int j=0; j<INODES_PER_BLOCK; j++){
            Inode* this_inode = &block.Inodes[j];
            if (this_inode->Valid == 0){
                this_inode->Size     = 0;
                this_inode->Indirect = 0;
                for (unsigned int k=0; k<POINTERS_PER_INODE; k++){
                    this_inode->Direct[k] = 0;
                }
                this_inode->Valid    = 1;

                disk_write(fs->disk, i+1, block.Data);
                return (i*INODES_PER_BLOCK + j);
            }
        }
    }

    // Record inode if found
    // me: failed to find invalid inode
    // printf("failed to find empty (invalid) inode\n");

    return -1;
}

// Optional: the following two helper functions may be useful. 

bool find_inode(FileSystem *fs, size_t inumber, Inode *inode)
{
    // make a shallow copy of Inode in disk

    int blocknum = inumber/INODES_PER_BLOCK + 1;
    if ((inumber < 0) | (inumber > fs->super.Inodes) | (blocknum > (fs->super.InodeBlocks+1))){
        // printf("err inumber\n");
        return false;
    }

    Block block;
    disk_read(fs->disk, blocknum, block.Data);
    *inode = block.Inodes[inumber - (blocknum-1)*128];

    if (inode->Valid == 0){
        // printf("in find_inode invalid\n");
        return false;
    }
    
    return true;
}

bool store_inode(FileSystem *fs, size_t inumber, Inode *inode)
{
    // perform shallow copy on Inode structure, 
    // which is ok since Inode struct does not have pointers, but defined array size
    // https://stackoverflow.com/questions/2241699/

    // inefficient read, but tricky workaround

    int blocknum = inumber/INODES_PER_BLOCK + 1;
    if ((inumber > fs->super.Inodes) | (blocknum > (fs->super.InodeBlocks+1))){
        // printf("err inumber\n");
        return false;
    }

    Block block;
    disk_read(fs->disk, blocknum, block.Data);

    block.Inodes[inumber - (blocknum-1)*128] = *inode;
    disk_write(fs->disk, blocknum, block.Data);

    return true;
}

// Remove inode ----------------------------------------------------------------

bool fs_remove(FileSystem *fs, size_t inumber)
{
    Inode found_inode;
    if (find_inode(fs, inumber, &found_inode) == false){
        return false;
    }

    // Free direct blocks
    for (unsigned int i=0; i<POINTERS_PER_INODE; i++){
        uint32_t *this_ptr = &found_inode.Direct[i];
        if (*this_ptr != 0){
            *this_ptr = 0;
            fs->block_bitmap[*this_ptr] = 0;
        }
    }

    // Free indirect blocks, me: only one indirect block
    uint32_t this_indirect = found_inode.Indirect;
    if (this_indirect){
        Block indirect_block;
        disk_read(fs->disk, this_indirect, indirect_block.Data);
        for (unsigned int k=0; k<POINTERS_PER_BLOCK; k++){
            uint32_t *this_ptr = &indirect_block.Pointers[k];
            if (*this_ptr != 0){
                // make ptr point to 0 just in case
                // actually only need to update in free block bitmap
                *this_ptr = 0;
                fs->block_bitmap[*this_ptr] = 0;
            }
        }
        disk_write(fs->disk, this_indirect, indirect_block.Data);
    }

    // Clear inode in inode table
    found_inode.Valid = 0;
    store_inode(fs, inumber, &found_inode);

    return true;
}

// Inode stat ------------------------------------------------------------------

ssize_t fs_stat(FileSystem *fs, size_t inumber)
{
    // Load inode information
    Inode found_inode;
    if (find_inode(fs, inumber, &found_inode) == false){
        return -1;
    }

    return found_inode.Size;
}

// Read from inode -------------------------------------------------------------

ssize_t fs_read(FileSystem *fs, size_t inumber, char *data, size_t length, size_t offset)
{
    // Load inode information
    Inode found_inode;
    if (!find_inode(fs, inumber, &found_inode)){
        // printf("fail at find_inode\n");
        return -1;
    }

    // signed and unsigned comparison shouldk be ok since we have sanity check above
    u_int32_t logical_size = found_inode.Size;
    u_int16_t remaining_sz = logical_size - offset;
    
    // debug
    // printf("\n");
    // printf("length:       %lu\n", length);
    // printf("offset:       %lu\n", offset);
    // printf("logical size: %u \n", logical_size);
    // printf("remaining_sz: %u \n", remaining_sz);


    // fail conditions: no more data to read, or exceeded buffer length (8196)
    if ((remaining_sz <= 0) | (offset >= length)){
        // printf("fail at remaining_sz <= 0\n");
        return -1;
    }

    // Read block and copy to data
    char buffer[BLOCK_SIZE];
    size_t numbytes_read = 0;
    int ptrnum = offset / BLOCK_SIZE;
    
    while (numbytes_read < length){
        if (remaining_sz <= 0){
            break;
        }

        // data reads
        if (ptrnum < POINTERS_PER_INODE){
            // read from direct
            // printf("reading at direct block: %u\n", found_inode.Direct[ptrnum]);
            disk_read(fs->disk, found_inode.Direct[ptrnum], buffer);
            strncat(data, buffer, BLOCK_SIZE);
            ptrnum++;
        }
        else{
            // read from indirect
            // printf("reading at indirect block:\n");
            break;
        }

        // update counts
        if (remaining_sz < BLOCK_SIZE){
            numbytes_read += found_inode.Size;
        }
        else{
            numbytes_read += BLOCK_SIZE;
        }
        offset += BLOCK_SIZE;
        remaining_sz -= numbytes_read;
    }

    // printf("returning numbytes_read: %lu\n", numbytes_read);
    return numbytes_read;
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
