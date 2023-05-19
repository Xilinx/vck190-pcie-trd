echo "MediaCtl settings for Sensor and CSI-RX Subsystem"
xmedia-ctl -d /dev/media0 -V '"IMX274 2-001a":0 [fmt:SRGGB10_1X10/1920x1080@1/30 field:none colorspace:srgb]'
xmedia-ctl -d /dev/media0 -V '"a4060000.csiss":0 [fmt:SRGGB10_1X10/1920x1080 field:none colorspace:srgb]'
xmedia-ctl -d /dev/media0 -V '"a4060000.csiss":1 [fmt:SRGGB10_1X10/1920x1080 field:none colorspace:srgb]'

echo "MediaCtl settings for extractFrames"
xmedia-ctl -d /dev/media0 -V '"a40c0000.extractEFrames":0 [fmt:SRGGB10_1X10/1920x1080 field:none colorspace:srgb]'
xmedia-ctl -d /dev/media0 -V '"a40c0000.extractEFrames":1 [fmt:SRGGB10_1X10/1920x1080 colorspace:srgb]'
xmedia-ctl -d /dev/media0 -V '"a40c0000.extractEFrames":2 [fmt:SRGGB10_1X10/1920x1080 colorspace:srgb]'

echo "MediaCtl settings for extractHdrMerge"
xmedia-ctl -d /dev/media0 -V '"a40e0000.hdrmerge":0 [fmt:SRGGB10_1X10/1920x1080 colorspace:srgb]'
xmedia-ctl -d /dev/media0 -V '"a40e0000.hdrmerge":1 [fmt:SRGGB10_1X10/1920x1080 colorspace:srgb]'
xmedia-ctl -d /dev/media0 -V '"a40e0000.hdrmerge":2 [fmt:SRGGB10_1X10/1920x1080 colorspace:srgb]'

echo "MediaCtl settings for ISPPipeline"
xmedia-ctl -d /dev/media0 -V '"a40f0000.ISPPipeline":0 [fmt:SRGGB10_1X10/1920x1080 colorspace:srgb]'
xmedia-ctl -d /dev/media0 -V '"a40f0000.ISPPipeline":1 [fmt:RBG888_1X24/1920x1080 field:none colorspace:srgb]'

echo "MediaCtl settings for Scaler"
xmedia-ctl -d /dev/media0 -V '"a4080000.scaler":0 [fmt:RBG888_1X24/1920x1080 field:none colorspace:srgb]'
xmedia-ctl -d /dev/media0 -V '"a4080000.scaler":1 [fmt:UYVY8_1X16/1920x1080 field:none colorspace:srgb]'


