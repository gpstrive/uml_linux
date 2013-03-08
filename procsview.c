nclude <linux/kernel.h>
#include <linux/module.h>
#include <linux/sched.h>

int init_module( void )
{
      /* Set up the anchor point */
      struct task_struct *task = &init_task;

        /* Walk through the task list, until we hit the init_task again */
        do {

                printk( KERN_INFO "*** %s [%d] parent %s\n",
                                    task->comm, task->pid, task->parent->comm );

                  } while ( (task = next_task(task)) != &init_task );

          return 0;

}

void cleanup_module( void )
{
      return;
}
