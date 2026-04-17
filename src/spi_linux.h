#ifndef SPI_LINUX_H
#define SPI_LINUX_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
    int fd;
    uint32_t speed_hz;
} spi_device_t;

int  spi_open(spi_device_t *dev, const char *device, uint32_t speed_hz);
void spi_close(spi_device_t *dev);
void spi_transfer(spi_device_t *dev, const uint8_t *tx, uint8_t *rx, size_t len);

#endif
