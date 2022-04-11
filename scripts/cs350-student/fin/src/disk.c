#include "disk.h"

#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

// Default constructor
Disk *new_disk()
{
    Disk *disk = malloc(sizeof(Disk));
    disk->FileDescriptor = 0;
    disk->Blocks = 0;
    disk->Reads = 0;
    disk->Writes = 0;
    disk->Mounts = 0;
    return disk;
}

// Destructor
void free_disk(Disk *disk)
{
    if (disk->FileDescriptor > 0)
    {
        printf("%lu disk block reads\n", disk->Reads);
        printf("%lu disk block writes\n", disk->Writes);
        close(disk->FileDescriptor);
        disk->FileDescriptor = 0;
    }
    free(disk);
}

// Open disk image
// @param	path	    Path to disk image
// @param	nblocks	    Number of blocks in disk image
void disk_open(Disk *disk, const char *path, size_t nblocks)
{
    disk->FileDescriptor = open(path, O_RDWR | O_CREAT, 0600);
    if (disk->FileDescriptor < 0)
    {
        char what[BUFSIZ];
        snprintf(what, BUFSIZ, "Unable to open %s: %s", path, strerror(errno));
        // throw std::runtime_error(what);
        exit(1);
    }

    if (ftruncate(disk->FileDescriptor, nblocks * BLOCK_SIZE) < 0)
    {
        char what[BUFSIZ];
        snprintf(what, BUFSIZ, "Unable to open %s: %s", path, strerror(errno));
        // throw std::runtime_error(what);
        exit(1);
    }

    disk->Blocks = nblocks;
    disk->Reads = 0;
    disk->Writes = 0;
}

// Return size of disk (in terms of blocks)
size_t disk_size(Disk *disk)
{
    return disk->Blocks;
}

// Return whether or not disk is mounted
bool disk_mounted(Disk *disk)
{
    return disk->Mounts > 0;
}

// Increment mounts
void disk_mount(Disk *disk)
{
    disk->Mounts++;
}

// Decrement mounts
void disk_unmount(Disk *disk)
{
    if (disk->Mounts > 0)
        disk->Mounts--;
}

// Check parameters
// @param	blocknum    Block to operate on
// @param	data	    Buffer to operate on
void disk_sanity_check(Disk *disk, int blocknum, char *data)
{
    char what[BUFSIZ];

    if (blocknum < 0) {
    	snprintf(what, BUFSIZ, "blocknum (%d) is negative!", blocknum);
    	// throw std::invalid_argument(what);
        exit(1);
    }

    if (blocknum >= (int)disk->Blocks) {
    	snprintf(what, BUFSIZ, "blocknum (%d) is too big!", blocknum);
    	// throw std::invalid_argument(what);
        exit(1);
    }

    if (data == NULL) {
    	snprintf(what, BUFSIZ, "null data pointer!");
    	// throw std::invalid_argument(what);
        exit(1);
    }
}

// Read block from disk
// @param	blocknum    Block to read from
// @param	data	    Buffer to read into
void disk_read(Disk *disk, int blocknum, char *data)
{
    disk_sanity_check(disk, blocknum, data);

    if (lseek(disk->FileDescriptor, blocknum*BLOCK_SIZE, SEEK_SET) < 0) {
    	char what[BUFSIZ];
    	snprintf(what, BUFSIZ, "Unable to lseek %d: %s", blocknum, strerror(errno));
    	// throw std::runtime_error(what);
        exit(1);
    }

    if (read(disk->FileDescriptor, data, BLOCK_SIZE) != BLOCK_SIZE) {
    	char what[BUFSIZ];
    	snprintf(what, BUFSIZ, "Unable to read %d: %s", blocknum, strerror(errno));
    	// throw std::runtime_error(what);
        exit(1);
    }

    disk->Reads++;
}

// Write block to disk
// @param	blocknum    Block to write to
// @param	data	    Buffer to write from
void disk_write(Disk *disk, int blocknum, char *data)
{
    disk_sanity_check(disk, blocknum, data);

    if (lseek(disk->FileDescriptor, blocknum*BLOCK_SIZE, SEEK_SET) < 0) {
    	char what[BUFSIZ];
    	snprintf(what, BUFSIZ, "Unable to lseek %d: %s", blocknum, strerror(errno));
    	// throw std::runtime_error(what);
        exit(1);
    }

    if (write(disk->FileDescriptor, data, BLOCK_SIZE) != BLOCK_SIZE) {
    	char what[BUFSIZ];
    	snprintf(what, BUFSIZ, "Unable to write %d: %s", blocknum, strerror(errno));
    	// throw std::runtime_error(what);
        exit(1);
    }

    disk->Writes++;
}

