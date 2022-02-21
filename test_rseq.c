#define _GNU_SOURCE
#include <assert.h>
#include <immintrin.h>
#include <rseq/rseq.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <x86intrin.h>


#define BENCH_FUNC __attribute__((noinline, noclone, aligned(4096)))

#define COMPILER_DO_NOT_OPTIMIZE_OUT_tput(X)                                   \
    asm volatile("" : : "i,r,m"(X) : "memory")
#define COMPILER_DO_NOT_OPTIMIZE_OUT_lat(X)                                    \
    asm volatile("lfence" : : "i,r,m"(X) : "memory")

#define _CAT(X, Y) X##Y
#define CAT(X, Y)  _CAT(X, Y)


static uint64_t
to_ns(struct timespec * ts) {
    return (1000UL * 1000UL * 1000UL * ts->tv_sec + ts->tv_nsec);
}

static uint64_t
dif_ns(struct timespec * ts0, struct timespec * ts1) {
    return to_ns(ts0) - to_ns(ts1);
}

static void
gettime(struct timespec * ts) {
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, ts);
}
#define TRIALS (300 * 1000 * 1000)

#define _make_bench(func, type)                                                \
    uint64_t BENCH_FUNC CAT(CAT(func, _bench_), type)() {                      \
        struct timespec ts[2];                                                 \
        gettime(ts);                                                           \
        for (uint32_t trials = TRIALS; trials; --trials) {                     \
            CAT(COMPILER_DO_NOT_OPTIMIZE_OUT_, type)(func());                  \
        }                                                                      \
        gettime(ts + 1);                                                       \
                                                                               \
        return dif_ns(ts + 1, ts);                                             \
    }

#define make_bench(func)                                                       \
    _make_bench(func, lat);                                                    \
    _make_bench(func, tput)

make_bench(__builtin_ia32_rdpid);
make_bench(rseq_current_cpu);
make_bench(rseq_current_cpu_raw);

int
main(int argc, char ** argv) {

    printf("%lf\n", ((double)rseq_current_cpu_bench_tput()) / ((double)TRIALS));
    printf("%lf\n",
           ((double)rseq_current_cpu_raw_bench_tput()) / ((double)TRIALS));

    printf("%lf\n",
           ((double)__builtin_ia32_rdpid_bench_tput()) / ((double)TRIALS));
#ifdef LAT
    printf("%lf\n", ((double)rseq_current_cpu_bench_lat()) / ((double)TRIALS));
    printf("%lf\n",
           ((double)__builtin_ia32_rdpid_bench_lat()) / ((double)TRIALS));
    printf("%lf\n",
           ((double)rseq_current_cpu_raw_bench_lat()) / ((double)TRIALS));
#endif
}
