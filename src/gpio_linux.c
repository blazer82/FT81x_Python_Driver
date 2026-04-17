#include "gpio_linux.h"

#include <gpiod.h>
#include <stddef.h>

int gpio_open(gpio_device_t *dev, const char *chipname, int cs1, int cs2, int dc) {
    dev->cs1_pin = cs1;
    dev->cs2_pin = cs2;
    dev->dc_pin = dc;
    dev->chip = NULL;
    dev->lines = NULL;

    dev->chip = gpiod_chip_open(chipname);
    if (!dev->chip) return -1;

    struct gpiod_line_settings *settings = gpiod_line_settings_new();
    if (!settings) goto fail;
    gpiod_line_settings_set_direction(settings, GPIOD_LINE_DIRECTION_OUTPUT);

    struct gpiod_line_config *line_cfg = gpiod_line_config_new();
    if (!line_cfg) { gpiod_line_settings_free(settings); goto fail; }

    /* CS1 and CS2: output HIGH (inactive, active-low) */
    gpiod_line_settings_set_output_value(settings, GPIOD_LINE_VALUE_ACTIVE);
    unsigned int cs_offsets[2] = { (unsigned int)cs1, (unsigned int)cs2 };
    gpiod_line_config_add_line_settings(line_cfg, cs_offsets, 2, settings);

    /* DC: output LOW (command mode) */
    gpiod_line_settings_set_output_value(settings, GPIOD_LINE_VALUE_INACTIVE);
    unsigned int dc_offset = (unsigned int)dc;
    gpiod_line_config_add_line_settings(line_cfg, &dc_offset, 1, settings);

    struct gpiod_request_config *req_cfg = gpiod_request_config_new();
    if (!req_cfg) {
        gpiod_line_config_free(line_cfg);
        gpiod_line_settings_free(settings);
        goto fail;
    }
    gpiod_request_config_set_consumer(req_cfg, "ft81x");

    dev->lines = gpiod_chip_request_lines(dev->chip, req_cfg, line_cfg);

    gpiod_request_config_free(req_cfg);
    gpiod_line_config_free(line_cfg);
    gpiod_line_settings_free(settings);

    if (!dev->lines) goto fail;

    return 0;

fail:
    if (dev->chip) { gpiod_chip_close(dev->chip); dev->chip = NULL; }
    return -1;
}

void gpio_close(gpio_device_t *dev) {
    if (dev->lines) { gpiod_line_request_release(dev->lines); dev->lines = NULL; }
    if (dev->chip) { gpiod_chip_close(dev->chip); dev->chip = NULL; }
}

void gpio_set(gpio_device_t *dev, int pin, int value) {
    gpiod_line_request_set_value(dev->lines, (unsigned int)pin,
                                 value ? GPIOD_LINE_VALUE_ACTIVE : GPIOD_LINE_VALUE_INACTIVE);
}

int gpio_get(gpio_device_t *dev, int pin) {
    return gpiod_line_request_get_value(dev->lines, (unsigned int)pin) == GPIOD_LINE_VALUE_ACTIVE ? 1 : 0;
}
