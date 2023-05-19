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

#include "pcie_abstract.h"

#define GET_FILE_LENGTH           0x0
#define GET_KERNEL_MODE           0x1
#define SET_READ_TRANSFER_DONE    0x5
#define CLR_READ_TRANSFER_DONE    0x6
#define SET_WRITE_TRANSFER_DONE   0x7
#define CLR_WRITE_TRANSFER_DONE   0x8
#define GET_RESOLUTION            0x9
#define GET_FPS                   0xb
#define GET_FILTER_TYPE           0xd
#define ALLOC_DMA_BUFF            0xe
#define RELEASE_DMA_BUFF          0xf
#define PCIE_DMABUF_IMPORT        0x1b
#define GET_USE_CASE              0x1d
#define MAP_DMA_BUFF              0x1e
#define UNMAP_DMA_BUF             0x1f
#define READ_STOP_MIPI_FEED       0x20
#define NUM_DMA_BUF       	  0x21

#define SET_READ_TRANSFER_DONE_VAL   0xef
#define CLR_READ_TRANSFER_DONE_VAL   0x00
#define SET_WRITE_TRANSFER_DONE_VAL  0xef
#define CLR_WRITE_TRANSFER_DONE_VAL  0x00

GST_DEBUG_CATEGORY_EXTERN (pcie_gst_app_debug);
#define GST_CAT_DEFAULT pcie_gst_app_debug

gint pcie_open()
{
    gint fpga_fd = 0;

    fpga_fd = open(DEVICE_NAME, O_RDWR);
    if (fpga_fd < 0) {
        GST_ERROR ("Failed to open device %d", fpga_fd);
        return -EINVAL;
    }

    return fpga_fd;
}

gulong pcie_get_file_length(gint fpga_fd, gulong *file_length)
{
    gint ret = -EINVAL;

    if (fpga_fd >= 0) {
        ret = ioctl(fpga_fd, GET_FILE_LENGTH, file_length);
        if (ret < 0)
            GST_ERROR ("Failed to run ioctl to get file-length");
    }

    return ret;
}

gint pcie_get_input_resolution(gint fpga_fd, struct resolution *input_res)
{
    gint ret = -EINVAL;

    if (fpga_fd >= 0) {
        ret = ioctl(fpga_fd, GET_RESOLUTION, input_res);
        if (ret < 0)
            GST_ERROR ("Failed to run ioctl to get input resolution");
    }

    return ret;
}

gint pcie_get_fps(gint fpga_fd, guint *fps)
{
    gint ret = -EINVAL;

    if (fpga_fd >= 0) {
        ret = ioctl(fpga_fd, GET_FPS, fps);
        if (ret < 0)
            GST_ERROR ("Failed to run ioctl to get fps");
    }

    return ret;
}

gint pcie_get_kernel_mode(gint fpga_fd, guint *kernel_mode)
{
    gint ret = -EINVAL;

    if (fpga_fd >= 0) {
        ret = ioctl(fpga_fd, GET_KERNEL_MODE, kernel_mode);
        if (ret < 0)
            GST_ERROR ("Failed to run ioctl to get kernel mode");
    }

    return ret;
}

gint pcie_get_filter_type(gint fpga_fd, guint *filter_type)
{
    gint ret = -EINVAL;

    if (fpga_fd >= 0) {
        ret = ioctl(fpga_fd, GET_FILTER_TYPE, filter_type);
        if (ret < 0)
            GST_ERROR ("Failed to run ioctl to get filter type");
    }

    return ret;
}

gint pcie_read_stop_mipi_feed(gint fpga_fd, guint *mipi_feed)
{
    gint ret = -EINVAL;

    if (fpga_fd >= 0) {
        ret = ioctl(fpga_fd, READ_STOP_MIPI_FEED, mipi_feed);
        if (ret < 0)
            GST_ERROR ("Failed to run ioctl to read stop mipi feed");
    }

    return ret;
}

gint pcie_num_dma_buf(gint fpga_fd)
{
    gint ndmabuf = -EINVAL;
    if (fpga_fd >= 0) {
        ndmabuf = ioctl(fpga_fd, NUM_DMA_BUF,NULL);
        if (ndmabuf < 0)
            GST_ERROR ("Failed to run ioctl to number of dma buffers allocated");
    }

    return ndmabuf;
}

gint pcie_dma_import(gint fpga_fd, struct dma_buf_imp *dma_import)
{
    gint ret = -EINVAL;

    dma_import->dma_imp_flag = 1;

    if(fpga_fd >= 0) {
        ret = ioctl(fpga_fd, PCIE_DMABUF_IMPORT, dma_import);
        if (ret < 0)
            GST_ERROR ("Failed to run ioctl for dma buf import");
    }

    return ret;
}

gint pcie_dma_import_release(gint fpga_fd, struct dma_buf_imp *dma_import)
{
    gint ret = -EINVAL;

    dma_import->dma_imp_flag = 0;
    dma_import->dir = 0; /* DMA_BIDIRECTIONAL */

    if(fpga_fd >= 0) {
        ret = ioctl(fpga_fd, PCIE_DMABUF_IMPORT, dma_import);
        if (ret < 0)
            GST_ERROR ("Failed to run ioctl for dma buf import release");
    }

    return ret;
}

gint pcie_dma_export(gint fpga_fd, struct dma_buf_export *dma_export)
{
    gint ret = -EINVAL;

    if(fpga_fd >= 0) {
        ret = ioctl(fpga_fd, ALLOC_DMA_BUFF, dma_export);
        if (ret < 0)
            GST_ERROR ("Failed to run ioctl for dma buf export");
    }

    return ret;
}

gint pcie_dma_export_release(gint fpga_fd, struct dma_buf_export *dma_export)
{
    gint ret = -EINVAL;

    if(fpga_fd >= 0) {
        ret = ioctl(fpga_fd, RELEASE_DMA_BUFF, dma_export);
        if (ret < 0)
            GST_ERROR ("Failed to run ioctl for dma-buf export release");
    }

    return ret;
}

gint pcie_dma_map(gint fpga_fd, struct dma_buf_export *dma_map)
{
    gint ret = -EINVAL;

    if(fpga_fd >= 0) {
        ret = ioctl(fpga_fd, MAP_DMA_BUFF, dma_map);
        if (ret < 0)
            GST_ERROR ("Failed to run ioctl to map dma-buf");
    }

    return ret;
}

gint pcie_dma_unmap(gint fpga_fd, struct dma_buf_export *dma_map)
{
    gint ret = -EINVAL;

    if(fpga_fd >= 0) {
        ret = ioctl(fpga_fd, UNMAP_DMA_BUF, dma_map);
        if (ret < 0)
            GST_ERROR ("Failed to run ioctl to unmap dma-buf");
    }

    return ret;
}

gint pcie_get_usecase_type(gint fpga_fd, guint *usecase)
{
    gint  ret     = 0;

    if (fpga_fd >= 0) {
        ret = ioctl(fpga_fd, GET_USE_CASE, usecase);
        if (ret < 0)
            GST_ERROR ("Failed to run ioctl to get use-case type");
    }

    return ret;
}

gint pcie_set_read_transfer_done(gint fpga_fd)
{
    gint  ret   = -EINVAL;
    guint value = SET_READ_TRANSFER_DONE_VAL;

    if (fpga_fd >= 0) {
        ret = ioctl(fpga_fd, SET_READ_TRANSFER_DONE, &value);
        if (ret < 0) {
            GST_ERROR ("Failed to run ioctl for read transfer done");
        }
    }

    return ret;
}

gint pcie_set_write_transfer_done(gint fpga_fd)
{
    gint  ret   = -EINVAL;
    guint value = SET_WRITE_TRANSFER_DONE_VAL;

    if (fpga_fd >= 0) {
        ret = ioctl(fpga_fd, SET_WRITE_TRANSFER_DONE, &value);
        if (ret < 0) {
            GST_ERROR ("Failed to run ioctl for write transfer done");
        }
    }

    return ret;
}

gint pcie_clr_read_transfer_done(gint fpga_fd)
{
    gint  ret   = -EINVAL;
    guint value = CLR_READ_TRANSFER_DONE_VAL;

    if (fpga_fd >= 0) {
        ret = ioctl(fpga_fd, CLR_READ_TRANSFER_DONE, &value);
        if (ret < 0) {
            GST_ERROR ("Failed to run ioctl for read transfer done");
        }
    }

    return ret;
}

gint pcie_clr_write_transfer_done(gint fpga_fd)
{
    gint  ret   = -EINVAL;
    guint value = CLR_WRITE_TRANSFER_DONE_VAL;

    if (fpga_fd >= 0) {
        ret = ioctl(fpga_fd, CLR_WRITE_TRANSFER_DONE, &value);
        if (ret < 0) {
            GST_ERROR ("Failed to run ioctl for write transfer done");
        }
    }

    return ret;
}


gint pcie_read(gint fpga_fd, gint size, gulong offset, gchar *buff)
{
    glong  ret       = -EINVAL;
    gulong lseek_cnt = 0;

    if (fpga_fd >= 0) {
        if (offset) {
            lseek_cnt = lseek(fpga_fd, offset, SEEK_SET);
            if (lseek_cnt != offset) {
                GST_ERROR ("Seek offset 0x%lx != 0x%lx", lseek_cnt, offset);
                return -EIO;
            }
        }

        ret = read(fpga_fd, buff, size);
        if (ret < 0) {
            GST_ERROR ("Return code = 0x%lx, read size = 0x%x", ret, size);
            return -EIO;
        }
    }

    return ret;
}

gint pcie_write(gint fpga_fd, gint size, gint offset, gchar *buff)
{
    glong ret = -EINVAL;

    if (fpga_fd >= 0) {
        ret = write(fpga_fd, buff, size);
        if (ret < 0) {
            GST_ERROR ("Return code = 0x%lx, write size = 0x%x", ret, size);
            return -EIO;
        }
    }

    return ret;
}

void pcie_close(gint fpga_fd)
{
    close(fpga_fd);
}
