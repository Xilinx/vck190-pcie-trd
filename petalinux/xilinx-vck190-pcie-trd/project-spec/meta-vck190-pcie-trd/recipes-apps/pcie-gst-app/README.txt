Steps to build the pcie-gst-app
---------

# petalinux-build -c pcie-gst-app


Steps to run the pcie-gst-app
----------

NOTE: Assuming setup has been done correctly.

1. Launch the host application with required use-case
2. Once host application is running, launch the pcie-gst-app with below command

# pcie-gst-app

To check the fps information
# GST_DEBUG="*pcie*:4" GST_DEBUG_FILE=/run/fps.log pcie-gst-app

To check latency data
# GST_DEBUG="GST_TRACER:7" GST_TRACERS="latency" GST_DEBUG_FILE=/run/latency.log pcie-gst-app

To check interlatency data
# GST_DEBUG="GST_TRACER:7" GST_TRACERS="interlatency" GST_DEBUG_FILE=/run/interlatency.log pcie-gst-app
