#pragma once

#include <driver/i2s.h>

class AtoMatrix {
    public:
	AtoMatrix(i2s_port_t port, uint8_t* pixels, gpio_num_t pin = gpio_num_t::GPIO_NUM_27, size_t led_num = 25);
	void update();
	void hide();

	const static uint8_t RED	  = 0b11100000;
	const static uint8_t GREEN = 0b00011100;
	const static uint8_t BLUE  = 0b00000011;
	const static uint8_t WHITE = 0b11111111;
	const static uint8_t BLACK = 0b00000000;

    private:
	i2s_port_t port;
	uint32_t* buffer;
	uint8_t* pixels;
	size_t led_count;

	const static uint32_t red[8];
	const static uint32_t green[8];
	const static uint32_t blue[4];
};
