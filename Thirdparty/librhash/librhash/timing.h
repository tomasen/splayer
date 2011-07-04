/* timing.h */
#ifndef TIMING_H
#define TIMING_H

#ifndef _WIN32
#include <sys/time.h> /* for timeval */
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* timedelta_t and timing interface */
#ifdef _WIN32
typedef unsigned long long timedelta_t;
#else
typedef struct timeval timedelta_t;
#endif

/* timer functions */
void timer_start(timedelta_t* timer);
double timer_stop(timedelta_t* timer);

/* flags for running a benchmark */
#define BENCHMARK_QUIET 1
#define BENCHMARK_CPB 2
#define BENCHMARK_RAW 4

void run_benchmark(unsigned hash_id, unsigned flags, FILE* output);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* TIMING_H */
