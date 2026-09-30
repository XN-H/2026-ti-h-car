#include "ti_msp_dl_config.h"
#include "key.h"
#define KEY_DEBOUNCE_TICKS      3U
#define KEY_LONG_PRESS_TICKS   150U

static bool stable_pressed = false;
static bool long_press_detected = false;

static uint8_t debounce_ticks = 0U;
static uint16_t pressed_ticks = 0U;

static KeyEvent pending_event = KEY_EVENT_NONE;

bool Key_IsPressedRaw(void)
{
    return DL_GPIO_readPins(
        KEY_PORT,
        KEY_BUTTON_PIN) == 0U;
}

void Key_Init(void)
{
    stable_pressed = false;
		long_press_detected = false;
    debounce_ticks = 0U;
    pressed_ticks = 0U;
    pending_event = KEY_EVENT_NONE;
}

void Key_Update10ms(void)
{
    bool raw_pressed = Key_IsPressedRaw();

    if (raw_pressed != stable_pressed)
    {
        debounce_ticks++;

        if (debounce_ticks >= KEY_DEBOUNCE_TICKS)
        {
            stable_pressed = raw_pressed;
            debounce_ticks = 0U;

						if (stable_pressed)
						{
								/* 刚刚确认按键按下，开始一次新的计时 */
								pressed_ticks = 0U;
								long_press_detected = false;
						}
						else
						{
								/* 刚刚确认按键松开，此时才产生按键事件 */
								if (long_press_detected)
								{
										pending_event = KEY_EVENT_LONG;
								}
								else
								{
										pending_event = KEY_EVENT_SHORT;
								}
						}
        }
    }
    else
    {
        debounce_ticks = 0U;
    }

		if (stable_pressed && !long_press_detected)
		{
				if (pressed_ticks < KEY_LONG_PRESS_TICKS)
				{
						pressed_ticks++;
				}

				if (pressed_ticks >= KEY_LONG_PRESS_TICKS)
				{
						long_press_detected = true;
				}
		}
}
KeyEvent Key_GetEvent(void)
{
    KeyEvent event = pending_event;
    pending_event = KEY_EVENT_NONE;
    return event;
}

bool Key_IsPressed(void)
{
    return stable_pressed;
}