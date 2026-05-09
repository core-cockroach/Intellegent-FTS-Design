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
	{ 0xe8213e80, "_printk" },
	{ 0xb660992f, "of_find_node_opts_by_path" },
	{ 0x48d7b3f0, "of_address_to_resource" },
	{ 0xdc04a5a6, "of_node_put" },
	{ 0x3239fbdb, "arm64_use_ng_mappings" },
	{ 0xe4de56b4, "__ubsan_handle_load_invalid_value" },
	{ 0xee285526, "ioremap_prot" },
	{ 0x12ad300e, "iounmap" },
	{ 0xa62cb5a3, "arm_smccc_1_1_get_conduit" },
	{ 0xd272d446, "__stack_chk_fail" },
	{ 0xa84200a9, "module_layout" },
};

static const u32 ____version_ext_crcs[]
__used __section("__version_ext_crcs") = {
	0xe8213e80,
	0xb660992f,
	0x48d7b3f0,
	0xdc04a5a6,
	0x3239fbdb,
	0xe4de56b4,
	0xee285526,
	0x12ad300e,
	0xa62cb5a3,
	0xd272d446,
	0xa84200a9,
};
static const char ____version_ext_names[]
__used __section("__version_ext_names") =
	"_printk\0"
	"of_find_node_opts_by_path\0"
	"of_address_to_resource\0"
	"of_node_put\0"
	"arm64_use_ng_mappings\0"
	"__ubsan_handle_load_invalid_value\0"
	"ioremap_prot\0"
	"iounmap\0"
	"arm_smccc_1_1_get_conduit\0"
	"__stack_chk_fail\0"
	"module_layout\0"
;

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "9F881B403D977CC0DE924A2");
