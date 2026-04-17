#ifndef GPIO_LINUX_H
#define GPIO_LINUX_H

struct gpiod_chip;
struct gpiod_line_request;

typedef struct {
    struct gpiod_chip *chip;
    struct gpiod_line_request *lines;
    int cs1_pin;
    int cs2_pin;
    int dc_pin;
} gpio_device_t;

int  gpio_open(gpio_device_t *dev, const char *chipname, int cs1, int cs2, int dc);
void gpio_close(gpio_device_t *dev);
void gpio_set(gpio_device_t *dev, int pin, int value);
int  gpio_get(gpio_device_t *dev, int pin);

#endif
