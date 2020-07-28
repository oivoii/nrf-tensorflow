

#include <zephyr/types.h>
#include <string.h>
#include <zephyr.h>

#include "nrfx_i2s.h"
#include "i2s.h"

#ifdef __cplusplus
extern "C" {
#endif

static u32_t m_buffer_rx32u[I2S_DATA_BLOCK_WORDS];
static s32_t m_buffer_rx32s[I2S_DATA_BLOCK_WORDS];
static s16_t m_buffer_rx16s[I2S_DATA_BLOCK_WORDS];
static nrfx_i2s_buffers_t initial_buffers;
static int counter = 0;

void (*external_handler)(void);

ISR_DIRECT_DECLARE(i2s_isr_handler)
{
	nrfx_i2s_irq_handler();
	ISR_DIRECT_PM(); /* PM done after servicing interrupt for best latency
			  */
	return 1; /* We should check if scheduling decision should be made */
}

static void data_handler(nrfx_i2s_buffers_t const *p_released, u32_t status)
{
	counter++;
	if (NRFX_I2S_STATUS_NEXT_BUFFERS_NEEDED == status) {
		nrfx_err_t err = nrfx_i2s_next_buffers_set(&initial_buffers);
		if (err != NRFX_SUCCESS) { // TODO: Handle me
			printk("I2S ERROR!, continuing running as if nothing happened.\n");
		}
	}
	if (p_released) {
		if (p_released->p_rx_buffer != NULL) {
			external_handler();
		}
	}
}

static int mic_start_collecting_sound_level()
{
	memset(&m_buffer_rx32u, 0x00, sizeof(m_buffer_rx32u));
	memset(&m_buffer_rx32s, 0x00, sizeof(m_buffer_rx32s));
	memset(&m_buffer_rx16s, 0x00, sizeof(m_buffer_rx16s));
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
	if (err_code != NRFX_SUCCESS) {
		printk("I2S init error\n");
		k_panic();
	}
	nrfx_i2s_start(&initial_buffers, I2S_DATA_BLOCK_WORDS, 0);
	//k_sleep(K_SECONDS(1));

	return 0;
}

void get_sound_init(void (*handler)() )
{
	external_handler = handler;
	IRQ_DIRECT_CONNECT(I2S0_IRQn, 0, i2s_isr_handler, 0);
	int err = mic_start_collecting_sound_level();
	if (err != 0) {
		printk("I2S Fail: %d", err);
	}
}
void filter_sound(s16_t my_buffer[I2S_DATA_BLOCK_WORDS], s16_t numerator = 1, s16_t denominator = 1)
{
	if (numerator == denominator){
		return;
	}
	s16_t filter_input;
	s16_t current_filter_output = 0;
	for (int i = 0; i < I2S_DATA_BLOCK_WORDS; i++)
	{
		filter_input = my_buffer[i];
		current_filter_output = current_filter_output \
								- (current_filter_output * numerator / denominator) \
								+ (filter_input  * numerator / denominator);
		my_buffer[i] = current_filter_output;
	}
	return;
}
void get_sound(void* buffer, size_t size)
{
	s64_t buf_sum = 0;
	s64_t buf_mean = 0;
	s64_t words = I2S_DATA_BLOCK_WORDS;

	for (int i = 0; i < I2S_DATA_BLOCK_WORDS; i++) 
	{
		memcpy(m_buffer_rx32s + i,m_buffer_rx32u + i, sizeof(u32_t));
		m_buffer_rx32s[i] >>= 8;
		buf_sum += m_buffer_rx32s[i];
	}
	buf_mean = buf_sum/words;
	for (int i = 0; i < I2S_DATA_BLOCK_WORDS; i++) {
		m_buffer_rx32s[i] -= buf_mean;
		memcpy(&m_buffer_rx16s[i],&m_buffer_rx32s[i], sizeof(s16_t));
	}
	filter_sound(m_buffer_rx16s);
	static int counterrr = 0;
	if (counterrr < 32)
	{
		memcpy(buffer, m_buffer_rx16s, sizeof(m_buffer_rx16s));
		counterrr++;
	}
}

#ifdef __cplusplus
}
#endif