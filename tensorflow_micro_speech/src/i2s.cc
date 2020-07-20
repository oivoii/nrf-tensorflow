

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
// static s16_t output_buf[I2S_DATA_BLOCK_WORDS];
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
void get_sound(void* buffer, size_t size)
{
	//memset(&m_buffer_rx32s, 0x00, sizeof(m_buffer_rx32s));
	s64_t buf_sum = 0;
	s64_t buf_mean = 0;
	s64_t words = I2S_DATA_BLOCK_WORDS;
	//printk("Start\n");
	
	//printk("Counter: %d\n", counter);

	for (int i = 0; i < I2S_DATA_BLOCK_WORDS; i++) {
		//if (!(i % 100))
		//{
		//	printk("\n\n%d\n", m_buffer_rx32u[i]);
		//}
		m_buffer_rx32u[i] <<= 8;
		// if (!(i % 100))
		// {
		// 	printk("%d\n", m_buffer_rx32u[i]);
		// }
		memcpy(m_buffer_rx32s + i,m_buffer_rx32u + i, sizeof(u32_t));
		// if (!(i % 100))
		// {
		// 	printk("%d\n", m_buffer_rx32u[i]);
		// }
		m_buffer_rx32s[i] >>= 16;
		//m_buffer_rx32s[i] >>= 6;
		//printk("%d, ", m_buffer_rx32s[i]);
		buf_sum += m_buffer_rx32s[i];

	}
	//memset(&m_buffer_rx32u, 0x00, sizeof(m_buffer_rx32u));
	buf_mean = buf_sum/words;
	for (int i = 0; i < I2S_DATA_BLOCK_WORDS; i++) {
		m_buffer_rx32s[i] -= buf_mean;
	}
	for (int i = 0; i < I2S_DATA_BLOCK_WORDS; i++)
	{
		// if (i == 25)
		// {
		// 	printk("\n\n16: %d, 32: %d\n", m_buffer_rx16s[i], m_buffer_rx32s[i]);
		// }
		//m_buffer_rx32s[i] <<= 14;
		//m_buffer_rx32s[i] >>= 16;
		//printk("%d, ", m_buffer_rx32s[i]);
		memcpy(&m_buffer_rx16s[i],&m_buffer_rx32s[i], sizeof(s16_t));
		// if (i == 25)
		// {
		// 	printk("\n16: %d, 32: %d\n", m_buffer_rx16s[i], m_buffer_rx32s[i]);
		// }
	}
	//printk("\n\n\n\n");
	memcpy(buffer, m_buffer_rx16s, sizeof(m_buffer_rx16s));
	
	//printk("buf_sum: %lld\n", buf_sum);
	//printk("buf_mean: %lld\n", buf_mean);
}

// void main()
// {
// 	printk("start");
// 	get_sound_init();
// 	k_sleep(K_SECONDS(4));
// 	get_sound(output_buf, I2S_DATA_BLOCK_WORDS);
// 	k_sleep(K_SECONDS(5));
// 	get_sound(output_buf, I2S_DATA_BLOCK_WORDS);
// 	for (int i = 0; i < I2S_DATA_BLOCK_WORDS; i++) {
// 		printk("%i, ", m_buffer_rx32s[i]);
// 		 k_sleep(K_MSEC(16));
// 	}
// 	printk("\n\n");
// 	while (1) {
// 		k_sleep(K_SECONDS(4));
// 	}
// }

#ifdef __cplusplus
}
#endif