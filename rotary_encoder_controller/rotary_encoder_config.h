#ifndef rotary_encoder_config_h
#define rotary_encoder_config_h

#define SPI_FREQUENCY 1000000
#define SPI_WORD_SIZE 64
#define SPI_COMMAND_REDUNDANCY 11
#define SPI_READ_MAX_RETRY 8

#define SPEEDS_COUNT_TO_KEEP 10

#define COMMAND_TIMING 5
#define COMMAND_READ 42

#define SPI_MISO (gpio_num_t) 19
#define SPI_MOSI (gpio_num_t) 23
#define SPI_CLK (gpio_num_t) 18
#define SPI_CS   (gpio_num_t) 5

#endif