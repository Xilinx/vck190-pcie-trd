/*********************************************************************
 * Copyright (C) 2021 Xilinx, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 ********************************************************************/

#ifndef _PCIE_SINK_H_
#define _PCIE_SINK_H_

#include <pcie_main.h>

/**
 * @brief The callback API when appsink receives a new sample
 *
 * @param[out] sink GstElement sink object
 * @param[in] data user data
 *
 * @return GST_FLOW_OK on success
 */
GstFlowReturn new_sample_cb (GstElement* sink, App* data);

#endif /* _PCIE_SINK_H_ */
