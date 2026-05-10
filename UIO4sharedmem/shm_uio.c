#include <linux/module.h>
#include <linux/uio_driver.h>
#include <linux/platform_device.h>

/* 
 * Reserved memory: 200 MB from 0xF0000000 to 0xFC800000.
 * We place the shared struct at offset 180 MB (0xB400000) + 4 KB,
 * so physical address = 0xF0000000 + 0xB400000 = 0xFB400000.
 * This is page-aligned (4 KB) and fits within the reserved block.
 */
#define SHM_PHYS_ADDR   0xFB400000
#define SHM_SIZE        0x1000       // 4 KB, one page

static struct uio_info shm_uio_info = {
    .name    = "rtos_shm",
    .version = "1.0",
    .irq     = UIO_IRQ_NONE,

    .mem[0] = {
        .memtype = UIO_MEM_PHYS,
        .addr    = SHM_PHYS_ADDR,
        .size    = SHM_SIZE,
    },
};

static struct platform_device *shm_pdev;

static int __init shm_uio_init(void)
{
    int ret;

    pr_info("rtos_shm: initializing...\n");

    /* 1. Create dummy platform device */
    shm_pdev = platform_device_register_simple("rtos_shm_dev", -1, NULL, 0);
    if (IS_ERR(shm_pdev)) {
        pr_err("rtos_shm: ERROR - platform_device_register_simple failed (err=%ld)\n",
               PTR_ERR(shm_pdev));
        return PTR_ERR(shm_pdev);
    }
    pr_info("rtos_shm: platform device created successfully\n");

    /* 2. Register UIO device */
    pr_info("rtos_shm: registering UIO device (phys=0x%lx, size=%lu, irq=none)\n",
            (unsigned long)SHM_PHYS_ADDR, (unsigned long)SHM_SIZE);

    ret = uio_register_device(&shm_pdev->dev, &shm_uio_info);
    if (ret != 0) {
        pr_err("rtos_shm: ERROR - uio_register_device failed (err=%d)\n", ret);
        platform_device_unregister(shm_pdev);
        return ret;              // <-- better to return the actual error code
    }

    pr_info("rtos_shm: UIO device registered successfully as /dev/uioX\n");
    pr_info("rtos_shm: name=%s, version=%s\n",
            shm_uio_info.name, shm_uio_info.version);
    pr_info("rtos_shm: memory region[0]: addr=0x%lx, size=%lu\n",
            (unsigned long)shm_uio_info.mem[0].addr,
            (unsigned long)shm_uio_info.mem[0].size);

    return 0;
}

static void __exit shm_uio_exit(void)
{
    pr_info("rtos_shm: removing UIO device...\n");
    uio_unregister_device(&shm_uio_info);
    platform_device_unregister(shm_pdev);
    pr_info("rtos_shm: UIO device removed\n");
}

module_init(shm_uio_init);
module_exit(shm_uio_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("cockroach");
MODULE_DESCRIPTION("UIO driver for RTOS shared memory (uncached mapping)");