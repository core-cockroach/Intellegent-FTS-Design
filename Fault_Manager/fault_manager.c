#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/resource.h>
#include <sys/reboot.h>
#include <time.h>
#include <errno.h>
#include <stdint.h>

#include "fault_manager.h"

/* ------------- Internal state (prevents redundant actions) ------------- */
typedef enum {
    MITIGATION_NONE = 0,
    MITIGATION_LOW,
    MITIGATION_MEDIUM,
    MITIGATION_HIGH
} MitigationLevel;

static MitigationLevel last_bus_level    = MITIGATION_NONE;
static MitigationLevel last_timing_level = MITIGATION_NONE;
static MitigationLevel last_mem_level    = MITIGATION_NONE;
static MitigationLevel last_os_level     = MITIGATION_NONE;

/* ------------------------------------------------------------------ *
 *  Helper: set nice value of the calling thread / process.
 *  For demonstration, we'll adjust the entire process (PRIO_PROCESS).
 *  In practice you'd target specific PIDs from a config file.
 * ------------------------------------------------------------------ */
static void set_nice(int priority) {
    if (setpriority(PRIO_PROCESS, 0, priority) != 0) {
        perror("setpriority");
    } else {
        printf("[FaultMgr] Set process nice value to %d\n", priority);
    }
}

/* ------------------------------------------------------------------ *
 *  Helper: send a signal to a process (PID stored in a file or config)
 *  For the example we'll use a predefined PID of a logger task.
 *  You need to adapt this to your actual system.
 * ------------------------------------------------------------------ */
#define LOGGER_PID_FILE "/var/run/logger.pid"
static void signal_logger(int sig) {
    FILE *fp = fopen(LOGGER_PID_FILE, "r");
    if (!fp) {
        fprintf(stderr, "[FaultMgr] Cannot open %s: %s\n", LOGGER_PID_FILE, strerror(errno));
        return;
    }
    pid_t pid;
    if (fscanf(fp, "%d", &pid) != 1) {
        fprintf(stderr, "[FaultMgr] Invalid PID in %s\n", LOGGER_PID_FILE);
        fclose(fp);
        return;
    }
    fclose(fp);
    if (kill(pid, sig) == -1) {
        perror("kill");
    } else {
        printf("[FaultMgr] Sent signal %d to logger PID %d\n", sig, pid);
    }
}

/* ------------------------------------------------------------------ *
 *  Helper: write to a /proc or /sys file (needs root).
 * ------------------------------------------------------------------ */
static int write_sysfs(const char *path, const char *value) {
    FILE *f = fopen(path, "w");
    if (!f) {
        fprintf(stderr, "[FaultMgr] Cannot open %s: %s\n", path, strerror(errno));
        return -1;
    }
    fprintf(f, "%s", value);
    fclose(f);
    printf("[FaultMgr] Wrote '%s' to %s\n", value, path);
    return 0;
}

/* ================================================================== *
 *  BUS SATURATION (0)
 * ================================================================== */
static void bus_saturation_low(void) {
    /* Throttle non‑critical traffic: e.g., reduce logging rate */
    printf("[FaultMgr] BUS: LOW – Throttling non‑critical bus traffic\n");
    signal_logger(SIGSTOP);   // stop logger to free bandwidth
}

static void bus_saturation_medium(void) {
    /* Prioritise critical messages: increase QoS / nice of important tasks */
    printf("[FaultMgr] BUS: MEDIUM – Boosting priority of critical bus tasks\n");
    set_nice(-5);  // higher priority for our process (example)
}

static void bus_saturation_high(void) {
    /* Shed load: stop all non‑essential communication, switch to low‑rate mode */
    printf("[FaultMgr] BUS: HIGH – Shedding bus load, signaling all non-essential tasks\n");
    signal_logger(SIGSTOP);   // already stopped, but ensure
    // Additional example: write to a sysfs entry that reduces bus interface speed
    write_sysfs("/sys/devices/platform/bus/can0/max_bitrate", "500000");
}

/* ================================================================== *
 *  TIMING FAULT (1)
 * ================================================================== */
static void timing_fault_low(void) {
    /* Adjust priorities: boost deadline-critical tasks */
    printf("[FaultMgr] TIMING: LOW – Boosting real‑time task priorities\n");
    set_nice(-10);
}

static void timing_fault_medium(void) {
    /* Relax deadlines: e.g., signal control process to increase its cycle time */
    printf("[FaultMgr] TIMING: MEDIUM – Sending SIGUSR1 to extend control loop period\n");
    // You would define a specific PID for the control task
    // kill(control_pid, SIGUSR1);   /* control task lowers its rate */
}

static void timing_fault_high(void) {
    /* Drop non‑critical tasks */
    printf("[FaultMgr] TIMING: HIGH – Stopping non‑critical tasks\n");
    signal_logger(SIGSTOP);
    // Also, try to bump the CPU governor to "performance"
    write_sysfs("/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor", "performance");
}

/* ================================================================== *
 *  MEMORY FRAGMENTATION (2)
 * ================================================================== */
static void memory_frag_low(void) {
    /* Clean caches / trim malloc arena */
    printf("[FaultMgr] MEM: LOW – Releasing cached memory\n");
    // Trigger malloc_trim (glibc) – this is a hint, actual effect depends
    if (malloc_trim(0)) {
        printf("[FaultMgr] malloc_trim succeeded\n");
    }
    // Drop clean caches (requires root)
    write_sysfs("/proc/sys/vm/drop_caches", "1");
}

static void memory_frag_medium(void) {
    /* Compact memory – only works if kernel support is present */
    printf("[FaultMgr] MEM: MEDIUM – Requesting memory compaction\n");
    write_sysfs("/proc/sys/vm/compact_memory", "1");
    // Also try to restart a task that might be leaking (example)
    signal_logger(SIGHUP);   // let logger reinitialise
}

static void memory_frag_high(void) {
    /* Suspend allocations and switch to pre‑reserved pools */
    printf("[FaultMgr] MEM: HIGH – Suspending non‑critical allocations, activating pre‑reserved pool\n");
    // Here you would set a flag that causes custom allocator to use a separate pool.
    // For demonstration, we’ll kill a memory‑intensive process.
    signal_logger(SIGKILL);
    // Also request compaction more aggressively
    write_sysfs("/proc/sys/vm/compact_memory", "1");
}

/* ================================================================== *
 *  OS FAULT (3)
 * ================================================================== */
static void os_fault_low(void) {
    /* For temperature or early signs, enable active cooling */
    printf("[FaultMgr] OS: LOW – Enabling fan and log warning\n");
    // Example: turn on a GPIO-driven fan
    write_sysfs("/sys/class/gpio/gpio17/value", "1");
    // Log to syslog
    system("logger 'Fault Manager: OS fault LOW – temperature > 85°C'");
}

static void os_fault_medium(void) {
    /* Save critical state and notify operator */
    printf("[FaultMgr] OS: MEDIUM – Saving diagnostic snapshot to /var/log/fault.log\n");
    // In a real system you'd dump registers, task states, etc.
    FILE *f = fopen("/var/log/fault.log", "a");
    if (f) {
        time_t now = time(NULL);
        fprintf(f, "OS fault MEDIUM at %s", ctime(&now));
        fclose(f);
    }
}

static void os_fault_high(void) {
    /* Execute controlled reboot or failover */
    printf("[FaultMgr] OS: HIGH – System is unstable, saving state and rebooting in 2 seconds...\n");
    // Last‑ditch save
    FILE *f = fopen("/var/log/fault.log", "a");
    if (f) {
        time_t now = time(NULL);
        fprintf(f, "OS fault HIGH – reboot triggered at %s", ctime(&now));
        fclose(f);
    }
    sync();
    // Wait a bit for the message to be printed, then reboot (needs root)
    sleep(2);
    reboot(RB_AUTOBOOT);
}

/* ================================================================== *
 *  Public interface
 * ================================================================== */
void fault_manager_init(void) {
    last_bus_level = MITIGATION_NONE;
    last_timing_level = MITIGATION_NONE;
    last_mem_level = MITIGATION_NONE;
    last_os_level = MITIGATION_NONE;
    printf("[FaultMgr] Initialised. Ready to receive predictions.\n");
}

void apply_mitigation(const FaultPrediction *pred) {
    MitigationLevel current_level;

    /* Map risk_score to a mitigation level */
    if (pred->risk_score > 80.0)
        current_level = MITIGATION_HIGH;
    else if (pred->risk_score > 50.0)
        current_level = MITIGATION_MEDIUM;
    else
        current_level = MITIGATION_LOW;

    switch (pred->fault_code) {
        case FAULT_BUS_SATURATION:
            if (current_level != last_bus_level) {
                if (current_level == MITIGATION_LOW)      bus_saturation_low();
                else if (current_level == MITIGATION_MEDIUM) bus_saturation_medium();
                else if (current_level == MITIGATION_HIGH)   bus_saturation_high();
                last_bus_level = current_level;
            }
            break;

        case FAULT_TIMING:
            if (current_level != last_timing_level) {
                if (current_level == MITIGATION_LOW)      timing_fault_low();
                else if (current_level == MITIGATION_MEDIUM) timing_fault_medium();
                else if (current_level == MITIGATION_HIGH)   timing_fault_high();
                last_timing_level = current_level;
            }
            break;

        case FAULT_MEM_FRAGMENTATION:
            if (current_level != last_mem_level) {
                if (current_level == MITIGATION_LOW)      memory_frag_low();
                else if (current_level == MITIGATION_MEDIUM) memory_frag_medium();
                else if (current_level == MITIGATION_HIGH)   memory_frag_high();
                last_mem_level = current_level;
            }
            break;

        case FAULT_OS:
            if (current_level != last_os_level) {
                if (current_level == MITIGATION_LOW)      os_fault_low();
                else if (current_level == MITIGATION_MEDIUM) os_fault_medium();
                else if (current_level == MITIGATION_HIGH)   os_fault_high();
                last_os_level = current_level;
            }
            break;

        default:
            /* No fault – reset all levels */
            if (last_bus_level   != MITIGATION_NONE) { last_bus_level   = MITIGATION_NONE; }
            if (last_timing_level!= MITIGATION_NONE) { last_timing_level= MITIGATION_NONE; }
            if (last_mem_level   != MITIGATION_NONE) { last_mem_level   = MITIGATION_NONE; }
            if (last_os_level    != MITIGATION_NONE) { last_os_level    = MITIGATION_NONE; }
            break;
    }
}

void fault_manager_deinit(void) {
    /* Restore priorities, restart killed tasks, etc. */
    printf("[FaultMgr] De‑initialising, restoring system state...\n");
    set_nice(0);  // reset priority to default
    // If we stopped the logger, bring it back
    signal_logger(SIGCONT);
    // Turn off fan
    write_sysfs("/sys/class/gpio/gpio17/value", "0");
    // Reset CPU governor
    write_sysfs("/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor", "ondemand");
}