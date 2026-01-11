/**
 * @file workload.c
 * @author Matthias Becker
 * @brief Implements the workload used with switch-controlled Task C
 * @version 0.2
 * @date 2025-11-10
 */

#include <stdio.h>
#include <inttypes.h>
#include "bsp.h"
#include "workload.h"

/*-----------------------------------------------------------*/
void job_A(jobReturn_t* retval) {
    retval->start = time_us_64();
    BSP_WaitClkCycles(EXECUTION_TIME_A);
    retval->stop = time_us_64();
}
/*-----------------------------------------------------------*/
void job_B(jobReturn_t* retval) {
    retval->start = time_us_64();
    BSP_WaitClkCycles(EXECUTION_TIME_B);
    retval->stop = time_us_64();
}
/*-----------------------------------------------------------*/

/**
 * @brief Task C — Execution time dynamically controlled by 8 DIP switches
 *
 * Each switch contributes additional workload.
 * Switch value (0–255) × 0.1 ms added to the base execution time.
 */
void job_C(jobReturn_t* retval) {
    retval->start = time_us_64();

    // ---------------------------
    // 读取 8 位拨码开关的值
    // ---------------------------
    uint32_t switch_value = 0;
    for (int i = 0; i < 8; i++) {
        if (BSP_GetInput(SW_10+i)) {
            switch_value |= (1 << i);
        }
    }

    // ---------------------------
    // 计算动态等待时间
    // 每个单位延长 0.5 ms（你可以调节这个系数）
    // ---------------------------
    uint64_t dynamic_cycles = EXECUTION_TIME_C + switch_value * (CYCLES_PER_MS / 2); 
    // (1/20 ms = 0.05ms per switch increment)

    // ---------------------------
    // 执行对应的忙等待
    // ---------------------------
    BSP_WaitClkCycles(dynamic_cycles);

    retval->stop = time_us_64();
}

/*-----------------------------------------------------------*/
void job_D(jobReturn_t* retval) {
    retval->start = time_us_64();
    BSP_WaitClkCycles(EXECUTION_TIME_D);
    retval->stop = time_us_64();
}
/*-----------------------------------------------------------*/
void job_E(jobReturn_t* retval) {
    retval->start = time_us_64();
    BSP_WaitClkCycles(EXECUTION_TIME_E);
    retval->stop = time_us_64();
}
/*-----------------------------------------------------------*/
void job_F(jobReturn_t* retval) {
    retval->start = time_us_64();
    BSP_WaitClkCycles(EXECUTION_TIME_F);
    retval->stop = time_us_64();
}
/*-----------------------------------------------------------*/
