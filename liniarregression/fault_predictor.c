#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* ------------------------------------------------------------------ */
/*  External model functions                                          */
/* ------------------------------------------------------------------ */

/* From rtos_fault_model.c (emlearn Random Forest)                     */
/* Predicts the fault class (int). Feature array: 16 floats.           */
extern int32_t fault_model_predict(const float *features, int32_t length);

/* Same model, returns probability per class (array of n_classes floats). */
extern int32_t fault_model_predict_proba(const float *features, int32_t length,
                                         float *scores, int32_t n_classes);

/* From rtos_linear_regressor.c (m2cgen Linear Regression)             */
extern double score(double *input);

/* ------------------------------------------------------------------ */
/*  Config                                                            */
/* ------------------------------------------------------------------ */
#define NUM_FEATURES 16
#define NUM_FAULT_CLASSES 5   /* -1, 0, 1, 2, 3 */

/* Fault class order (must match your model.classes_ after training)   */
static const int class_order[] = {-1, 0, 1, 2, 3};

/* Human‑readable names for each fault class */
static const char *class_names[] = {
    "no fault",
    "bus saturation",
    "timing fault",
    "memory fragmentation",
    "OS fault"
};

/* ------------------------------------------------------------------ */
/*  Feature struct (order exactly matches training columns)           */
/* ------------------------------------------------------------------ */
typedef struct {
    double cpu_load;
    double free_heap;
    double free_stack_task_1;
    double free_stack_task_2;
    double free_stack_task_3;
    double free_stack_task_4;
    double free_stack_task_5;
    double task_overrun_last_cycle;
    double max_overrun_window;
    double msg_queue_len;
    double sem_wait_max_us;
    double wd_reset_count;
    double temperature;
    double missed_deadlines_1sec;
    double preempt_count_1sec;
    double cpu_idle_hits_10ms;
} HealthSnapshot;

/* ------------------------------------------------------------------ */
/*  Output prediction struct                                          */
/* ------------------------------------------------------------------ */
typedef struct {
    const char *likely_fault;           /* most probable fault name     */
    int         fault_code;             /* numeric fault code           */
    float       fault_probability;      /* probability of that fault    */
    float       all_probabilities[NUM_FAULT_CLASSES]; /* all probs     */
    double      risk_score;             /* from linear regression (0-100) */
    const char *criticality;            /* HIGH / MEDIUM / LOW         */
} FaultPrediction;

/* ------------------------------------------------------------------ */
/*  Convert snapshot to double feature array                          */
/* ------------------------------------------------------------------ */
void snapshot_to_features(const HealthSnapshot *snap, double features[NUM_FEATURES])
{
    features[0]  = snap->cpu_load;
    features[1]  = snap->free_heap;
    features[2]  = snap->free_stack_task_1;
    features[3]  = snap->free_stack_task_2;
    features[4]  = snap->free_stack_task_3;
    features[5]  = snap->free_stack_task_4;
    features[6]  = snap->free_stack_task_5;
    features[7]  = snap->task_overrun_last_cycle;
    features[8]  = snap->max_overrun_window;
    features[9]  = snap->msg_queue_len;
    features[10] = snap->sem_wait_max_us;
    features[11] = snap->wd_reset_count;
    features[12] = snap->temperature;
    features[13] = snap->missed_deadlines_1sec;
    features[14] = snap->preempt_count_1sec;
    features[15] = snap->cpu_idle_hits_10ms;
}

/* ------------------------------------------------------------------ */
/*  Dual‑model prediction: fault type + risk score + criticality     */
/* ------------------------------------------------------------------ */
FaultPrediction predict_fault_and_risk(const HealthSnapshot *snap)
{
    FaultPrediction result;
    memset(&result, 0, sizeof(result));

    /* 1) Prepare features as double (for regression) and float (for forest) */
    double features_double[NUM_FEATURES];
    float  features_float [NUM_FEATURES];
    snapshot_to_features(snap, features_double);
    for (int i = 0; i < NUM_FEATURES; i++)
        features_float[i] = (float)features_double[i];

    /* 2) Random Forest – get probabilities for each fault class */
    float scores[NUM_FAULT_CLASSES];
    fault_model_predict_proba(features_float, NUM_FEATURES,
                              scores, NUM_FAULT_CLASSES);

    /* Find the most likely class */
    int max_idx = 0;
    for (int i = 1; i < NUM_FAULT_CLASSES; i++) {
        if (scores[i] > scores[max_idx])
            max_idx = i;
    }

    result.fault_code       = class_order[max_idx];
    result.likely_fault     = class_names[max_idx];
    result.fault_probability = scores[max_idx];
    memcpy(result.all_probabilities, scores, sizeof(scores));

    /* 3) Linear regression – risk score (0‑100) */
    double risk = score(features_double);
    if (risk < 0.0)   risk = 0.0;
    if (risk > 100.0) risk = 100.0;
    result.risk_score = risk;

    /* 4) Criticality from risk score */
    if (risk > 80.0)
        result.criticality = "HIGH";
    else if (risk > 50.0)
        result.criticality = "MEDIUM";
    else
        result.criticality = "LOW";

    return result;
}

/* ------------------------------------------------------------------ */
/*  Main: periodic loop example                                       */
/* ------------------------------------------------------------------ */
int main(void)
{
    /* Example snapshot – replace with real data acquisition */
    HealthSnapshot snap = {
        .cpu_load                = 45.2,
        .free_heap               = 850000,
        .free_stack_task_1       = 1200,
        .free_stack_task_2       = 1100,
        .free_stack_task_3       = 980,
        .free_stack_task_4       = 2000,
        .free_stack_task_5       = 1500,
        .task_overrun_last_cycle = 0,
        .max_overrun_window      = 0,
        .msg_queue_len           = 2,
        .sem_wait_max_us         = 200,
        .wd_reset_count          = 0,
        .temperature             = 48.0,
        .missed_deadlines_1sec   = 0,
        .preempt_count_1sec      = 4,
        .cpu_idle_hits_10ms      = 1
    };

    /* Run prediction */
    FaultPrediction pred = predict_fault_and_risk(&snap);

    /* Print structured output */
    printf("====================== Fault Prediction ======================\n");
    printf("Likely fault:      %s (code %d)\n", pred.likely_fault, pred.fault_code);
    printf("Fault probability: %.4f\n", pred.fault_probability);
    printf("Risk score:        %.2f / 100\n", pred.risk_score);
    printf("Criticality:       %s\n", pred.criticality);
    printf("All probabilities:\n");
    for (int i = 0; i < NUM_FAULT_CLASSES; i++) {
        printf("  %-20s: %.4f\n", class_names[i], pred.all_probabilities[i]);
    }
    printf("===============================================================\n");

    /* ---- In your real loop ---- */
    /*
    while (1) {
        // read snapshot from shared memory / driver
        FaultPrediction pred = predict_fault_and_risk(&snap);
        // send prediction over socket, log, or take action
        // sleep 10 ms
    }
    */

    return 0;
}