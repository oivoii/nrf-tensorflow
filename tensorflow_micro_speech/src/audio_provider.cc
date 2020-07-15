/* Copyright 2018 The TensorFlow Authors. All Rights Reserved.

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

#include "audio_provider.h"
#include "micro_features/micro_model_settings.h"

/*----------- audio input functions start ----------------- */
#include <stdint.h>
#include <string.h>
#include <zephyr.h>

#include "nrfx_i2s.h"
#include <dk_buttons_and_leds.h>

#define I2S_WS_PIN 4
#define I2S_SD_PIN 5
#define I2S_SCK_PIN 6

#define I2S_DATA_BLOCK_WORDS 16129 //How many samples we want to record, and thus how long.

static u32_t m_buffer_rx32u[I2S_DATA_BLOCK_WORDS];
static u16_t m_buffer_rx16u[I2S_DATA_BLOCK_WORDS];
static int16_t m_buffer_rx16i[I2S_DATA_BLOCK_WORDS];
static nrfx_i2s_buffers_t initial_buffers;
static int counter = 0;
static bool data_ready_flag = false;

ISR_DIRECT_DECLARE(i2s_isr_handler)
{
  nrfx_i2s_irq_handler();
  ISR_DIRECT_PM(); /* PM done after servicing interrupt for best latency
			  */
  return 1;        /* We should check if scheduling decision should be made */
}

static void data_handler(nrfx_i2s_buffers_t const *p_released, u32_t status)
{
  counter++;
  if (NRFX_I2S_STATUS_NEXT_BUFFERS_NEEDED == status)
  {
    nrfx_err_t err = nrfx_i2s_next_buffers_set(&initial_buffers);
    if (err != NRFX_SUCCESS)
    { // TODO: Handle me
      printk("ERROR!, continuing running as if nothing happened.\n");
    }
  }
  if (p_released)
  {
    if (p_released->p_rx_buffer != NULL)
    {
      data_ready_flag = true;
    }
  }
}

static int mic_start_collecting_sound_level()
{
  memset(&m_buffer_rx32u, 0x00, sizeof(m_buffer_rx32u));
  memset(&m_buffer_rx16i, 0x00, sizeof(m_buffer_rx16i));
  initial_buffers.p_rx_buffer = m_buffer_rx32u;

  nrfx_i2s_config_t config =
      NRFX_I2S_DEFAULT_CONFIG(I2S_SCK_PIN, I2S_WS_PIN,
                              NRFX_I2S_PIN_NOT_USED,
                              NRFX_I2S_PIN_NOT_USED, I2S_SD_PIN);
  config.mode = NRF_I2S_MODE_MASTER;
  config.mck_setup = NRF_I2S_MCK_32MDIV31;
  config.channels = NRF_I2S_CHANNELS_LEFT;
  config.sample_width = NRF_I2S_SWIDTH_24BIT_IN32BIT;
  config.ratio = NRF_I2S_RATIO_64X;

  nrfx_err_t err_code = nrfx_i2s_init(&config, data_handler);
  if (err_code != NRFX_SUCCESS)
  {
    printk("I2S init error\n");
    k_panic();
  }
  return 0;
}

void get_sound_init()
{
  IRQ_DIRECT_CONNECT(I2S0_IRQn, 0, i2s_isr_handler, 0);
  int err = mic_start_collecting_sound_level();
  if (err != 0)
  {
    printk("Fail: %d", err);
  }
}
void get_sound()
{
  dk_set_led_on(DK_LED4);
  u16_t lowest = -1;

  nrfx_i2s_start(&initial_buffers, I2S_DATA_BLOCK_WORDS, 0);
  //printk("Counter: %d\n", counter);
  while (!data_ready_flag)
  {
    k_sleep(K_MSEC(5));
    //Wait for data.
  }
  k_sleep(K_SECONDS(1));
  nrfx_i2s_stop();
  data_ready_flag = false;
  for (int i = 0; i < I2S_DATA_BLOCK_WORDS; i++)
  {
    m_buffer_rx16u[i] = (u16_t)((m_buffer_rx32u[i] << 14) >> 16);
    if (m_buffer_rx16u[i] <= lowest)
    {
      lowest = m_buffer_rx16u[i];
    }
  }
  u32_t sum = 0;
  for (int i = 0; i < I2S_DATA_BLOCK_WORDS; i++)
  {
    m_buffer_rx16u[i] = m_buffer_rx16u[i] - lowest;
    sum += m_buffer_rx16u[i];
  }
  sum = sum / I2S_DATA_BLOCK_WORDS;

  for (int i = 0; i < I2S_DATA_BLOCK_WORDS; i++)
  {
    m_buffer_rx16i[i] = m_buffer_rx16u[i] - sum;
  }
  dk_set_led_off(DK_LED4);
}

/*----------- audio input functions end ----------------- */

namespace
{
  int32_t g_latest_audio_timestamp = 0;
} // namespace

TfLiteStatus GetAudioSamples(tflite::ErrorReporter *error_reporter,
                             int start_ms, int duration_ms,
                             int *audio_samples_size, int16_t **audio_samples)
{
  
  get_sound();

  *audio_samples = m_buffer_rx16i;
  g_latest_audio_timestamp += 100;
  *audio_samples_size = kMaxAudioSampleSize;
  return kTfLiteOk;
}

int32_t LatestAudioTimestamp()
{
  return g_latest_audio_timestamp;
}
