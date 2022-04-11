/* main.c
 * ----------------------------------------------------------
 *  CS350
 *  Final Programming Assignment
 *
 * ----------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "fs.h"
#include "disk.h"

#define W_BLK		0
#define W_NONBLK	1

#define DISK_BLK_SIZE	4096

#define ERR_EMPTY_CMD	-999

const char MSG_ERROR[30] = "An error has occurred\n";
const int FUNC_COUNT = 12;
const char * FUNC_MAP[] = {
    "format",
    "mount",
    "debug",
    "create",
    "remove <inode>",
    "cat <inode>",
    "stat <inode>",
    "copyin <file> <inode>",
    "copyout <inode> <file>",
    "help",
    "quit",
    "exit"
};

struct job {
	int argc;
	char *argv[256];
};

struct fs {
	Disk *disk;
	FileSystem *fs;
};

int parse_line(char *line, int len, struct job *job);

int func_format(struct fs *f);
int func_mount(struct fs *f);
int func_debug(struct fs *f);
int func_create(struct fs *f);
int func_remove(struct fs *f, ssize_t inode);
int func_cat(struct fs *f, ssize_t inode);
int func_stat(struct fs *f, ssize_t inode);
int func_copyin(struct fs *f, char * file, ssize_t inode);
bool func_copyout(struct fs *f, ssize_t inode, char * file);
int func_exit(struct fs *f, struct job *job);
int func_help();
int clean_up(struct fs *f, struct job *job);
int internal_func(struct fs *f, struct job *job);

int 
parse_line(char *line, int len, struct job *job)
{
	int rt = 0, argc = 0;
	int p_len;
	char *arg;

	p_len = 0;
	arg = (char *)calloc(256, sizeof(char));
	for (int i=0;i<len;i++) {
		if (line[i] == ' ' || line[i] == '\n' || i == len-1) {
			if (p_len == 0) {
				continue;
			}
		
			// insert the curr arg to the proc arg list
			arg[p_len] = '\0';
			job->argv[job->argc++] = arg;

			// reset curr arg
			p_len = 0;
			arg = (char *)calloc(256, sizeof(char));
			argc++;
			continue;
		}

		// append curr char to the end of curr arg
		arg[p_len++] = line[i];
	}

	if (job->argc < 1) {
		rt = ERR_EMPTY_CMD;
		goto done;
	}

	// set the last arg to NULL
	job->argv[job->argc] = NULL;
done:
	free(line);
	return (rt);
}

int 
func_format(struct fs *f)
{
	int rt;

	rt = fs_format(f->disk);
	if (!rt)
		fprintf(stdout, "format failed!\n");
	else
		fprintf(stdout, "disk formatted.\n");
	
	fflush(stdout);
	return (0);
}

int
func_mount(struct fs *f)
{
	int rt;

	rt = fs_mount(f->fs, f->disk);
	if (!rt) 
		fprintf(stdout, "mount failed!\n");
	else
		fprintf(stdout, "disk mounted.\n");

	fflush(stdout);
	return (0);
}

int
func_debug(struct fs *f)
{
	fs_debug(f->disk);
	return (0);
}

int
func_create(struct fs *f)
{
	ssize_t inode = fs_create(f->fs);
	if (inode >= 0)
		fprintf(stdout, "created inode %ld.\n", inode);
	else
		fprintf(stdout, "create failed!\n");
		
	fflush(stdout);
	return (0);
}

int
func_remove(struct fs *f, ssize_t inode)
{
	int rt;

	rt = fs_remove(f->fs, inode);
	if (!rt) 
		fprintf(stdout, "remove failed!\n");
	else
		fprintf(stdout, "removed inode %ld.\n", inode);

	fflush(stdout);
	return (0);
}

int
func_cat(struct fs *f, ssize_t inode)
{
	int rt;

	rt = func_copyout(f, inode, "/dev/stdout");
	if (rt)
		printf("cat failed!\n");

	fflush(stdout);
	return (0);
}

int
func_stat(struct fs *f, ssize_t inode)
{
	ssize_t bytes = fs_stat(f->fs, inode);
	if (bytes >= 0)
		fprintf(stdout, "inode %ld has size %ld bytes.\n", inode, bytes);
	else
		fprintf(stdout, "stat failed!\n");

	fflush(stdout);
	return (0);
}

int
func_copyin(struct fs *f, char * file, ssize_t inode)
{
	FILE *fp;
	ssize_t sz = 0, wr = 0, total = 0;
	char buf[BUFSIZ] = {0};
	
	fp = fopen(file, "r");
	if (fp == NULL) {
		fprintf(stderr, "Unable to open %s.\n", file);
		return (1);
	}

	while (true) {
		memset(buf, 0, BUFSIZ);
		sz = fread(buf, 1, sizeof(buf), fp);
		if (sz <= 0) {
			break;
		}
		total += sz;
		wr += fs_write(f->fs, inode, buf, sz, wr);
	}

	fprintf(stdout, "%ld bytes copied\n", total);
	fclose(fp);

	return (0);
}

bool
func_copyout(struct fs *f, ssize_t inode, char * file)
{
	FILE *fp;
	ssize_t sz = 0, total = 0;
	char buf[BUFSIZ] = {0};
	char *read_out = NULL;

	fp = fopen(file, "w");
	if (fp == NULL) {
		fprintf(stderr, "Unable to open %s.\n", file);
		return (1);
	}

	while (true) {
		memset(buf, 0, BUFSIZ);
		sz = fs_read(f->fs, inode, buf, sizeof(buf), total);
		if (sz <= 0) {
			break;
		}

		read_out = (char *)realloc(read_out, total + BUFSIZ + 1);
		memset(read_out + total, 0, BUFSIZ + 1);
		memcpy(read_out + total, buf, sz);
		total += sz;
	}

	fprintf(stdout, "%ld bytes copied\n", total);
	fflush(stdout);
	
	if (total) {
		read_out[total] = '\0';
		fprintf(fp, "%s", read_out);
		fflush(fp);
		free(read_out);
	}

	fclose(fp);
	return (0);
}

int
func_exit(struct fs *f, struct job *job)
{
	clean_up(f, job);
	exit(0);
	return (0);
}

int 
func_help()
{
	for (int i=0;i<FUNC_COUNT;i++) {
		write(2, FUNC_MAP[i], strlen(FUNC_MAP[i]));
		//if (i < FUNC_COUNT - 1)
		write(2, "\n", 1);
	} 
	return (0);
}

int 
clean_up(struct fs *f, struct job *job)
{
	for (int i=0;i<job->argc;i++)
		free(job->argv[i]);
	job->argc = 0;

	if (f) {
		if (f->fs)
			free_fs(f->fs);
		if (f->disk)
			free_disk(f->disk);
	}
	f->fs = NULL;
	f->disk = NULL;

	return (0);
}

int 
internal_func(struct fs *f, struct job *job)
{
	int rt = -1;

	if (strcmp(job->argv[0], "format") == 0) {
		rt = func_format(f);
	} else if (strcmp(job->argv[0], "mount") == 0) {
		rt = func_mount(f);
	} else if (strcmp(job->argv[0], "debug") == 0) {
		rt = func_debug(f);
	} else if (strcmp(job->argv[0], "create") == 0) {
		rt = func_create(f);
	} else if (strcmp(job->argv[0], "remove") == 0) {
		if (job->argc == 2) {
			rt = func_remove(f, atoi(job->argv[1]));
		}
	} else if (strcmp(job->argv[0], "cat") == 0) {
		if (job->argc == 2) {
			rt = func_cat(f, atoi(job->argv[1]));
		}
	} else if (strcmp(job->argv[0], "stat") == 0) {
		if (job->argc == 2) {
			rt = func_stat(f, atoi(job->argv[1]));
		}
	} else if (strcmp(job->argv[0], "copyin") == 0) {
		if (job->argc == 3) {
			rt = func_copyin(f, job->argv[1], atoi(job->argv[2]));
		}
	} else if (strcmp(job->argv[0], "copyout") == 0) {
		if (job->argc == 3) {
			rt = func_copyout(f, atoi(job->argv[1]), job->argv[2]);
		}
	} else if (strcmp(job->argv[0], "help") == 0) {
		rt = func_help();
	} else if (strcmp(job->argv[0], "quit") == 0) {
		rt = func_exit(f, job);
	} else if (strcmp(job->argv[0], "exit") == 0) {
		rt = func_exit(f, job);
	}

	return (rt);
}

int 
main(int argc, char ** argv)
{
	char *line = NULL;
	size_t sz = 0;
	struct stat f_stat;

	char * disk_fn;
	int disk_blk;
	struct fs f;
	FILE *fp;
	
	int good_input, error, len;

	struct job job;

	setbuf(stdout, NULL);

	f.fs = NULL;
	f.disk = NULL;

	if (argc == 3) {
		disk_fn = argv[1];
		disk_blk = atoi(argv[2]);
	} else {
		goto error_exit;
	}

	f.disk = new_disk();

	/* Load disk */
	/* Check existance, if not create a new one */
	if (stat(disk_fn, &f_stat) != 0) {
		fp = fopen(disk_fn, "w");
		for (int i=0;i<DISK_BLK_SIZE*disk_blk;i++) 
			fprintf(fp, "%d", 0);
		fclose(fp);
	}
	disk_open(f.disk, disk_fn, disk_blk);
	assert(f.disk != NULL);

	f.fs = new_fs();

	/* Interactive cmds */
	for (;;) {
		write(2, "sfs> ", 5);
		good_input = 0;
		job.argc = 0;
		sz = 0;
		line = NULL;

		// read input
		if ((len = getline(&line, &sz, stdin)) > 0)
			good_input = 1;
		else
			free(line);

		if (good_input) {
			// parse input
			error = parse_line(line, len, &job);
			if (error && error != ERR_EMPTY_CMD)
				write(STDERR_FILENO, MSG_ERROR, strlen(MSG_ERROR));

			error = internal_func(&f, &job);
			if (error)
				write(STDERR_FILENO, MSG_ERROR, strlen(MSG_ERROR));

			for (int i=0;i<job.argc;i++)
				free(job.argv[i]);
			job.argc = 0;
		} else
			break;
	}

	clean_up(&f, &job);
	return (0);

error_exit:
	write(STDERR_FILENO, MSG_ERROR, strlen(MSG_ERROR));
	clean_up(&f, &job);
	return(0);
}
