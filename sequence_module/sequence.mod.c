#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
 .name = KBUILD_MODNAME,
 .init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
 .exit = cleanup_module,
#endif
 .arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0x3d694970, "module_layout" },
	{ 0x65289674, "kmalloc_caches" },
	{ 0xf444a047, "seq_open" },
	{ 0x4aabc7c4, "__tracepoint_kmalloc" },
	{ 0x576120be, "seq_printf" },
	{ 0x41ef32a4, "remove_proc_entry" },
	{ 0x6273c8cd, "seq_read" },
	{ 0xfd3c346d, "kmem_cache_alloc_notrace" },
	{ 0xb4390f9a, "mcount" },
	{ 0x898c48a, "create_proc_entry" },
	{ 0x12bbc623, "seq_lseek" },
	{ 0x37a0cba, "kfree" },
	{ 0xcfdead86, "seq_release" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "BCEFFA8CB4F6FFCC551CD45");
