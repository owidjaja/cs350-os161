#include "opt-A1.h"

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
for (unsigned int i=0; i<array_num(p->p_children); i++){
  struct proc *temp_child = array_get(p->p_children, i);
  array_remove(p->p_children, i);
  
  spinlock_acquire(&temp_child->p_lock);
  if (temp_child->p_exitstatus == 0){
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

#if OPT_A1    // a1: 5.3.2
  spinlock_acquire(&p->p_lock);         // synchronization lock access
  if (p->p_parent!= NULL){  // parent still running
    p->p_exitstatus = 0;                // mark p as exited
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

  /* this is just a stub implementation that always reports an
     exit status of 0, regardless of the actual exit status of
     the specified process.   
     In fact, this will return 0 even if the specified process
     is still running, and even if it never existed in the first place.

     Fix this!
  */

  if (options != 0) {
    return(EINVAL);
  }
  /* for now, just pretend the exitstatus is 0 */
  exitstatus = 0;
  result = copyout((void *)&exitstatus,status,sizeof(int));
  if (result) {
    return(result);
  }
  *retval = pid;
  return(0);
}

#if OPT_A1	// a1: 5.2
int sys_fork(pid_t *retval, struct trapframe *parent_tf){

	// create child process
	struct proc *child_proc = proc_create_runprogram("child");
	if (child_proc == NULL) { return ENOMEM; }

  // a1: 5.3.1
  child_proc->p_parent = curproc;

  // a1: 5.3.3
  int add_err = array_add(curproc->p_children, child_proc, NULL); // index_ret pointer set to NULL
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
