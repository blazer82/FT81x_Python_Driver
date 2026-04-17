#include "spi_linux.h"

#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>

int spi_open(spi_device_t *dev, const char *device, uint32_t speed_hz) {
    dev->fd = open(device, O_RDWR);
    if (dev->fd < 0) return -1;

    uint8_t mode = SPI_MODE_0 | SPI_NO_CS;
    if (ioctl(dev->fd, SPI_IOC_WR_MODE, &mode) < 0) goto fail;

    uint8_t bits = 8;
    if (ioctl(dev->fd, SPI_IOC_WR_BITS_PER_WORD, &bits) < 0) goto fail;

    if (ioctl(dev->fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed_hz) < 0) goto fail;

    dev->speed_hz = speed_hz;
    return 0;

fail:
    close(dev->fd);
    dev->fd = -1;
    return -1;
}

void spi_close(spi_device_t *dev) {
    if (dev->fd >= 0) close(dev->fd);
    dev->fd = -1;
}

void spi_transfer(spi_device_t *dev, const uint8_t *tx, uint8_t *rx, size_t len) {
    struct spi_ioc_transfer tr;
    memset(&tr, 0, sizeof(tr));
    tr.tx_buf = (unsigned long)tx;
    tr.rx_buf = (unsigned long)rx;
    tr.len = len;
    tr.speed_hz = dev->speed_hz;
    tr.bits_per_word = 8;
    ioctl(dev->fd, SPI_IOC_MESSAGE(1), &tr);
}
