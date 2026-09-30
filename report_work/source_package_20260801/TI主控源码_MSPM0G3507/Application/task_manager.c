#include "task_manager.h"
#include "key.h"
#include "oled.h"
#include "encoder_pid.h"
#include "task_1.h"
#include "task_2.h"
#include "motion.h"
#include "task_3.h"

static AppTaskId selected_task = APP_TASK_1;
static AppState app_state = APP_STATE_SELECT;
static uint32_t task_start_tick = 0U;
static uint32_t task_elapsed_ticks = 0U;

static bool task_timer_running = false;




//时间内部函数文件
static void TaskTimer_Start(uint32_t current_tick)
{
    task_start_tick = current_tick;
    task_elapsed_ticks = 0U;
    task_timer_running = true;
}

static void TaskTimer_Update(uint32_t current_tick)
{
    if (task_timer_running)
    {
        task_elapsed_ticks =
            (uint32_t)(current_tick - task_start_tick);
    }
}

static void TaskTimer_Stop(uint32_t current_tick)
{
    if (!task_timer_running)
    {
        return;
    }

    TaskTimer_Update(current_tick);
    task_timer_running = false;
}
static void TaskManager_Finish(uint32_t current_tick)
{
    if (app_state != APP_STATE_RUNNING)
    {
        return;
    }

    

    switch (selected_task)
    {
        case APP_TASK_1:
            Task1_Exit();
            break;

        case APP_TASK_2:
            Task2_Exit();
            break;
				case APP_TASK_3:
						Task3_Exit();
						break;

        default:
            break;
    }

			/*
			 * 先将PI控制器内部状态清零。
			 * 这个函数最后会暂时进入滑行状态。
			 */
			CarSpeedPIDSet(0, 0);

			/*
			 * 再发送短路制动命令。
			 * 必须最后调用，否则会被SpeedStopA/B覆盖。
			 */
			CarBrake();
			TaskTimer_Stop(current_tick);

			app_state = APP_STATE_FINISHED;
}


static const char *task_names[APP_TASK_COUNT] =
{
    "TASK 1",
    "TASK 2",
    "TASK 3",
    "TASK 4"
};

static void TaskManager_ShowText(
    uint8_t x,
    uint8_t y,
    const char *text)
{
    OLED_ShowString(
        x,
        y,
        (const uint8_t *)text);
}
void TaskManager_Init(void)
{
    selected_task = APP_TASK_1;
    app_state = APP_STATE_SELECT;
		task_timer_running = false;

    Key_Init();
    CarSpeedPIDSet(0, 0);
}
void TaskManager_Update10ms(uint32_t current_tick)
{
    KeyEvent key_event;

    Key_Update10ms();
    key_event = Key_GetEvent();

    if (app_state == APP_STATE_SELECT)
    {
        if (key_event == KEY_EVENT_SHORT)
        {
            selected_task = (AppTaskId)(
                (selected_task + 1) %
                APP_TASK_COUNT);
        }
				else if (key_event == KEY_EVENT_LONG)
				{
						TaskTimer_Start(current_tick);

						switch (selected_task)
						{
								case APP_TASK_1:
										Task1_Enter();
										break;

								case APP_TASK_2:
										Task2_Enter();
										break;
								
								case APP_TASK_3:
										Task3_Enter();
										break;

								default:
										break;
						}

						app_state = APP_STATE_RUNNING;
				}
    }
				else if (app_state == APP_STATE_RUNNING)
				{
						if (key_event != KEY_EVENT_NONE)
						{
								TaskManager_Finish(current_tick);
						}
				}
				else
				{
						if (key_event != KEY_EVENT_NONE)
						{
								CarStop();
								app_state = APP_STATE_SELECT;
						}
				}
}


void TaskManager_Run(uint32_t current_tick)
{
    if (app_state != APP_STATE_RUNNING)
    {
        return;
    }


		TaskTimer_Update(current_tick);

    switch (selected_task)
    {
				case APP_TASK_1:
						if (Task1_Run(current_tick))
						{
								TaskManager_Finish(current_tick);
						}
						break;

				case APP_TASK_2:
				{
						Task2RunResult task2_result =
								Task2_Run(current_tick);

						if (task2_result == TASK2_RESULT_FINISHED)
						{
								TaskManager_Finish(current_tick);
						}

						break;
				}
				
				
				case APP_TASK_3:
				{
						Task3RunResult task3_result =
								Task3_Run(current_tick);

						if (task3_result == TASK3_RESULT_FINISHED)
						{
								TaskManager_Finish(current_tick);
						}

						break;
				}
        case APP_TASK_4:
        default:
            break;
    }
}


static void TaskManager_DrawTime(uint8_t y)
{
    uint32_t seconds = task_elapsed_ticks / 100U;
    uint32_t centiseconds = task_elapsed_ticks % 100U;

    TaskManager_ShowText(0, y, "TIME:");
    OLED_ShowNumber(40, y, seconds, 3, 12);
    OLED_ShowChar(64, y, '.', 12, 1);
    OLED_ShowChar(72, y, '0' + centiseconds / 10U, 12, 1);
    OLED_ShowChar(80, y, '0' + centiseconds % 10U, 12, 1);
}

void TaskManager_Draw100ms(void)
{
    uint8_t i;

    OLED_Clear();

    if (app_state == APP_STATE_SELECT)
    {
        for (i = 0U; i < APP_TASK_COUNT; i++)
        {
            OLED_ShowChar(
                0,
                i * 16U,
                (i == selected_task) ? '>' : ' ',
                12,
                1);

            TaskManager_ShowText(
                16,
                i * 16U,
                task_names[i]);
        }
    }
			else if (app_state == APP_STATE_FINISHED)
			{
					TaskManager_ShowText(0, 0, task_names[selected_task]);
					TaskManager_ShowText(0, 16, "FINISHED");
					TaskManager_DrawTime(32);
					TaskManager_ShowText(0, 48, "PRESS: MENU");
			}
			else
			{
					switch (selected_task)
					{
							case APP_TASK_1:
									Task1_Draw();
									break;

							case APP_TASK_2:
									Task2_Draw();
									break;
							case APP_TASK_3:
									Task3_Draw();
									break;

							default:
									TaskManager_ShowText(0, 16, "RUNNING");
									TaskManager_ShowText(0, 32, task_names[selected_task]);
									break;
					}

					/* 最后绘制，让时间覆盖在第一行 */
					TaskManager_DrawTime(0);
			}
			OLED_Refresh_Gram();
}
AppTaskId TaskManager_GetSelectedTask(void)
{
    return selected_task;
}

AppState TaskManager_GetState(void)
{
    return app_state;
}

