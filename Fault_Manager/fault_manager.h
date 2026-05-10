#ifndef FAULT_MANAGER_H
#define FAULT_MANAGER_H

#include <stdint.h>

#define NUM_FAULT_CLASSES 5   /* -1, 0, 1, 2, 3 */
#define NUM_FEATURES      16

/* Names for each fault class (must match your model.classes_ order) */
static const char *class_names[] = {
    "no fault",
    "bus saturation",
    "timing fault",
    "memory fragmentation",
    "OS fault"
};

/* Fault codes (as used by the model) */
#define FAULT_NONE               -1
#define FAULT_BUS_SATURATION      0
#define FAULT_TIMING              1
#define FAULT_MEM_FRAGMENTATION   2
#define FAULT_OS                  3

/* Output struct from the predictor – identical to the one in your predictor code */
typedef struct {
    const char *likely_fault;
    int         fault_code;
    float       fault_probability;
    float       all_probabilities[NUM_FAULT_CLASSES];
    double      risk_score;       /* 0‑100 from linear regression */
    const char *criticality;     /* "HIGH", "MEDIUM", "LOW" */
} FaultPrediction;

/* Initialise the fault manager (call once at startup) */
void fault_manager_init(void);

/* Apply appropriate mitigation for the given prediction.
   Safe to call periodically (every 10 ms) – internal state prevents repeated actions. */
void apply_mitigation(const FaultPrediction *pred);

/* Clean up and revert any persistent changes (e.g., restore priorities) */
void fault_manager_deinit(void);

#endif /* FAULT_MANAGER_H */