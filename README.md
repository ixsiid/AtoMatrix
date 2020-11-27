# M5Atom Simple LED Matrix for esp-idf

M5AtomをESP-IDF環境で開発する際に、簡単にLED Matrixを扱うためのライブラリ
実装の簡便化のために次の制約がある。
色数は256色（RGB332フォーマット）
明るさ変更不可（FastLEDのsetBrightnessで、60相当に設定済み）



# Example

```
uint8_t px[25];
AtoMatrix* matrix;

void app_main() {

	for (int i = 0; i < 25; i++) px[i] = AtoMatrix::Red;

	matrix = new AtoMatrix(0, px);

	xTaskCreate([](void* arg) {
		uint8_t color[4]   = {AtoMatrix::RED, AtoMatrix::BLUE, AtoMatrix::GREEN, AtoMatrix::WHITE};
		uint8_t colorIndex = 0;

		while (true) {
			vTaskDelay(1000 / portTICK_RATE_MS);

			for (int i=0; i<25; i++) px[i] = color[colorIndex];
			colorIndex = (colorIndex + 1) % 4;
			
			matrix->update();
			count = (count + 1) % 32;
		}
	}, "MainLoopThread", 1024 * 8, matrix, 10, nullptr);
}

```


# 開発メモ

WS2812C-2020を制御するためには、数百nsのパルスを発生させる必要がある。
通常のGPIO制御では、数十kHz~程度の制御しかできない（パルス幅で20μs~）

タイマーとGPIOが連動する仕組みを利用する必要がある。
例: PWM, RMT, LEDC, I2S

このライブラリではI2Sを用いている


```
i2s_config_t config;
config.mode		    = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX);
config.sample_rate	    = 120000.0f;
config.bits_per_sample  = i2s_bits_per_sample_t::I2S_BITS_PER_SAMPLE_16BIT;
config.channel_format   = i2s_channel_fmt_t::I2S_CHANNEL_FMT_RIGHT_LEFT;
config.intr_alloc_flags = 0;
config.dma_buf_count    = 4;
config.dma_buf_len	    = 64;
config.use_apll	    = false;
```

ここで、`dma_buf_count * dma_buf_len [byte]` がバッファサイズになる。
I2Sは、オーディオ用インターフェースのためか、バッファ出力がループされるようになっている。

`i2c_write`でバッファに書き込む際に、バッファサイズ分余計に0を書き込むことで、WS2812への書き込みが1度で済むようにしている。

I2Sの通信速度は、`sample_rate * bits_per_sample * channel [bps]`となる。
sample_rateは、floatで指定するが実際に設定される値は離散的になり、i2c_get_clk()で得られる。
この環境では、実際には 125000Hz に設定されるため、4Mbps となり、 1bit のデータパルス幅は 250ns になるが、実際にオシロで計測すると 300ns 程だった。
1bitパルス幅が 300ns であれば、 0b1000 で High 300ns, Low 900ns となり、WS2812においての1bit Lowに、
0b1100 で High 600ns, Low 600ns となり、WS2812においての1bit Highになる。
変換前のデータ長も固定長になるため都合がいい


```
buffer = new uint32_t[led_count * 3 + 400];
for (int i = 0; i < 144; i++) buffer[i] = 0;
for (int i = led_count * 3 + 144; i < led_count * 3 + 400; i++) buffer[i] = 0;
```

送信用bufferのうち先頭 144 * 4 byte は、連続して更新があった際にも確実にリセットを送信するためのディレイ（280μs以上のLow信号）
後方 256 byte は、バッファを0で埋めるためのデータになっている。

