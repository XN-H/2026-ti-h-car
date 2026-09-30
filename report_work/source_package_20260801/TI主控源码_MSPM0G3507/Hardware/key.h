#ifndef KEY_H
#define KEY_H

#include <stdbool.h>

typedef enum
{
    KEY_EVENT_NONE = 0,
    KEY_EVENT_SHORT,
    KEY_EVENT_LONG
} KeyEvent;

void Key_Init(void);
void Key_Update10ms(void);
KeyEvent Key_GetEvent(void);
bool Key_IsPressed(void);
bool Key_IsPressedRaw(void);

#endif