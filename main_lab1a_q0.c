#include <stdio.h>
#include <inttypes.h>     // ✅ 必须放在 stdio.h 之后
#include "bsp.h"
#include "workload.h"

int main(void) {
    BSP_Init();
    jobReturn_t r;

    printf("Measuring task execution times:\n");

    job_A(&r);
    printf("A: %" PRIu64 " us\n", r.stop - r.start);

    job_B(&r);
    printf("B: %" PRIu64 " us\n", r.stop - r.start);

    job_C(&r);
    printf("C: %" PRIu64 " us\n", r.stop - r.start);

    job_D(&r);
    printf("D: %" PRIu64 " us\n", r.stop - r.start);

    job_E(&r);
    printf("E: %" PRIu64 " us\n", r.stop - r.start);

    job_F(&r);
    printf("F: %" PRIu64 " us\n", r.stop - r.start);

    while (1);
}
