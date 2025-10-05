#ifndef HAL_INPUT_H
#define HAL_INPUT_H

#include <stdint.h>
#include "MacTypes.h"

void hal_input_poll(void);
void hal_input_get_mouse(Point* mouse_loc);
Boolean hal_input_get_keyboard_state(KeyMap key_map);

#endif /* HAL_INPUT_H */
