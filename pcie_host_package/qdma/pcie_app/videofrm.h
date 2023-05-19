/*
 * Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
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

#ifndef VIDEO_H
#define VIDEO_H

#include <QWidget>
#include <QPixmap>
#include <QDateTime>

class videofrm : public QWidget
{
    Q_OBJECT
public:
    explicit videofrm(QWidget *parent = nullptr);

    int fmcount = 0;
    bool waitBuf = false;
    QTimer * t;
    uchar *b;
    QByteArray ba;
    int counter = 0;
    int sizeval = 0;
    QString vfilename;
    char * yuvfrm;
    uchar * dfrm;
    QDateTime dg;
    int WID = 1920;
    int HEI = 1080;
    int FPS = 30;
    int testcnt = 0;
    bool hasWindow;
    int convert_yuv_to_rgb_buffer(unsigned char *yuv, unsigned char *rgb, unsigned int width, unsigned int height);
    void config_frame();
    void setResolution(int wi, int he, int fp);
public slots:
    void updateframe();
    void updateFPSslot(int val);
    void stopTimer();
    void exitApp();
signals:
    void updateFPS(int);
    void stopTimersig();
    void ctrlc();
};


#endif // VIDEO_H
