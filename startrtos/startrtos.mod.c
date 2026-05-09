#include <linux/module.h>
#include <linux/export-internal.h>
#include <linux/compiler.h>

MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};



static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0x92997ed8, "_printk" },
	{ 0x921b07b1, "__cpu_online_mask" },
	{ 0x50c7e6ee, "of_find_compatible_node" },
	{ 0x645c2e80, "rpi_firmware_get" },
	{ 0x469438cc, "of_node_put" },
	{ 0x50c2ae54, "rpi_firmware_property" },
	{ 0xf9a482f9, "msleep" },
	{ 0xf0fdf6cb, "__stack_chk_fail" },
	{ 0x401a591b, "param_ops_ullong" },
	{ 0xdf42cd46, "param_ops_int" },
	{ 0x474e54d2, "module_layout" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "E0A0B24C4B2830CF5EA97DA");
