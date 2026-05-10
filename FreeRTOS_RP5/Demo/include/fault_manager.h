#ifndef FAULT_MANAGER_H
#define FAULT_MANAGER_H

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>

//#include "FreeRTOS.h"   // for xTaskHandle
//#include <task.h>

/* Fault codes */
typedef enum {
    FAULT_BUS_SATURATION = 0,
    FAULT_TIMING,
    FAULT_MEM_FRAGMENTATION,
    FAULT_OS,
    FAULT_NONE
} FaultCode;

/* Prediction structure */
typedef struct {
    FaultCode fault_code;
    float risk_score;      /* 0.0 to 100.0 */
} FaultPrediction;



/* Public API */
void fault_manager_init(xTaskHandle loggerTask);
void apply_mitigation(const FaultPrediction *pred);
void fault_manager_deinit(void);

#endif