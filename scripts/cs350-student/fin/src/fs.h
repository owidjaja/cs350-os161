// fs.h: File System

#pragma once

#include "disk.h"

#include <sys/types.h>
#include <stdint.h>
#include <stdbool.h>

#define MAGIC_NUMBER 0xf0f03410
#define INODES_PER_BLOCK 128
#define POINTERS_PER_INODE 5
#define POINTERS_PER_BLOCK 1024

typedef struct
{                         // Superblock structure
    uint32_t MagicNumber; // File system magic number
    uint32_t Blocks;      // Number of blocks in file system
    uint32_t InodeBlocks; // Number of blocks reserved for inodes
    uint32_t Inodes;      // Number of inodes in file system
} SuperBlock;

typedef struct
{
    uint32_t Valid;                      // Whether or not inode is valid
    uint32_t Size;                       // Size of file
    uint32_t Direct[POINTERS_PER_INODE]; // Direct pointers
    uint32_t Indirect;                   // Indirect pointer
} Inode;

typedef union
{
    SuperBlock Super;                      // Superblock
    Inode Inodes[INODES_PER_BLOCK];        // Inode block
    uint32_t Pointers[POINTERS_PER_BLOCK]; // Pointer block
    char Data[BLOCK_SIZE];                 // Data block
} Block;

typedef struct
{
    Disk *disk;
    
    // FIXME: Internal member variables
} FileSystem;

void fs_debug(Disk *disk);
bool fs_format(Disk *disk);

FileSystem *new_fs();
void free_fs(FileSystem *fs);

bool fs_mount(FileSystem *fs, Disk *disk);

ssize_t fs_create(FileSystem *fs);
bool fs_remove(FileSystem *fs, size_t inumber);
ssize_t fs_stat(FileSystem *fs, size_t inumber);

ssize_t fs_read(FileSystem *fs, size_t inumber, char *data, size_t length, size_t offset);
ssize_t fs_write(FileSystem *fs, size_t inumber, char *data, size_t length, size_t offset);
