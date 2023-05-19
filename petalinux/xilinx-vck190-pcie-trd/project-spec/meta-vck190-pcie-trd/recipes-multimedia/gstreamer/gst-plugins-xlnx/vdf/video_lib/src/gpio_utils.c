/*
 * Copyright (C) 2017 – 2018 Xilinx, Inc.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * XILINX BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the name of the Xilinx shall not be used
 * in advertising or otherwise to promote the sale, use or other dealings in
 * this Software without prior written authorization from Xilinx.
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

#include "gpio_utils.h"
#include "video.h"

#define GPIO_DIR_IN         0
#define GPIO_DIR_OUT        1

int gpio_export(unsigned int gpio)
{
	int fd, len, ret2, ret = 0;
	char buf[11];

	fd = open("/sys/class/gpio/export", O_WRONLY);
	if (fd < 0) {
		perror("gpio/export");
		return fd;
	}

	len = snprintf(buf, sizeof(buf), "%d", gpio);
	ret2 = write(fd, buf, len);
	if (ret2 != len) {
		ret = VLIB_ERROR_FILE_IO;
	}

	close(fd);

	return ret;
}

int gpio_unexport(unsigned int gpio)
{
	int fd, len, ret2, ret = 0;
	char buf[11];

	fd = open("/sys/class/gpio/unexport", O_WRONLY);
	if (fd < 0) {
		perror("gpio/unexport");
		return fd;
	}

	len = snprintf(buf, sizeof(buf), "%d", gpio);
	ret2 = write(fd, buf, len);
	if (ret2 != len) {
		ret = VLIB_ERROR_FILE_IO;
	}

	close(fd);

	return ret;
}

static int gpio_dir(unsigned int gpio, unsigned int dir)
{
	int fd, len, ret2, ret = 0;
	char buf[60];
	const char *dir_s;

	len = snprintf(buf, sizeof(buf), "/sys/class/gpio/gpio%d/direction",
		       gpio);

	if (len > 60)
		perror("\n***Filename too long in gpio_dir function\n");

	fd = open(buf, O_WRONLY);
	if (fd < 0) {
		perror("gpio/direction");
		return fd;
	}

	if (dir == GPIO_DIR_OUT) {
		dir_s = "out";
	} else {
		dir_s = "in";
	}

	len = strlen(dir_s);
	ret2 = write(fd, dir_s, len);
	if (ret2 != len) {
		ret = VLIB_ERROR_FILE_IO;
	}

	close(fd);

	return ret;
}

int gpio_dir_out(unsigned int gpio)
{
	return gpio_dir(gpio, GPIO_DIR_OUT);
}

int gpio_dir_in(unsigned int gpio)
{
	return gpio_dir(gpio, GPIO_DIR_IN);
}

int gpio_value(unsigned int gpio, unsigned int value)
{
	int fd, len, ret2, ret = 0;
	char buf[60];
	const char *val_s;

	len = snprintf(buf, sizeof(buf), "/sys/class/gpio/gpio%d/value", gpio);

	if (len > 60)
		perror("\n***Filename too long in gpio_dir function\n");

	fd = open(buf, O_WRONLY);
	if (fd < 0) {
		perror("gpio/value");
		return fd;
	}

	if (value) {
		val_s = "1";
	} else {
		val_s = "0";
	}

	len = strlen(val_s);
	ret2 = write(fd, val_s, len);
	if (ret2 != len) {
		ret = VLIB_ERROR_FILE_IO;
	}

	close(fd);

	return ret;
}

static int gpio_active_low(unsigned int gpio, unsigned int act_low)
{
	int fd, len, ret2, ret = 0;
	char buf[60];
	const char *val_s;

	len = snprintf(buf, sizeof(buf), "/sys/class/gpio/gpio%d/active_low",
		       gpio);

	if (len > 60)
		perror("\n***Filename too long in gpio_dir function\n");

	fd = open(buf, O_WRONLY);
	if (fd < 0) {
		perror("gpio/active_low");
		return fd;
	}

	if (act_low) {
		val_s = "1";
	} else {
		val_s = "0";
	}

	len = strlen(val_s);
	ret2 = write(fd, val_s, len);
	if (ret2 != len) {
		ret = VLIB_ERROR_FILE_IO;
	}

	close(fd);

	return ret;
}

int gpio_act_low(unsigned int gpio)
{
	return gpio_active_low(gpio, 1);
}

int gpio_act_high(unsigned int gpio)
{
	return gpio_active_low(gpio, 0);
}
