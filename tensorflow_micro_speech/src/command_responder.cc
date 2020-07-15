/* Copyright 2019 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#include "command_responder.h"
#include <zephyr.h>

#include <dk_buttons_and_leds.h>
#include <cstring>
/* The devicetree node identifier for the "led0" alias. */

// The default implementation writes out the name of the recognized command
// to the error console. Real applications will want to take some custom
// action instead, and should implement their own versions of this function.
void RespondToCommand(tflite::ErrorReporter *error_reporter,
                      int32_t current_time, const char *found_command,
                      uint8_t score, bool is_new_command)
{
  if (!strcmp(found_command, "silence"))
  {
    dk_set_led_on(DK_LED1);

    dk_set_led_off(DK_LED2);
    dk_set_led_off(DK_LED3);

  }
  else if (!strcmp(found_command, "unknown"))
  {
    dk_set_led_on(DK_LED2);
    dk_set_led_off(DK_LED1);

    dk_set_led_off(DK_LED3);

  }
  else if (!strcmp(found_command, "yes"))
  {
    dk_set_led_on(DK_LED3);
    dk_set_led_off(DK_LED1);
    dk_set_led_off(DK_LED2);

  }
  else if (!strcmp(found_command, "no"))
  {
    // dk_set_led_on(DK_LED4);
    // dk_set_led_off(DK_LED1);
    // dk_set_led_off(DK_LED2);
    // dk_set_led_off(DK_LED3);
  }
  if (is_new_command)
  {

    TF_LITE_REPORT_ERROR(error_reporter, "Heard %s (%d) @%dms", found_command,
                         score, current_time);
  }
}

void led_init(void)
{
  int err;

  err = dk_leds_init();
  if (err)
  {
    printk("LEDs init failed (err %d)\n", err);
    return;
  }
}
