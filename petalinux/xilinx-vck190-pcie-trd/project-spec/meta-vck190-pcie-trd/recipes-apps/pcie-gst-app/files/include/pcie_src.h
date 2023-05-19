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

#ifndef _PCIE_SRC_H_
#define _PCIE_SRC_H_

#include <pcie_main.h>

/**
 * @brief This method is called by the need-data signal callback, we create a
 * dma type buffer with data and push it to the next element.
 *
 * @param[out] appsrc GstElement type object
 * @param[out] size size
 * @param[in] app user data
 *
 * @return TRUE on success
 */
gboolean feed_data (GstElement* appsrc, guint size, App* app);

/**
 * @brief This signal callback triggers when appsrc needs data. Here, we add
 * an idle handler to the mainloop to start pushing data into the appsrc
 *
 * @param[out] source GstElement type object
 * @param[out] size size
 * @param[in] data user data
 */
void start_feed (GstElement* source, guint size, App* data);

/**
 * @brief This callback triggers when appsrc has enough data so that we can stop
 * sending the data and remove the idle handler from the mainloop.
 *
 * @param[out] source GstElement type object
 * @param[in] data user data
 */
void stop_feed (GstElement* source, App* data);

#endif /* _PCIE_SRC_H_ */
