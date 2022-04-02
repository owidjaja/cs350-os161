#include "opt-A1.h"
#include "opt-A3.h"

#include <types.h>
#include <kern/errno.h>
#include <kern/unistd.h>
#include <kern/wait.h>
#include <lib.h>
#include <syscall.h>
#include <current.h>
#include <proc.h>
#include <thread.h>
#include <addrspace.h>
#include <copyinout.h>
#include <mips/trapframe.h>
#include <clock.h>

  /* this implementation of sys__exit does not do anything with the exit code */
  /* this needs to be fixed to get exit() and waitpid() working properly */

void sys__exit(int exitcode) {

  struct addrspace *as;
  struct proc *p = curproc;
  /* for now, just include this to keep the compiler from complaining about
     an unused variable */
  (void)exitcode;

  DEBUG(DB_SYSCALL,"Syscall: _exit(%d)\n",exitcode);

  KASSERT(curproc->p_addrspace != NULL);
  as_deactivate();
  /*
   * clear p_addrspace before calling as_destroy. Otherwise if
   * as_destroy sleeps (which is quite possible) when we
   * come back we'll be calling as_activate on a
   * half-destroyed address space. This tends to be
   * messily fatal.
   */
  as = curproc_setas(NULL);
  as_destroy(as);

#if OPT_A1    // a1 - 5.3.3: monitoring child proc from parent
unsigned int num_child = array_num(p->p_children);
// kprintf("[a1] num_child=%d\n", num_child);
for (unsigned int i=0; i<num_child; i++){
	// kprintf("[a1] index =%d\n", i);
	// kprintf("[a1] a->num=%d\n", array_num(p->p_children));
  struct proc *temp_child = array_get(p->p_children, 0);
  array_remove(p->p_children, 0);
  
  spinlock_acquire(&temp_child->p_lock);
  if (temp_child->p_exitstatus == 1){
    // exited
    spinlock_release(&temp_child->p_lock);
    proc_destroy(temp_child);
  }
  else{
    // running
    temp_child->p_parent = NULL;
    spinlock_release(&temp_child->p_lock);
  }
}

#endif

  /* detach this thread from its process */
  /* note: curproc cannot be used after this call */
  proc_remthread(curthread);

#if OPT_A1    // a1 - 5.3.2: monitor parent from child
  spinlock_acquire(&p->p_lock);         // synchronization lock access
  if (p->p_parent!= NULL){  // parent still running
    p->p_exitstatus = 1;                // mark p as exited
    p->p_exitcode = exitcode;
    spinlock_release(&p->p_lock);
  }
  else{                                 // parent exited
    spinlock_release(&p->p_lock);
    proc_destroy(p);
  }
#else
  /* if this is the last user process in the system, proc_destroy()
     will wake up the kernel menu thread */
  proc_destroy(p);
#endif
  thread_exit();
  /* thread_exit() does not return, so we should never get here */
  panic("return from thread_exit in sys_exit\n");
}


/* stub handler for getpid() system call                */
int
sys_getpid(pid_t *retval)
{
  #if OPT_A1
    *retval = curproc->p_pid;
    return (0);

  #else
    /* for now, this is just a stub that always returns a PID of 1 */
    /* you need to fix this to make it work properly */
    *retval = 1;
    return(0);

  #endif /* OPT_A1 */
}

/* stub handler for waitpid() system call                */

int
sys_waitpid(pid_t pid,
	          userptr_t status,
	          int options,
	          pid_t *retval)
{
  int exitstatus;
  int result;

  if (options != 0) {
    return(EINVAL);
  }

#if OPT_A1    // a1 - 5.4: waitpid
  struct proc *p = curproc;
  struct proc *this_cp = NULL;
  struct proc *temp_child = NULL;

	unsigned int num_child = array_num(p->p_children);
  for (unsigned int i=0; i<num_child; i++){
    this_cp = array_get(p->p_children, 0);
    if (this_cp->p_pid == pid){
      // found child_proc
        temp_child = this_cp;
        array_remove(p->p_children, 0);
        break;
    }
  }

  if (temp_child == NULL){
    // child pid not found
    // missing passing exit code to exitstatus, which passes to userptr_t status
    *retval = -1;
    return ESRCH;
  }

  // busy polling, provided by instructions
  // breaks if temp_child->p_exitstatus == 1 (exited)
  spinlock_acquire (&temp_child ->p_lock);
  while (!temp_child ->p_exitstatus) {
    spinlock_release (&temp_child ->p_lock);
    clocksleep (1);
    spinlock_acquire (&temp_child ->p_lock);
  }
  spinlock_release (&temp_child ->p_lock);

  // temporary pass to _MKWAIT_EXIT in a1
  exitstatus = _MKWAIT_EXIT(temp_child->p_exitcode);
  proc_destroy(temp_child);

#else
  /* this is just a stub implementation that always reports an
     exit status of 0, regardless of the actual exit status of
     the specified process.   
     In fact, this will return 0 even if the specified process
     is still running, and even if it never existed in the first place.

     Fix this!
  */

  /* for now, just pretend the exitstatus is 0 */
  exitstatus = 0;
#endif

  result = copyout((void *)&exitstatus,status,sizeof(int));
  if (result) {
    return(result);
  }
  *retval = pid;
  return(0);
}

#if OPT_A1	// a1: 5.2
int sys_fork(pid_t *retval, struct trapframe *parent_tf){
  struct proc *p = curproc;

	// create child process
	struct proc *child_proc = proc_create_runprogram("child");
	if (child_proc == NULL) { return ENOMEM; }

  // a1: 5.3.1
  child_proc->p_parent = p;

  // a1: 5.3.3
  int add_err = array_add(p->p_children, child_proc, NULL); // index_ret pointer set to NULL
  if (add_err) { return add_err; }

  // for parent to return child pid
  *retval = child_proc->p_pid;

	// copy address space
	int ac_err = as_copy(curproc_getas(), &(child_proc->p_addrspace));
	if (ac_err != 0) { return ac_err; }

	// create new trapframe for child, and copy parent tf contents
	struct trapframe *child_tf = kmalloc(sizeof(struct trapframe));
	*child_tf = *parent_tf;

	// create thread
  // on having ampersand & in function pointer parameter
  // https://stackoverflow.com/questions/16917043/do-function-pointers-need-an-ampersand
  int th_fork_err = thread_fork("child_thread", child_proc, enter_forked_process, child_tf, 0);
	if (th_fork_err != 0) { return th_fork_err; }

  clocksleep(1);
	return 0;
}
#endif

#if OPT_A3    // 5: implementing execv
#include <kern/fcntl.h>
#include <vfs.h>
#include <test.h>
#define MAXARGS    16
#define MAXARGSIZE 128

int sys_execv(char *progname, char **tf_argv){
  struct addrspace *as;
	struct vnode *v;
	vaddr_t entrypoint, stackptr;
	int result;

	/* Open the file. */
	result = vfs_open(progname, O_RDONLY, 0, &v);
	if (result) {
		return result;
	}

	// kprintf("[a3] args_alloc\n");
  char **kern_args = args_alloc();

  // kprintf("[a3] start argcopy_in\n");
  unsigned int nargs = argcopy_in(kern_args, tf_argv);
  // kprintf("[a3] finish argcopy_in\n");
	
	// for (unsigned int i=0; i<nargs; i++){
  //   kprintf("[a3] kern_args[%d]: '%s'\n", i, kern_args[i]);
	// }

  // kprintf("[a3] create new address space\n");
	/* Create a new address space. */
	as = as_create();
	if (as ==NULL) {
		vfs_close(v);
		return ENOMEM;
	}

	struct addrspace *old_as = curproc_getas();
  // kprintf("[a3] switch and activate\n");
	/* Switch to it and activate it. */
	curproc_setas(as);
	as_activate();
  as_destroy(old_as);

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


  // my code

	// get argv: array of addresses of arguments
	char **argv;
	size_t argv_memsize = (nargs + 1) * sizeof(userptr_t);
  argv = kmalloc(argv_memsize);
    
	for (unsigned int i=0; i<nargs; i++){
		argv[i] = (char *) argcopy_out(&stackptr, kern_args[i]);
	}	
	argv[nargs] = NULL;


  // copy out argv array into space above stack
	stackptr -= (stackptr % 4);
	stackptr -= (nargs + 1)*sizeof(userptr_t);
	int err = copyout(argv, (userptr_t) stackptr, argv_memsize);
	if (err){
		return err;
	}
	kfree(argv);
	args_free(kern_args);

	
	/* Warp to user mode. */
	enter_new_process(nargs, (userptr_t) stackptr, stackptr, entrypoint);
	
	/* enter_new_process does not return. */
	panic("enter_new_process returned\n");
	return EINVAL;
}

char **args_alloc(){
  char **args;
  args = (char **) kmalloc((MAXARGS+1) * sizeof(char *));

  for (int i=0; i<MAXARGS; i++){
    args[i] = (char *) kmalloc((MAXARGSIZE+1) * sizeof(char));
  }
  args[MAXARGS] = NULL;

  return args;
}

void args_free(char **args){
  for (int i=0; i<MAXARGS; i++){
    kfree(args[i]);
  }
  kfree(args);
}

// accepts a dynamically allocated array of buffers,
// sequentially copies in command line arguments from userspace using copyinstr, 
// and returns the total
// number of strings copied in. Hint: The array in userspace is NULL terminated
int argcopy_in(char **kern_args, char **user_argv){
  size_t got;
	unsigned int argc = 0;
  for (; argc<MAXARGS; argc++){
    // kprintf("[a3] user_argv[%d]: '%s'\n", argc, user_argv[argc]);
		if (user_argv[argc] == NULL){
			break;
		}
    copyinstr((userptr_t) user_argv[argc], kern_args[argc], 128, &got);
  }

  return argc;
}


#endif
