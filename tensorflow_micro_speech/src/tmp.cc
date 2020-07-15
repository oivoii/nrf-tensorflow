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


namespace {
bool g_is_audio_initialized = false;
// An internal buffer able to fit 16x our sample size
constexpr int kAudioCaptureBufferSize = DEFAULT_PDM_BUFFER_SIZE * 16;
int16_t g_audio_capture_buffer[kAudioCaptureBufferSize];
// A buffer that holds our output
int16_t g_audio_output_buffer[kMaxAudioSampleSize];
// Mark as volatile so we can check in a while loop to see if
// any samples have arrived yet.
volatile int32_t g_latest_audio_timestamp = 0;
}  // namespace

void CaptureSamples() {
  // This is how many bytes of new data we have each time this is called
  const int number_of_samples = DEFAULT_PDM_BUFFER_SIZE;
  // Calculate what timestamp the last audio sample represents
  const int32_t time_in_ms =
      g_latest_audio_timestamp +
      (number_of_samples / (kAudioSampleFrequency / 1000));
  // Determine the index, in the history of all samples, of the last sample
  const int32_t start_sample_offset =
      g_latest_audio_timestamp * (kAudioSampleFrequency / 1000);
  // Determine the index of this sample in our ring buffer
  const int capture_index = start_sample_offset % kAudioCaptureBufferSize;
  // Read the data to the correct place in our buffer
  PDM.read(g_audio_capture_buffer + capture_index, DEFAULT_PDM_BUFFER_SIZE);
  // This is how we let the outside world know that new audio data has arrived.
  g_latest_audio_timestamp = time_in_ms;
}

TfLiteStatus InitAudioRecording(tflite::ErrorReporter* error_reporter) {
  // Hook up the callback that will be called with each sample
  PDM.onReceive(CaptureSamples);
  // Start listening for audio: MONO @ 16KHz with gain at 20
  PDM.begin(1, kAudioSampleFrequency);
  PDM.setGain(20);
  // Block until we have our first audio sample
  while (!g_latest_audio_timestamp) {
  }

  return kTfLiteOk;
}

TfLiteStatus GetAudioSamples(tflite::ErrorReporter* error_reporter,
                             int start_ms, int duration_ms,
                             int* audio_samples_size, int16_t** audio_samples) {
  // Set everything up to start receiving audio
  if (!g_is_audio_initialized) {
    TfLiteStatus init_status = InitAudioRecording(error_reporter);
    if (init_status != kTfLiteOk) {
      return init_status;
    }
    g_is_audio_initialized = true;
  }
  // This next part should only be called when the main thread notices that the
  // latest audio sample data timestamp has changed, so that there's new data
  // in the capture ring buffer. The ring buffer will eventually wrap around and
  // overwrite the data, but the assumption is that the main thread is checking
  // often enough and the buffer is large enough that this call will be made
  // before that happens.

  // Determine the index, in the history of all samples, of the first
  // sample we want
  const int start_offset = start_ms * (kAudioSampleFrequency / 1000);
  // Determine how many samples we want in total
  const int duration_sample_count =
      duration_ms * (kAudioSampleFrequency / 1000);
  for (int i = 0; i < duration_sample_count; ++i) {
    // For each sample, transform its index in the history of all samples into
    // its index in g_audio_capture_buffer
    const int capture_index = (start_offset + i) % kAudioCaptureBufferSize;
    // Write the sample to the output buffer
    g_audio_output_buffer[i] = g_audio_capture_buffer[capture_index];
  }

  // Set pointers to provide access to the audio
  *audio_samples_size = kMaxAudioSampleSize;
  *audio_samples = g_audio_output_buffer;

  return kTfLiteOk;
}

int32_t LatestAudioTimestamp() { return g_latest_audio_timestamp; }