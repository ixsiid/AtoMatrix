#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "AtoMatrix.h"

#define TAG "ESPIDF_MATRIX"

extern "C" {
void app_main();
}

uint8_t px[25];

static uint32_t numericFont[20] = {
    //.......1....2....3....4....5....
    0b00000000001000101001010010100010,	 //  0
    0b00000000001000110000100001000010,
    0b00000000001000101000010001000111,
    0b00000000011100001000100000100111,
    0b00000000010100101001110000100001,

    0b00000000011100100001100000100110,	 //  5
    0b00000000001100100001100010100010,
    0b00000000011100001000010001000010,
    0b00000000001000101000100010100111,
    0b00000000011100101001110000100001,

    0b00000001001010101101011010110010,	 // 10
    0b00000001001010110100101001010010,
    0b00000001001010101100011001010111,
    0b00000001011110001100101000110111,
    0b00000001010110101101111000110001,

    0b00000001011110100101101000110110,	 // 15
    0b00000001001110100101101010110010,
    0b00000001011110001100011001010010,
    0b00000001001010101100101010110111,
    0b00000001011110101101111000110001,
};

void app_main() {
	for (int i = 0; i < 25; i++) {
		px[i] = 0;
	}
	
	AtoMatrix* matrix = new AtoMatrix(0, px);

	xTaskCreate([](void* arg) {
		AtoMatrix* matrix = (AtoMatrix*)arg;

		uint8_t count = 0;
		int ox = 0, oy = 0;

		uint8_t fontColor[4]   = {AtoMatrix::RED, AtoMatrix::BLUE, AtoMatrix::GREEN, AtoMatrix::WHITE};
		uint8_t fontColorIndex = 0;

		while (true) {
			vTaskDelay(1000 / portTICK_RATE_MS);

			if (count < 20) {
				for (int i = 0; i < 25; i++) {
					bool on = numericFont[count] & (0b1 << (25 - 1 - i));
					px[i]   = on ? fontColor[fontColorIndex] : AtoMatrix::BLACK;
				}
				fontColorIndex = (fontColorIndex + 1) % 4;
			} else {
				fontColorIndex = 0;
				for (int x = 0; x < 5; x++) {
					for (int y = 0; y < 5; y++) {
						uint8_t r = (x + ox) % 7;
						uint8_t g = (y + oy) % 7;
						uint8_t b = (x + y + ox + oy) % 3;

						px[y * 5 + x] = (r << 5) | (g << 2) | (b << 0);
					}
				}
				ox += 1;
				oy += 1;
			}
			matrix->update();
			count = (count + 1) % 32;
		}
	}, "MainLoopThread", 1024 * 8, matrix, 10, nullptr);
}
