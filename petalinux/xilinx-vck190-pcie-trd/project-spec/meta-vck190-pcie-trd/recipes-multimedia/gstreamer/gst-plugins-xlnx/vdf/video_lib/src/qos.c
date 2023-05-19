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

#define _GNU_SOURCE
#include <fcntl.h>
#include <glob.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#include "helper.h"
#include "video_int.h"

#define DDR_QOS_MAP_NODE	"ddr_qos@fd090000"
#define AFIFM_HP0_MAP_NODE	"afifm@fd380000"
#define AFIFM_HP1_MAP_NODE	"afifm@fd390000"
#define AFIFM_HP2_MAP_NODE	"afifm@fd3a0000"
#define AFIFM_HP3_MAP_NODE	"afifm@fd3b0000"

struct register_data {
	uintptr_t offs;
	uint32_t val;
};

#define DDR_QOS_PORT_TYPE			0
#define DDR_QOS_PORT_TYPE_BEST_EFFORT		0
#define DDR_QOS_PORT_TYPE_LOW_LATENCY		1
#define DDR_QOS_PORT_TYPE_VIDEO			2
#define DDR_QOS_PORT_TYPE_PORT0_TYPE_SHIFT	0
#define DDR_QOS_PORT_TYPE_PORT1R_TYPE_SHIFT	2
#define DDR_QOS_PORT_TYPE_PORT2R_TYPE_SHIFT	6
#define DDR_QOS_PORT_TYPE_PORT3_TYPE_SHIFT	10
#define DDR_QOS_PORT_TYPE_PORT4_TYPE_SHIFT	12
#define DDR_QOS_PORT_TYPE_PORT5_TYPE_SHIFT	14
static const struct register_data qos_data_dp_ddr[] = {
	{ .offs = DDR_QOS_PORT_TYPE,
	  .val = DDR_QOS_PORT_TYPE_LOW_LATENCY << DDR_QOS_PORT_TYPE_PORT0_TYPE_SHIFT |
		 DDR_QOS_PORT_TYPE_LOW_LATENCY << DDR_QOS_PORT_TYPE_PORT1R_TYPE_SHIFT |
		 DDR_QOS_PORT_TYPE_LOW_LATENCY << DDR_QOS_PORT_TYPE_PORT2R_TYPE_SHIFT |
		 DDR_QOS_PORT_TYPE_VIDEO << DDR_QOS_PORT_TYPE_PORT3_TYPE_SHIFT |
		 DDR_QOS_PORT_TYPE_BEST_EFFORT << DDR_QOS_PORT_TYPE_PORT4_TYPE_SHIFT |
		 DDR_QOS_PORT_TYPE_BEST_EFFORT << DDR_QOS_PORT_TYPE_PORT5_TYPE_SHIFT, },
};

static const struct register_data qos_data_hdmi_ddr[] = {
	{ .offs = DDR_QOS_PORT_TYPE,
	  .val = DDR_QOS_PORT_TYPE_LOW_LATENCY << DDR_QOS_PORT_TYPE_PORT0_TYPE_SHIFT |
		 DDR_QOS_PORT_TYPE_LOW_LATENCY << DDR_QOS_PORT_TYPE_PORT1R_TYPE_SHIFT |
		 DDR_QOS_PORT_TYPE_LOW_LATENCY << DDR_QOS_PORT_TYPE_PORT2R_TYPE_SHIFT |
		 DDR_QOS_PORT_TYPE_BEST_EFFORT << DDR_QOS_PORT_TYPE_PORT3_TYPE_SHIFT |
		 DDR_QOS_PORT_TYPE_BEST_EFFORT << DDR_QOS_PORT_TYPE_PORT4_TYPE_SHIFT |
		 DDR_QOS_PORT_TYPE_BEST_EFFORT << DDR_QOS_PORT_TYPE_PORT5_TYPE_SHIFT, },
};

#define AFIFM_RDQOS	8
#define AFIFM_WRQOS	0x1c
static const struct register_data qos_data_afifmhp1[] = {
	{ .offs = AFIFM_WRQOS, .val = 0, },
};

static const struct register_data qos_data_afifmhp2[] = {
	{ .offs = AFIFM_RDQOS, .val = 0, },
	{ .offs = AFIFM_WRQOS, .val = 0, },
};

static const struct register_data qos_data_afifmhp3[] = {
	{ .offs = AFIFM_RDQOS, .val = 0, },
	{ .offs = AFIFM_WRQOS, .val = 0, },
};

static const struct register_data qos_data_dp_afifmhp0[] = {
	{ .offs = AFIFM_RDQOS, .val = 0x7, },
};

static const struct register_data qos_data_hdmi_afifmhp0[] = {
	{ .offs = AFIFM_RDQOS, .val = 0xf, },
};

struct register_init_data {
	const char *fn;
	size_t sz;
	const struct register_data *data;
};

#define DEFINE_RINIT_DATA(_fn, _arry)	{ .fn = _fn, .sz = ARRAY_SIZE(_arry), .data = _arry, }
static const struct register_init_data qos_settings_dp[] = {
	DEFINE_RINIT_DATA(DDR_QOS_MAP_NODE, qos_data_dp_ddr),
	DEFINE_RINIT_DATA(AFIFM_HP0_MAP_NODE, qos_data_dp_afifmhp0),
	DEFINE_RINIT_DATA(AFIFM_HP1_MAP_NODE, qos_data_afifmhp1),
	DEFINE_RINIT_DATA(AFIFM_HP2_MAP_NODE, qos_data_afifmhp2),
	DEFINE_RINIT_DATA(AFIFM_HP3_MAP_NODE, qos_data_afifmhp3),
};

static const struct register_init_data qos_settings_hdmi[] = {
	DEFINE_RINIT_DATA(DDR_QOS_MAP_NODE, qos_data_hdmi_ddr),
	DEFINE_RINIT_DATA(AFIFM_HP0_MAP_NODE, qos_data_hdmi_afifmhp0),
	DEFINE_RINIT_DATA(AFIFM_HP1_MAP_NODE, qos_data_afifmhp1),
	DEFINE_RINIT_DATA(AFIFM_HP2_MAP_NODE, qos_data_afifmhp2),
	DEFINE_RINIT_DATA(AFIFM_HP3_MAP_NODE, qos_data_afifmhp3),
};

struct register_init {
	const struct register_init_data *data;
	size_t sz;
};

static const struct register_init qos_settings[] = {
	{ .data = qos_settings_dp, .sz = ARRAY_SIZE(qos_settings_dp) },
	{ .data = qos_settings_hdmi, .sz = ARRAY_SIZE(qos_settings_hdmi) },
};

static int get_uio_device(char *dev, const char *node)
{
	int ret;
	unsigned int i;
	glob_t pglob;
	char buf[32];
	FILE* f;

	/* Glob all uio device names in the system */
	ret = glob("/sys/class/uio/uio*/maps/map0/name", 0, NULL, &pglob);
	if (ret) {
		VLIB_REPORT_ERR("No uio devices present in system");
		ret = VLIB_ERROR_OTHER;
		goto error;
	}

	/*
	 * Extract node name from glob paths and compare with user provided node
	 * name. Return the corresponding uio device name.
	 */
	for (i = 0; i < pglob.gl_pathc; i++) {
		f = fopen(pglob.gl_pathv[i], "r");
		if (f == NULL) {
			VLIB_REPORT_ERR("Failed to open file %s: %s",
					pglob.gl_pathv[i], strerror(errno));
			ret = VLIB_ERROR_FILE_IO;
			goto error;
		}

		memset(buf, 0, sizeof buf);
		fscanf(f, "%s", buf);
		if (strcasestr(buf, node) != NULL) {
			unsigned int n;
			sscanf(pglob.gl_pathv[i], "/sys/class/uio/uio%u/maps/map0/name", &n);
			sprintf(dev, "/dev/uio%u", n);
			vlib_info("Found uio device '%s' for node '%s'\n", dev, node);
			fclose(f);
			break;
		}

		fclose(f);
	}

	/* Return error if there is no match */
	if (i == pglob.gl_pathc) {
		VLIB_REPORT_ERR("No uio device found for node '%s'", node);
		ret = VLIB_ERROR_OTHER;
	}

error:
	globfree(&pglob);

	return ret;
}

int vlib_platform_set_qos(size_t qos_setting)
{
	int ret = 0;

	vlib_info("set platform QoS for %s\n", qos_setting ? "HDMI Tx" : "DP Tx");

	const struct register_init *r = &qos_settings[qos_setting];
	ASSERT2(r, "invalid device\n");

	for (size_t i = 0; i < r->sz; i++) {
		const struct register_init_data *rb = &r->data[i];

		char f[32];
		get_uio_device(f, rb->fn);

		int fd = open(f, O_RDWR);
		if (fd == -1) {
			VLIB_REPORT_ERR("failed to open file '%s': %s", f,
					strerror(errno));
			continue;
		}

		uint32_t *map = mmap(NULL, 0x1000, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
		if (map == MAP_FAILED) {
			VLIB_REPORT_ERR("failed to mmap file '%s': %s", f,
					strerror(errno));
			ret = VLIB_ERROR_FILE_IO;
			goto err_close;
		}

		for (size_t j = 0; j < rb->sz; j++) {
			const struct register_data *rd = &rb->data[j];

			map[rd->offs / sizeof(uint32_t)] = rd->val;
		}

		munmap(map, 0x1000);
err_close:
		close(fd);
	}

	return ret;
}
