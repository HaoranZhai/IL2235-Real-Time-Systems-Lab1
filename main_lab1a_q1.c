#include <stdio.h>
#include "bsp.h"
#include "workload.h"
#include <inttypes.h>

#define FRAME_LENGTH_MS 5
#define HYPER_PERIOD_MS 100
#define TOTAL_FRAMES (HYPER_PERIOD_MS / FRAME_LENGTH_MS)

static uint32_t frame_index = 0;
static repeating_timer_t frame_timer;

/*************************************************************
 * Frame callback: executes jobs according to cyclic schedule
 *************************************************************/
bool frame_callback(repeating_timer_t *tmr)
{
    jobReturn_t rA, rB, rC, rD, rE, rF;
    bool executed = false;

    printf("\n=== Frame %u === ", frame_index);

    switch (frame_index)
    {
    case 0:
        job_A(&rA);
        job_B(&rB);
        printf("A:%" PRIu64 "us B:%" PRIu64 "us\n", 
                rA.stop - rA.start, rB.stop - rB.start);
        executed = true;
        break;

    case 1:
        job_F(&rF);
        printf("F:%" PRIu64 "us\n", rF.stop - rF.start);
        executed = true;
        break;

    case 2:
        job_A(&rA);
        job_B(&rB);
        job_C(&rC);
        printf("A:%" PRIu64 "us B:%" PRIu64 "us C:%" PRIu64 "us\n",
                rA.stop - rA.start, rB.stop - rB.start, rC.stop - rC.start);
        executed = true;
        break;

    case 4:
        job_D(&rD);
        printf("D:%" PRIu64 "us\n", rD.stop - rD.start);
        executed = true;
        break;

    case 6:
        job_A(&rA);
        job_B(&rB);
        printf("A:%" PRIu64 "us B:%" PRIu64 "us\n", 
                rA.stop - rA.start, rB.stop - rB.start);
        executed = true;
        break;

    case 7:
        job_F(&rF);
        printf("F:%" PRIu64 "us\n", rF.stop - rF.start);
        executed = true;
        break;

    case 8:
        job_E(&rE);
        printf("E:%" PRIu64 "us\n", rE.stop - rE.start);
        executed = true;
        break;

    case 10:
        job_A(&rA);
        job_B(&rB);
        printf("A:%" PRIu64 "us B:%" PRIu64 "us\n", 
                rA.stop - rA.start, rB.stop - rB.start);
        executed = true;
        break;

    case 11:
        job_F(&rF);
        printf("F:%" PRIu64 "us\n", rF.stop - rF.start);
        executed = true;
        break;

    case 12:
        job_A(&rA);
        job_B(&rB);
        job_C(&rC);
        printf("A:%" PRIu64 "us B:%" PRIu64 "us C:%" PRIu64 "us\n",
                rA.stop - rA.start, rB.stop - rB.start, rC.stop - rC.start);
        executed = true;
        break;

    case 14:
        job_D(&rD);
        printf("D:%" PRIu64 "us\n", rD.stop - rD.start);
        executed = true;
        break;

    case 16:
        job_E(&rE);
        printf("E:%" PRIu64 "us\n", rE.stop - rE.start);
        executed = true;
        break;

    case 17:
        job_F(&rF);
        printf("F:%" PRIu64 "us\n", rF.stop - rF.start);
        executed = true;
        break;

    case 18:
        job_A(&rA);
        job_B(&rB);
        printf("A:%" PRIu64 "us B:%" PRIu64 "us\n", 
                rA.stop - rA.start, rB.stop - rB.start);
        executed = true;
        break;

    default:
        printf("(No task)\n");
        break;
    }

    if (executed)
        BSP_ToggleLED(LED_GREEN);  // 点灯表示任务执行成功

    frame_index = (frame_index + 1) % TOTAL_FRAMES;
    return true;
}

/*************************************************************
 * Main function
 *************************************************************/
int main(void)
{
    BSP_Init();
    printf("Starting Cyclic Scheduler (compact output)...\n");

    add_repeating_timer_ms(FRAME_LENGTH_MS, frame_callback, NULL, &frame_timer);

    while (true)
    {
        sleep_ms(1000); // 主循环空转
    }
}
