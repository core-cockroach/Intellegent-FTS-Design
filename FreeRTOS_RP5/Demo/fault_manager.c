#include "./include/fault_manager.h"

volatile uint8_t use_reserved_pool;

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

static xTaskHandle xLoggerTask = NULL;
xTaskHandle xControlTask = NULL;     // set by application
xQueueHandle xControlQueue = NULL;   // set by application

static void set_task_priority(xTaskHandle task, unsigned portBASE_TYPE priority) {
    if (task) {
        vTaskPrioritySet(task, priority);
        printf("[FaultMgr] Priority set to %u\n", (unsigned)priority);
    }
}

static void suspend_task(xTaskHandle task) { if (task) vTaskSuspend(task); }
static void resume_task(xTaskHandle task)  { if (task) vTaskResume(task); }

/* ---------- BUS SATURATION ---------- */
static void bus_saturation_low(void) {
    printf("[FaultMgr] BUS LOW: Suspend logger\n");
    suspend_task(xLoggerTask);
}
static void bus_saturation_medium(void) {
    printf("[FaultMgr] BUS MEDIUM: Boost control task\n");
    set_task_priority(xControlTask, configMAX_PRIORITIES - 2);
}
static void bus_saturation_high(void) {
    printf("[FaultMgr] BUS HIGH: Shed load, suspend logger\n");
    suspend_task(xLoggerTask);
    extern void CAN_SetBitrate(uint32_t rate);
    CAN_SetBitrate(500000);
}

/* ---------- TIMING FAULT ---------- */
static void timing_fault_low(void) {
    printf("[FaultMgr] TIMING LOW: Boost control task\n");
    set_task_priority(xControlTask, configMAX_PRIORITIES - 1);
}
static void timing_fault_medium(void) {
    printf("[FaultMgr] TIMING MEDIUM: Extend control loop via queue\n");
    uint32_t new_period = 100;
    if (xControlQueue) xQueueSend(xControlQueue, &new_period, 0);
}
static void timing_fault_high(void) {
    printf("[FaultMgr] TIMING HIGH: Suspend logger, set CPU perf mode\n");
    suspend_task(xLoggerTask);
    extern void System_SetCPUPerformanceMode(void);
    System_SetCPUPerformanceMode();
}

/* ---------- MEMORY FRAGMENTATION ---------- */
static void memory_frag_low(void) {
    printf("[FaultMgr] MEM LOW: Free heap: %u bytes\n", xPortGetFreeHeapSize());
}
static void memory_frag_medium(void) {
    printf("[FaultMgr] MEM MEDIUM: Restart logger task\n");
    if (xLoggerTask) {
        vTaskDelete(xLoggerTask);
        extern void logger_task_func(void *);
        xTaskCreate(logger_task_func, (const signed char *)"Logger",
                    configMINIMAL_STACK_SIZE, NULL, 2, &xLoggerTask);
    }
}
static void memory_frag_high(void) {
    printf("[FaultMgr] MEM HIGH: Activate reserved pool, delete logger\n");
    
    use_reserved_pool = 1;
    if (xLoggerTask) vTaskDelete(xLoggerTask);
    xLoggerTask = NULL;
}

/* ---------- OS FAULT ---------- */
static void os_fault_low(void) {
    printf("[FaultMgr] OS LOW: Fan ON, log warning\n");
    extern void GPIO_SetFan(int);
    GPIO_SetFan(1);
}
static void os_fault_medium(void) {
    printf("[FaultMgr] OS MEDIUM: Save snapshot\n");
    extern void SaveFaultSnapshot(void);
    SaveFaultSnapshot();
}
static void os_fault_high(void) {
    printf("[FaultMgr] OS HIGH: Soft reset\n");
    extern void SystemSoftReset(void);
    vTaskDelay((2000 * configTICK_RATE_HZ) / 1000);
    SystemSoftReset();
}

/* ---------- Public API ---------- */
void fault_manager_init(xTaskHandle loggerTask) {
    last_bus_level = MITIGATION_NONE;
    last_timing_level = MITIGATION_NONE;
    last_mem_level = MITIGATION_NONE;
    last_os_level = MITIGATION_NONE;
    xLoggerTask = loggerTask;
    printf("[FaultMgr] Initialised\n");
}

void apply_mitigation(const FaultPrediction *pred) {
    MitigationLevel level;
    if (pred->risk_score > 80.0) level = MITIGATION_HIGH;
    else if (pred->risk_score > 50.0) level = MITIGATION_MEDIUM;
    else level = MITIGATION_LOW;

    switch (pred->fault_code) {
        case FAULT_BUS_SATURATION:
            if (level != last_bus_level) {
                if (level == MITIGATION_LOW) bus_saturation_low();
                else if (level == MITIGATION_MEDIUM) bus_saturation_medium();
                else if (level == MITIGATION_HIGH) bus_saturation_high();
                last_bus_level = level;
            }
            break;
        case FAULT_TIMING:
            if (level != last_timing_level) {
                if (level == MITIGATION_LOW) timing_fault_low();
                else if (level == MITIGATION_MEDIUM) timing_fault_medium();
                else if (level == MITIGATION_HIGH) timing_fault_high();
                last_timing_level = level;
            }
            break;
        case FAULT_MEM_FRAGMENTATION:
            if (level != last_mem_level) {
                if (level == MITIGATION_LOW) memory_frag_low();
                else if (level == MITIGATION_MEDIUM) memory_frag_medium();
                else if (level == MITIGATION_HIGH) memory_frag_high();
                last_mem_level = level;
            }
            break;
        case FAULT_OS:
            if (level != last_os_level) {
                if (level == MITIGATION_LOW) os_fault_low();
                else if (level == MITIGATION_MEDIUM) os_fault_medium();
                else if (level == MITIGATION_HIGH) os_fault_high();
                last_os_level = level;
            }
            break;
        default:
            last_bus_level = last_timing_level = last_mem_level = last_os_level = MITIGATION_NONE;
            break;
    }
}

void fault_manager_deinit(void) {
    set_task_priority(xControlTask, 3);
    resume_task(xLoggerTask);
    extern void GPIO_SetFan(int);
    GPIO_SetFan(0);
    extern void System_SetNormalPowerMode(void);
    System_SetNormalPowerMode();
}

/* ------ Stubs (replace with real implementations) ------ */
void CAN_SetBitrate(uint32_t rate) { (void)rate; }
void System_SetCPUPerformanceMode(void) {}
void System_SetNormalPowerMode(void) {}
void GPIO_SetFan(int on) { (void)on; }
void SaveFaultSnapshot(void) {}
void SystemSoftReset(void) { while(1); }
//void logger_task_func(void *pv) { (void)pv; for(;;) vTaskDelay(configTICK_RATE_HZ); }