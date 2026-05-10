
// #include <stdio.h>
// #include <stdint.h>
// #include <string.h>

// #define NUM_FEATURES 16
// #ifndef NUM_FAULT_CLASSES
// #define NUM_FAULT_CLASSES 5   /* -1, 0, 1, 2, 3 */
// #endif



// /* ------------------------------------------------------------------ */
// /*  Feature struct (order exactly matches training columns)           */
// /* ------------------------------------------------------------------ */
// typedef struct {
//     double cpu_load;
//     double free_heap;
//     double free_stack_task_1;
//     double free_stack_task_2;
//     double free_stack_task_3;
//     double free_stack_task_4;
//     double free_stack_task_5;
//     double task_overrun_last_cycle;
//     double max_overrun_window;
//     double msg_queue_len;
//     double sem_wait_max_us;
//     double wd_reset_count;
//     double temperature;
//     double missed_deadlines_1sec;
//     double preempt_count_1sec;
//     double cpu_idle_hits_10ms;
// } HealthSnapshot;

// /* ------------------------------------------------------------------ */
// /*  Output prediction struct                                          */
// /* ------------------------------------------------------------------ */
// typedef struct {
//     const char *likely_fault;           /* most probable fault name     */
//     int         fault_code;             /* numeric fault code           */
//     float       fault_probability;      /* probability of that fault    */
//     float       all_probabilities[5]; /* all probs     */
//     double      risk_score;             /* from linear regression (0-100) */
//     const char *criticality;            /* HIGH / MEDIUM / LOW         */
// } FaultPrediction;

// /* ------------------------------------------------------------------ */
// /*  External model functions                                          */
// /* ------------------------------------------------------------------ */

// /* From rtos_fault_model.c (emlearn Random Forest)                     */
// /* Predicts the fault class (int). Feature array: 16 floats.           */
// extern int32_t fault_model_predict(const float *features, int32_t length);

// /* Same model, returns probability per class (array of n_classes floats). */
// extern int32_t fault_model_predict_proba(const float *features, int32_t length,
//                                          float *scores, int32_t n_classes);

// /* From rtos_linear_regressor.c (m2cgen Linear Regression)             */
// extern double score(double *input);

// /* ------------------------------------------------------------------ */
// /*  Config                                                            */
// /* ------------------------------------------------------------------ */
