// disk.h: Disk emulator

#pragma once

#include <stdlib.h>
#include <stdbool.h>

#define BLOCK_SIZE 4096

typedef struct
{
    int FileDescriptor; // File descriptor of disk image
    size_t Blocks;      // Number of blocks in disk image
    size_t Reads;       // Number of reads performed
    size_t Writes;      // Number of writes performed
    size_t Mounts;      // Number of mounts
} Disk;

// Default constructor
Disk *new_disk();

// Destructor
// @param	disk pointer
void free_disk(Disk *disk);

// Open disk image
// @param	disk pointer
// @param	path	    Path to disk image
// @param	nblocks	    Number of blocks in disk image
void disk_open(Disk *disk, const char *path, size_t nblocks);

// Return size of disk (in terms of blocks)
// @param	disk pointer
size_t disk_size(Disk *disk);

// Return whether or not disk is mounted
// @param	disk pointer
bool disk_mounted(Disk *disk);

// Increment mounts
// @param	disk pointer
void disk_mount(Disk *disk);

// Decrement mounts
// @param	disk pointer
void disk_unmount(Disk *disk);

// Check parameters
// @param	disk pointer
// @param	blocknum    Block to operate on
// @param	data	    Buffer to operate on
void disk_sanity_check(Disk *disk, int blocknum, char *data);

// Read block from disk
// @param	disk pointer
// @param	blocknum    Block to read from
// @param	data	    Buffer to read into
void disk_read(Disk *disk, int blocknum, char *data);

// Write block to disk
// @param	disk pointer
// @param	blocknum    Block to write to
// @param	data	    Buffer to write from
void disk_write(Disk *disk, int blocknum, char *data);
