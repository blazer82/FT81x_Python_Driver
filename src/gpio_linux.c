#include "gpio_linux.h"

#include <gpiod.h>

int gpio_open(gpio_device_t *dev, const char *chipname, int cs1, int cs2, int dc) {
    /* TODO: gpiod_chip_open, request lines as output, set initial values */
    return -1;
}

void gpio_close(gpio_device_t *dev) {
    /* TODO: gpiod_line_request_release, gpiod_chip_close */
}

void gpio_set(gpio_device_t *dev, int pin, int value) {
    /* TODO: gpiod_line_request_set_value */
}

int gpio_get(gpio_device_t *dev, int pin) {
    /* TODO: gpiod_line_request_get_value */
    return 0;
}
