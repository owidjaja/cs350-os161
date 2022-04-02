#include "opt-A3.h"
/*
 * Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2008, 2009
 *	The President and Fellows of Harvard College.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE UNIVERSITY AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE UNIVERSITY OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * Sample/test code for running a user program.  You can use this for
 * reference when implementing the execv() system call. Remember though
 * that execv() needs to do more than this function does.
 */

#include <types.h>
#include <kern/errno.h>
#include <kern/fcntl.h>
#include <lib.h>
#include <proc.h>
#include <current.h>
#include <addrspace.h>
#include <vm.h>
#include <vfs.h>
#include <syscall.h>
#include <test.h>

#if OPT_A3
#include <copyinout.h>
#endif

/*
 * Load program "progname" and start running it in usermode.
 * Does not return except on error.
 *
 * Calls vfs_open on progname and thus may destroy it.
 */
int
#if OPT_A3
runprogram(char *progname, char **args, unsigned long nargs)
#else
runprogram(char *progname)
#endif
{
	struct addrspace *as;
	struct vnode *v;
	vaddr_t entrypoint, stackptr;
	int result;

	/* Open the file. */
	result = vfs_open(progname, O_RDONLY, 0, &v);
	if (result) {
		return result;
	}

	/* We should be a new process. */
	KASSERT(curproc_getas() == NULL);

	/* Create a new address space. */
	as = as_create();
	if (as ==NULL) {
		vfs_close(v);
		return ENOMEM;
	}

	/* Switch to it and activate it. */
	curproc_setas(as);
	as_activate();

	/* Load the executable. */
	result = load_elf(v, &entrypoint);
	if (result) {
		/* p_addrspace will go away when curproc is destroyed */
		vfs_close(v);
		return result;
	}

	/* Done with the file now. */
	vfs_close(v);

	/* Define the user stack in the address space */
	result = as_define_stack(as, &stackptr);
	if (result) {
		/* p_addrspace will go away when curproc is destroyed */
		return result;
	}

	// kprintf("[A3] executing my code..\n\n");
#if OPT_A3		// a3: 4.2

	// get argv: array of addresses of arguments
	char **argv;
	size_t argv_memsize = (nargs + 1) * sizeof(userptr_t);
    argv = kmalloc(argv_memsize);
    
	for (unsigned int i=0; i<nargs; i++){
		argv[i] = (char *) argcopy_out(&stackptr, args[i]);
	}	
	argv[nargs] = NULL;

	// kprintf("[A3] handle argv\n");
	// kprintf("[A3] old stackptr: 0x%08x = %u\n", stackptr, stackptr);

	// rounding stackptr to multiple of 4
	stackptr -= (stackptr % 4);
	// kprintf("[A3] rou stackptr: 0x%08x = %u\n", stackptr, stackptr);
	
	stackptr -= (nargs + 1)*sizeof(userptr_t);
	// kprintf("[A3] new stackptr: 0x%08x = %u\n", stackptr, stackptr);
	// kprintf("[A3] sizeof(argv): 0x%08x = %u\n", argv_memsize, argv_memsize);

	// copy out argv array into space above stack
	int err = copyout(argv, (userptr_t) stackptr, argv_memsize);
	if (err){
		return err;
	}

	kfree(argv);
	
	// kprintf("[A3] reached enter_new_process\n");
	// kprintf("[A3] fin stackptr: 0x%08x = %u\n\n", stackptr, stackptr);
	/* Warp to user mode. */
	enter_new_process(nargs, (userptr_t) stackptr, stackptr, entrypoint);
	
	/* enter_new_process does not return. */
	panic("enter_new_process returned\n");
	return EINVAL;

#else
	/* Warp to user mode. */
	enter_new_process(0 /*argc*/, NULL /*userspace addr of argv*/,
			  stackptr, entrypoint);
	
	/* enter_new_process does not return. */
	panic("enter_new_process returned\n");
	return EINVAL;
#endif
}

#if OPT_A3		// a3: 4.2
userptr_t argcopy_out(vaddr_t *stackptr, char *strcopy_out){
	// kprintf("[A3] in argcopy_out ..\n");
	// kprintf("[A3] strcopy_out: '%s'\n", strcopy_out);
	// kprintf("[A3] old stackptr: 0x%08x\n", *stackptr);
	// for each argument in argv[]
	size_t len = strlen(strcopy_out) + 1;
	*stackptr -= len;
	// kprintf("[A3] new stackptr: 0x%08x\n", *stackptr);

	size_t *got;
	// userptr_t temp_ptr = (userptr_t) (stackptr);
	// kprintf("[A3] init userptr: 0x%08x\n", (vaddr_t) temp_ptr);

	int gotlen = copyoutstr(strcopy_out, (userptr_t) *stackptr, len, got);
	(void) gotlen;
	
	// kprintf("[A3] rtrn userptr: 0x%08x\n", (vaddr_t) temp_ptr);
	// kprintf("[A3] ret stackptr: 0x%08x\n\n", *stackptr);
	return (userptr_t) *stackptr;
}
#endif
