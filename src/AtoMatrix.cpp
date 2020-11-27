#include "AtoMatrix.h"

const uint32_t AtoMatrix::red[8] = {
    0x88888888,  // 0
    0x888888c8,  // 2
    0x8888c88c,  // 11
    0x888c8c88,  // 20
    0x888cc88c,  // 29
    0x88c88cc8,  // 38
    0x88c8cccc,  // 47
    0x88ccc888,  // 56
};

const uint32_t AtoMatrix::green[8] = {
    0x88888888,  // 0
    0x8888888c,  // 1
    0x88888cc8,  // 6
    0x8888c8cc,  // 11
    0x888c8888,  // 16
    0x888c8c8c,  // 21
    0x888cc8c8,  // 26
    0x888ccccc,  // 31
};

const uint32_t AtoMatrix::blue[4] = {
    0x88888888,  // 0
    0x888c8c8c,  // 21
    0x88c8c8c8,  // 42
    0x88cccccc,  // 63
};

AtoMatrix::AtoMatrix(i2s_port_t port, uint8_t* pixels, gpio_num_t pin, size_t led_num) {
	this->port   = port;
	this->pixels = pixels;
	led_count	   = led_num;

	i2s_config_t config;
	config.mode		    = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX);
	config.sample_rate	    = 120000.0f;
	config.bits_per_sample  = i2s_bits_per_sample_t::I2S_BITS_PER_SAMPLE_16BIT;
	config.channel_format   = i2s_channel_fmt_t::I2S_CHANNEL_FMT_RIGHT_LEFT;
	config.intr_alloc_flags = 0;
	config.dma_buf_count    = 4;
	config.dma_buf_len	    = 64;
	config.use_apll	    = false;

	i2s_pin_config_t i2s_pin;
	i2s_pin.data_out_num = pin;
	i2s_pin.bck_io_num	 = I2S_PIN_NO_CHANGE;
	i2s_pin.data_in_num	 = I2S_PIN_NO_CHANGE;
	i2s_pin.ws_io_num	 = I2S_PIN_NO_CHANGE;

	i2s_driver_install(port, &config, 0, nullptr);
	i2s_set_pin(port, &i2s_pin);

	buffer = new uint32_t[led_count * 3 + 400];
	for (int i = 0; i < 144; i++) buffer[i] = 0;
	for (int i = led_count * 3 + 144; i < led_count * 3 + 400; i++) buffer[i] = 0;
	update();
}

void AtoMatrix::update() {
	for (int i = 0; i < led_count; i++) {
		uint8_t r = uint8_t((pixels[i] & 0b11100000) >> 5);
		uint8_t g = uint8_t((pixels[i] & 0b00011100) >> 2);
		uint8_t b = uint8_t((pixels[i] & 0b00000011) >> 0);

		buffer[i * 3 + 0 + 144] = red[r];
		buffer[i * 3 + 1 + 144] = green[g];
		buffer[i * 3 + 2 + 144] = blue[b];
	}

	size_t count;
	i2s_write(port, buffer, (led_count * 3 + 400) * sizeof(uint32_t), &count, 200 / portTICK_RATE_MS);
}
