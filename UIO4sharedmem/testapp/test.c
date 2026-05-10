#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdint.h>
#include <errno.h>
#include <dirent.h>

#define SHM_DATA_SIZE   16
#define UIO_NAME        "rtos_shm"

/**
 * Find the UIO device number that corresponds to our name.
 * Returns the full path like "/dev/uio2" or NULL on failure.
 */
char *find_uio_device(const char *target_name) {
    DIR *d = opendir("/sys/class/uio");
    if (!d) {
        perror("opendir /sys/class/uio");
        return NULL;
    }

    struct dirent *entry;
    while ((entry = readdir(d)) != NULL) {
        if (entry->d_name[0] == '.') continue;

        char name_path[256];
        snprintf(name_path, sizeof(name_path), "/sys/class/uio/%s/name", entry->d_name);
        FILE *f = fopen(name_path, "r");
        if (!f) continue;

        char buf[64];
        if (fgets(buf, sizeof(buf), f)) {
            // Remove trailing newline
            buf[strcspn(buf, "\n")] = 0;
            if (strcmp(buf, target_name) == 0) {
                fclose(f);
                closedir(d);

                char *dev_path = malloc(64);
                if (dev_path)
                    snprintf(dev_path, 64, "/dev/%s", entry->d_name);
                return dev_path;
            }
        }
        fclose(f);
    }
    closedir(d);
    return NULL;
}

struct shared_mem {
    volatile uint32_t rtos_flag;
    int32_t           rtos_payload[SHM_DATA_SIZE];
    volatile uint32_t linux_flag;
    int32_t           solution[SHM_DATA_SIZE];
};

int main() {
    // 1. Auto-detect the device path
    char *device_path = find_uio_device(UIO_NAME);
    if (!device_path) {
        fprintf(stderr, "ERROR: Could not find UIO device with name '%s'\n", UIO_NAME);
        fprintf(stderr, "Make sure the kernel module is loaded (insmod shm_uio.ko)\n");
        return 1;
    }
    printf("Found UIO device: %s\n", device_path);

    int fd = open(device_path, O_RDWR);
    if (fd < 0) {
        perror("open failed");
        free(device_path);
        return 1;
    }
    printf("Opened %s, fd=%d\n", device_path, fd);

    size_t map_size = sizeof(struct shared_mem);
    printf("Attempting mmap: size=%zu, offset=0\n", map_size);

    volatile struct shared_mem *shm = (volatile struct shared_mem *)
        mmap(NULL, map_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    if (shm == MAP_FAILED) {
        perror("mmap");
        fprintf(stderr, "mmap failed with errno=%d (%s)\n", errno, strerror(errno));
        close(fd);
        free(device_path);
        return 1;
    }

    printf("mmap succeeded. Shared memory mapped at %p\n", (void *)shm);

    // Optional: read mapping size from sysfs for verification
    {
        char map_size_path[256];
        snprintf(map_size_path, sizeof(map_size_path),
                 "/sys/class/uio/%s/maps/map0/size",
                 device_path + 5);  // skip "/dev/"
        FILE *fs = fopen(map_size_path, "r");
        if (fs) {
            unsigned long avail;
            if (fscanf(fs, "%lx", &avail) == 1)
                printf("UIO region size available: 0x%lx (%lu bytes)\n", avail, avail);
            fclose(fs);
        }
    }

    // --- Main polling loop ---
    printf("Entering main loop (polling rtos_flag)...\n");
    while (1) {
        if (shm->rtos_flag == 1) {
            // Memory barrier: ensure payload reads happen AFTER flag read
            __sync_synchronize();

            int32_t data[SHM_DATA_SIZE];
            for (int i = 0; i < SHM_DATA_SIZE; i++) {
                data[i] = shm->rtos_payload[i];
            }
            printf("Received data from RTOS: [%d, %d, ...]\n", data[0], data[1]);

            // Clear the flag to signal consumption
            shm->rtos_flag = 0;

            // --- Placeholder for ML inference ---
            // (you would process 'data' here)

            // Fill solution and notify RTOS
            for (int i = 0; i < SHM_DATA_SIZE; i++) {
                shm->solution[i] = i * 10;   // dummy result
            }
            __sync_synchronize();   // barrier: make solution visible
            shm->linux_flag = 1;

            printf("Solution sent to RTOS, linux_flag set.\n");
        }
        usleep(1000);  // 1 ms polling interval
    }

    // Cleanup (never reached in this example)
    munmap((void *)shm, map_size);
    close(fd);
    free(device_path);
    return 0;
}