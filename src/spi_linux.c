#include "spi_linux.h"

#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>

int spi_open(spi_device_t *dev, const char *device, uint32_t speed_hz) {
    /* TODO: open fd, set SPI_MODE_0, 8 bits/word, speed_hz */
    return -1;
}

void spi_close(spi_device_t *dev) {
    /* TODO: close(dev->fd) */
}

void spi_transfer(spi_device_t *dev, const uint8_t *tx, uint8_t *rx, size_t len) {
    /* TODO: struct spi_ioc_transfer + ioctl SPI_IOC_MESSAGE(1) */
}
