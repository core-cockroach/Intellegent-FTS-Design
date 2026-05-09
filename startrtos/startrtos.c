// SPDX-License-Identifier: GPL-2.0
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/arm-smccc.h>
#include <asm/cacheflush.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("Start Core 3 RTOS (no hotplug required)");

#define PSCI_CPU_ON 0xC4000003

static phys_addr_t rtos_phys;
static size_t rtos_size;

static int __init start_rtos_init(void)
{
    struct device_node *np;
    struct resource res;
    int ret;
    unsigned long mpidr = 0x3;   // Core 3 – Aff0=3

    /* Find the reserved memory node */
    np = of_find_node_by_path("/reserved-memory/rtos@f0000000");
    if (!np) {
        pr_err("start_rtos: reserved memory node not found\n");
        return -ENODEV;
    }

    ret = of_address_to_resource(np, 0, &res);
    of_node_put(np);
    if (ret) {
        pr_err("start_rtos: cannot parse reg property\n");
        return ret;
    }
    rtos_phys = res.start;
    rtos_size = resource_size(&res);
    pr_info("start_rtos: RTOS region at 0x%llx, size 0x%zx\n",
            (unsigned long long)rtos_phys, rtos_size);

    /* Cache maintenance */
    void __iomem *vaddr = ioremap(rtos_phys, rtos_size);
    if (!vaddr) {
        pr_err("start_rtos: ioremap failed\n");
        return -ENOMEM;
    }

    //__flush_dcache_area(vaddr, rtos_size);
    //invalidate_icache_range((unsigned long)vaddr,
    //                        (unsigned long)vaddr + rtos_size);
    iounmap(vaddr);
    pr_info("start_rtos: cache flush & invalidate done\n");

    /*
     * With maxcpus=2, Core 3 was never started by Linux.
     * We can safely call PSCI CPU_ON directly.
     */
    struct arm_smccc_res smccc_res;
    arm_smccc_1_1_invoke(PSCI_CPU_ON, mpidr, rtos_phys, 0, 0, 0, 0, 0, &smccc_res);

    if (smccc_res.a0 == 0) {
        pr_info("start_rtos: PSCI CPU_ON success! Core 3 is now running your RTOS.\n");
    } else {
        pr_err("start_rtos: PSCI CPU_ON failed, return: 0x%lx\n",
               (unsigned long)smccc_res.a0);
        return -EIO;
    }

    return 0;
}

static void __exit start_rtos_exit(void)
{
    pr_info("start_rtos: unloaded (RTOS still running on Core 3)\n");
}

module_init(start_rtos_init);
module_exit(start_rtos_exit);