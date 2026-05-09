#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/cpu.h>
#include <linux/of.h>
#include <soc/bcm2835/raspberrypi-firmware.h>

/* 64‑bit boot address tag (BCM2712) */
#define RPI_FIRMWARE_SET_BOOT_ADDR64  0x00030004

static int target_cpu = 3;
static unsigned long long entry_point = 0xF0008000;   // your RTOS start

module_param(target_cpu, int, 0644);
module_param(entry_point, ullong, 0644);

static int __init startrtos_init(void)
{
    struct device_node *fw_np;
    struct rpi_firmware *fw;
    u32 packet[3];   // [ core_id, addr_lo, addr_hi ]
    int ret;

    if (cpu_online(target_cpu)) {
        pr_err("startrtos: CPU%d online. Reboot with maxcpus=3.\n", target_cpu);
        return -EINVAL;
    }

    /* get firmware handle */
    fw_np = of_find_compatible_node(NULL, NULL, "raspberrypi,bcm2712-firmware");
    if (!fw_np)
        fw_np = of_find_compatible_node(NULL, NULL, "raspberrypi,bcm2835-firmware");
    if (!fw_np) {
        pr_err("startrtos: firmware node not found\n");
        return -ENODEV;
    }
    fw = rpi_firmware_get(fw_np);
    of_node_put(fw_np);
    if (!fw) {
        pr_err("startrtos: firmware not probed\n");
        return -ENODEV;
    }

    /* set boot address (64‑bit tag) */
    packet[0] = target_cpu;
    packet[1] = (u32)entry_point;           // low word
    packet[2] = (u32)(entry_point >> 32);   // high word

    pr_info("startrtos: setting boot addr for CPU%d to 0x%llx\n", target_cpu, entry_point);
    ret = rpi_firmware_property(fw, RPI_FIRMWARE_SET_BOOT_ADDR64,
                                packet, sizeof(packet));
    if (ret) {
        pr_err("startrtos: SET_BOOT_ADDR64 failed: %d\n", ret);
        return ret;
    }

    /* wake the core */
    pr_info("startrtos: sending SEV to CPU%d\n", target_cpu);
    asm volatile("dsb ish\n sev" ::: "memory");
    msleep(100);

    pr_info("startrtos: RTOS should now be running on CPU%d\n", target_cpu);
    return 0;
}

static void __exit startrtos_exit(void)
{
    pr_info("startrtos: module unloaded (RTOS core still active)\n");
}

module_init(startrtos_init);
module_exit(startrtos_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("cockroach");
MODULE_DESCRIPTION("RTOS boot for RPi5 – final");