#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/rmt_rx.h"
#include "driver/rmt_tx.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "esp32/rom/ets_sys.h"
#include "dht11.h"

static const char *TAG = "dht11";

static uint8_t GPIO_PIN = -1;

// RMT receive channel handle
static rmt_channel_handle_t rx_channel_handle = NULL;

// Data receive queue
static QueueHandle_t rx_receive_queue = NULL;

// RMT receive done event callback
static bool IRAM_ATTR rmt_rx_done_callback(rmt_channel_handle_t channel,
	 const rmt_rx_done_event_data_t *edata, void *user_data) {
	BaseType_t high_task_wakeup = pdFALSE;
	QueueHandle_t rx_receive_queue = (QueueHandle_t)user_data;
	// send the received RMT symbols to the parser task
	xQueueSendFromISR(rx_receive_queue, edata, &high_task_wakeup);
	return high_task_wakeup == pdTRUE;
}

// Parse RMT signal data
bool parse_items(rmt_symbol_word_t *item, int item_num, float *humidity, float *temperature) {
	int i = 0;
	unsigned int rh = 0, temp = 0, checksum = 0;

	if (item_num < 41) { 
		// Ensure we received enough signals
		ESP_LOGE(TAG, "item_num < 41  %d", item_num);
		return false;
	}

	if (item_num > 41) {
		item++; // Skip the signal header
	}

	// Parse relative humidity data
	for (i = 0; i < 16; i++, item++) {
		uint16_t duration = 0;
		if (item->level0) {
			duration = item->duration0;
		} else {
			duration = item->duration1;
		}

		rh = (rh << 1) + (duration < 35 ? 0 : 1);
	}

	// Parse temperature data
	for (i = 0; i < 16; i++, item++) {
		uint16_t duration = 0;
		if (item->level0) {
			duration = item->duration0;
		} else {
			duration = item->duration1;
		}

		temp = (temp << 1) + (duration < 35 ? 0 : 1);
	}

	// Parse checksum data
	for (i = 0; i < 8; i++, item++) {
		uint16_t duration = 0;
		if (item->level0) {
			duration = item->duration0;
		} else {
			duration = item->duration1;
		}

		checksum = (checksum << 1) + (duration < 35 ? 0 : 1);
	}

	// Verify checksum
	if ((((temp >> 8) + temp + (rh >> 8) + rh) & 0xFF) != checksum) {
		ESP_LOGI(TAG, "Checksum failure %4X %4X %2X\n", temp, rh, checksum);
		return false;
	}

	// Ensure relative humidity and temperature data is in range before return
	rh = rh >> 8;
	temp = (temp >> 8) * 10 + (temp & 0xFF);

	if (rh <= 100) {
		*humidity = rh;
	}
	if (temp <= 600) {
		*temperature = temp / 10.0;
	}

	return true;
}

void dht11_init(uint8_t gpio_pin) {
	GPIO_PIN = gpio_pin;

	// Create RMT receive channel
	rmt_rx_channel_config_t rx_channel_config = {
		.clk_src = RMT_CLK_SRC_APB,	  // Clock source
		.resolution_hz = 1000 * 1000, // Tick Resolution is 1 MHz (1 tick = 1 µs)
		.mem_block_symbols = 64,	  // Memory block is set to 64 * 4 = 256 bytes
		.gpio_num = GPIO_PIN,		  // GPIO pin for DHT11
		.flags.invert_in = false,	  // Set invert signal to false
		.flags.with_dma = false,	  // My understanding is that DMA is for ESP32S3 only (need to verify)
	};
	ESP_ERROR_CHECK(rmt_new_rx_channel(&rx_channel_config, &rx_channel_handle));

	// Create data receiving queue
	rx_receive_queue = xQueueCreate(1, sizeof(rmt_rx_done_event_data_t));
	assert(rx_receive_queue);

	// Register RMT receive done event callback
	ESP_LOGI(TAG, "Register RMT receive done event callback");
	rmt_rx_event_callbacks_t cbs = {
		.on_recv_done = rmt_rx_done_callback,
	};
	ESP_ERROR_CHECK(rmt_rx_register_event_callbacks(rx_channel_handle, &cbs, rx_receive_queue));

	// Enable RMT receive channel
	ESP_ERROR_CHECK(rmt_enable(rx_channel_handle));
}

bool dht11_read(float *temperature, float *humidity) {
	// Set GPIO pin to OUTPUT mode
	gpio_set_direction(GPIO_PIN, GPIO_MODE_OUTPUT);
	// HIGH signal for 20ms
	gpio_set_level(GPIO_PIN, 1);
	ets_delay_us(1000);
	// LOW signal for 20ms
	gpio_set_level(GPIO_PIN, 0);
	ets_delay_us(20000);
	// HIGH signal for 20us
	gpio_set_level(GPIO_PIN, 1);
	ets_delay_us(20);

	// Set GPIO pin to INPUT mode
	gpio_set_direction(GPIO_PIN, GPIO_MODE_INPUT);
	// Set PULLUP
	gpio_set_pull_mode(GPIO_PIN, GPIO_PULLUP_ONLY);

	// Start RTM receive
	rmt_receive_config_t receive_config = {
		.signal_range_min_ns = 100,			// Minimal signal width (0.1us). Signal is ignored (treated as inetreference) if the width is less than this value
		.signal_range_max_ns = 1000 * 1000, // Maximal signal width (1000us)，Signal is treaded as ending signal if the width is larger than this value
	};
	rmt_symbol_word_t raw_symbols[128]; // Buffer for received raw symbols
	ESP_ERROR_CHECK(rmt_receive(rx_channel_handle, raw_symbols, sizeof(raw_symbols), &receive_config));

	// wait for RX done signal
	rmt_rx_done_event_data_t rx_data; // Actual data received
	if (xQueueReceive(rx_receive_queue, &rx_data, pdMS_TO_TICKS(1000)) == pdTRUE) {
		// parse the receive symbols and print the result
		return parse_items(rx_data.received_symbols, rx_data.num_symbols, humidity, temperature);
	}

	return true;
}