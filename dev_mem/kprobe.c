/*
 * root@libcrack.so
 * kprobe to bypass > 1024M read from /dev/mem
 * From  /usr/src/linux-3.6-rc3/arch/x86/mm/init.c:
 *     devmem_is_allowed() checks to see if /dev/mem access to a certain address
 *     is valid. The argument is a physical page number.
 *     On x86, access has to be given to the first megabyte of ram because that area
 *     contains bios code and data regions used by X and dosemu and similar apps.
 *     Access has to be given to non-kernel-ram areas as well, these contain the PCI
 *     mmio resources as well as potential bios/acpi data regions.
 *
 *     int devmem_is_allowed(unsigned long pagenr)
 *     {
 *             if (pagenr <= 256)
 *                     return 1;
 *             if (iomem_is_exclusive(pagenr << PAGE_SHIFT))
 *                     return 0;
 *             if (!page_is_ram(pagenr))
 *                     return 1;
 *             return 0;
 *     }
 *
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/ptrace.h>
#include <linux/kprobes.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("root@libcrack.so");
MODULE_DESCRIPTION("kprobe to bypass >1024M read from /dev/mem");

/**************** CONSTANTS ***************/
static const char *func_name = "devmem_is_allowed";

/**************** FUNC HOOKS ***************/
static int ret_handler (struct kretprobe_instance *rp, struct pt_regs *regs)
{
    /* denied access bypass  */
    if (regs->ax == 0) {
        //printk("Intercepted %s returns (%%eax) 0 => setting %%eax to 0x01\n", func_name);
        regs->ax = 0x1;
    }
    return 0;   /* not reached */
}

/**************** KRETPROBE ***************/
static struct kretprobe krp = {
    .handler = ret_handler,
    .maxactive = 20 /* Probe up to 20 instances concurrently. */
};

/**************** INIT MODULE ***************/
int init_module(void)
{
    int ret;
    printk("Activating %s kreprobe \n", func_name);

    krp.kp.symbol_name = func_name;

    if ((ret=register_kretprobe(&krp)) < 0) {
        printk("register_kprobe for %s failed!\n", func_name);
    } else {
        printk("Kprobed %s :-D \n", func_name);
    }
    return 0;
}

/**************** CLEANUP MODULE ***************/
void cleanup_module(void)
{
    printk("Unregistering %s kprobe\n", func_name);
    unregister_kretprobe(&krp);
    printk("%s kprobe unregistered\n", func_name);
}
