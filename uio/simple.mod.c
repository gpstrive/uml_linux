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
	{ 0x8b8cade1, "driver_register" },
	{ 0x6994877, "platform_device_register_simple" },
	{ 0x4aabc7c4, "__tracepoint_kmalloc" },
	{ 0x4fbe8ea1, "uio_unregister_device" },
	{ 0xe06a47f7, "platform_bus_type" },
	{ 0xfd3c346d, "kmem_cache_alloc_notrace" },
	{ 0xb72397d5, "printk" },
	{ 0x69f2c68d, "driver_unregister" },
	{ 0xb4390f9a, "mcount" },
	{ 0x9b08c4e4, "platform_device_unregister" },
	{ 0x13a23846, "__uio_register_device" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=uio";


MODULE_INFO(srcversion, "E7C337DFDAC99B006BC5BF5");
