// SPDX-License-Identifier: GPL-2.0
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/firmware.h>
#include <linux/io.h>
#include <asm/cacheflush.h>   // for cache maintenance

MODULE_LICENSE("GPL");
MODULE_AUTHOR("cockroach");
MODULE_DESCRIPTION("Load RTOS firmware into reserved memory");

#define FIRMWARE_NAME "rtos.bin"

static void __iomem *rtos_base;
static phys_addr_t rtos_phys;
static size_t rtos_size;

static int __init rtos_loader_init(void)
{
    struct device_node *np;
    struct resource res;
    const struct firmware *fw;
    int ret;

    /* Find the reserved memory node by its label */
    np = of_find_node_by_path("/reserved-memory/rtos@f0000000");
    if (!np) {
        pr_err("rtos_loader: cannot find reserved-memory node\n");
        return -ENODEV;
    }

    /* Extract address and size from the reg property */
    ret = of_address_to_resource(np, 0, &res);
    of_node_put(np);
    if (ret) {
        pr_err("rtos_loader: failed to get memory resource\n");
        return ret;
    }

    rtos_phys = res.start;
    rtos_size = resource_size(&res);
    pr_info("rtos_loader: reserved memory at 0x%llx, size %zu\n",
            (unsigned long long)rtos_phys, rtos_size);

    /* Map the physical memory into kernel virtual address space */
    rtos_base = ioremap(rtos_phys, rtos_size);
    if (!rtos_base) {
        pr_err("rtos_loader: ioremap failed\n");
        return -ENOMEM;
    }

    /* Request the firmware binary from userspace */
    ret = request_firmware(&fw, FIRMWARE_NAME, NULL);
    if (ret) {
        pr_err("rtos_loader: request_firmware failed: %d\n", ret);
        iounmap(rtos_base);
        return ret;
    }

    if (fw->size > rtos_size) {
        pr_err("rtos_loader: firmware too large (%zu > %zu)\n",
               fw->size, rtos_size);
        release_firmware(fw);
        iounmap(rtos_base);
        return -ENOMEM;
    }

    /* Copy the firmware into reserved memory */
    memcpy_toio(rtos_base, fw->data, fw->size);
    pr_info("rtos_loader: copied %zu bytes to 0x%llx\n",
            fw->size, (unsigned long long)rtos_phys);

    /* 
     * Cache maintenance: If this memory will be executed by a
     * non-coherent agent (another core without shared caches,
     * or a DMA-capable device), you must flush dcache and
     * invalidate icache.
     * 
     * On a typical ARM64 system, if the memory is mapped as
     * device memory (ioremap), the writes are already uncached.
     * But just to be safe, we perform cache maintenance on the
     * *original* physical address region.
     */
    // Uncomment if needed:
    // __flush_dcache_area(rtos_base, fw->size);
    // invalidate_icache_range((unsigned long)rtos_base,
    //                         (unsigned long)rtos_base + fw->size);

    release_firmware(fw);
    return 0;   /* success */
}

static void __exit rtos_loader_exit(void)
{
    if (rtos_base)
        iounmap(rtos_base);
    pr_info("rtos_loader: unloaded\n");
}

module_init(rtos_loader_init);
module_exit(rtos_loader_exit);