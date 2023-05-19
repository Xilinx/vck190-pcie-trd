Run XVDPU application
=====================

There are two applications could be run to show the AI inference capability of XVDPU.


SMART-MIPI-APP
--------------


The smart-mipi-app application is targeted to run with Single-MIPI(Leopard IMX274 MIPI) 
sensor as input source, and HDMI as output sink. The application supports 4 models - yolov3, 
refinedet, densebox, ssd.

Follow these steps to boot the board into Linux. These steps can be skipped 
if you are already at the Linux prompt *xilinx-vck190-20222* 

* Ensure all steps under the section :ref:`Board Setup<Board Setup>` are
  verified. Make connections based on the input source and output sink
  selected.

* Insert the prepared micro SD card into the Versal SD card slot (refer to the
  image in :ref:`Board Setup<Board Setup>`)

* Have the UART0 terminal emulator tab connected.

* Turn ON power switch SW13.

* On Versal UART0 terminal, you would see the Versal device booting from the
  micro SD card starting with the message
  "Xilinx Versal Platform Loader and Manager"

* In about 60 seconds boot is complete. Observe the Linux prompt
  *xilinx-vck190-20222* and finish logging in.

* Use command line options provided below to run the smart-mipi-app application

Examples for single-mipi sensor
>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

* Run 1 channel mipi camera with 3840x2160 resolution monitor

  .. code-block:: bash

     sudo smart-mipi-app -s

* Run 1 channel mipi camera with 1920x1080 resolution monitor

  .. code-block:: bash

     sudo smart-mipi-app -s -W 1920 -H 1080

* Run 1 channel mipi camera with specified AI model

  .. code-block:: bash

     sudo smart-mipi-app -s -t yolov3

* Run 1 channel mipi camera with specified media device, the default is "/dev/media1"

  .. code-block:: bash

     sudo smart-mipi-app -s -m 2

**Note:** Currently, Only **yolov3, densebox, refinedet, ssd** models are supported by this application.

Command Options:
>>>>>>>>>>>>>>>>

* The examples show the capability of the smart-mipi-app for specific configurations. User can get more and detailed
  application options as following by invoking

.. code-block:: bash

   smart-mipi-app -h

.. code-block:: bash

      Usage:
      smart-mipi-app [OPTION?] - Application for detction on VCK190 board of Xilinx.

      Examples for single mipi camera:
      smart-mipi-app -s
               # Run single channel mipi camera with 3840x2160 resolution monitor.
      smart-mipi-app -s -W 1920 -H 1080
               # Change to 1920x1080 resolution monitor.
      smart-mipi-app -s -t ssd
               # Change ai task from yolov3 to ssd
      smart-mipi-app -s -m 2
               # Change mipi camera device to /dev/media2.

      Help Options:
      -h, --help                                Show help options
      --help-all                                Show all help options
      --help-gst                                Show GStreamer Options

      Application Options:
      -v, --verbose                             print gstreamer pipeline
      -s, --single                              only process one channel video and display fullscreen
      -W, --width=WIDTH                         resolution width of the input: [1920 | 3840], default: 3840
      -H, --height=HEIGHT                       resolution height of the input: [1080 | 2160], default: 2160
      -t, --task=TASK                           select AI task to be run: [yolov3 | facedetect | refinedet | ssd], default: yolov3, work only when single is true
      --t1=TASK                                 select AI task to be run for channel 1, default: refinedet
      --t2=TASK                                 select AI task to be run for channel 2, default: facedetect
      --t3=TASK                                 select AI task to be run for channel 3, default: ssd
      --t4=TASK                                 select AI task to be run for channel 4, default: yolov3
      -m, --media-device=NUM                    num of media-device, default: 1
      -n, --channel-num=NUM                     channel numbers of video: [1 | 2 | 3 | 4], work only when single is false
      -x, --xclbin-location=XCLBIN-LOCATION     set path of xclbin
      -c, --config-dir=CONFIG-DIR               set config path of gstreamer plugin
      -p, --performace                          print performance

Files structure of the application
>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

The application is installed as:

* Binary File Directory: /usr/bin

  .. code-block:: bash

     /usr/bin/smart-mipi-app

* Configuration file directory: /usr/share/vvas/smart-mipi-app

  .. code-block:: bash

      |-- facedetect
      |   |-- aiinference.json
      |   |-- drawresult.json
      |   `-- preprocess.json
      |-- refinedet
      |   |-- aiinference.json
      |   |-- drawresult.json
      |   `-- preprocess.json
      |-- ssd
      |   |-- aiinference.json
      |   |-- drawresult.json
      |   |-- label.json
      |   `-- preprocess.json
      `-- yolov3
          |-- aiinference.json
          |-- drawresult.json
          `-- preprocess.json

* Model file directory: /usr/share/vitis_ai_library/models

  .. code-block:: bash

      |-- densebox_640_360
      |   |-- densebox_640_360.prototxt
      |   |-- densebox_640_360.xmodel
      |   `-- md5sum.txt
      |-- refinedet_pruned_0_96
      |   |-- md5sum.txt
      |   |-- refinedet_pruned_0_96.prototxt
      |   `-- refinedet_pruned_0_96.xmodel
      |-- ssd_adas_pruned_0_95
      |   |-- label.json
      |   |-- md5sum.txt
      |   |-- ssd_adas_pruned_0_95.prototxt
      |   `-- ssd_adas_pruned_0_95.xmodel
      `-- yolov3_voc_tf
         |-- label.json
         |-- md5sum.txt
         |-- yolov3_voc_tf.prototxt
         `-- yolov3_voc_tf.xmodel


PCIE-GST-APP
------------

The pcie-gst-app application is targeted to show the PCIE related features including transfering video file though PCIE to EP board,
and transfering the raw/processed MIPI images captured at the EP board and transfering back to host machine. 

The pcie-gst-app provides mulitple usecases 2 of which are for XVDPU application, each supporting 4 AI models - yolov3, refinedet, densebox and ssd.

Setup
>>>>>

Please refer to :ref:`Board and System settings<Board and System settings>` to setup the host machine, and boot the vck190 board.


Run Host application
>>>>>>>>>>>>>>>>>>>>


**Note:**  Make sure, HOST application is launched before starting EP application.
 
Here are list of control information passed to endpoint :
  
  .. code-block:: bash
  
     -- Usecase to run.
     -- Resolution.
     -- Filter type.
     -- FPS (Default 30fps).
     -- Rawvideofile (with abosolute path of video file to play).
 
This example demonstrates Usecase-2(MIPI --> DPU Inference --> Appsink(PCIe))

#. First run Host Machine Software setup steps, then execute pcie_host_app application as following.
	
   .. code-block:: bash
  
	  # ./pcie_host_app

#. From the six usecases select case 2 for MIPI DPU pipeline or 7 for videofile DPU pipeline or 9 to quit application.

   .. code-block:: bash
  
    # ./pcie_host_app
    Enter 1 to run  : MIPI-->filter2d-->pciesink--> displayonhost
    Enter 2 to run  : MIPI-->dpu-->pciesink--> displayonhost
    Enter 3 to run  : MIPI-->pciesink--> displayonhost
    Enter 4 to run  : RawVideofilefromHost-->pciesrc-->filter2d-->pciesink-->displayonhost
    Enter 5 to run  : RawVideofilefromHost--> pciesrc-->pciesink-->displayonhost
    Enter 6 to run  : RawVideofilefromHost--> pciesrc-->filter2d-->kmssink
    Enter 7 to run  : RawVideofilefromHost--> pciesrc-->dpu-->kmssink
    Enter 8 to run  : RawVideofilefromHost--> pciesrc-->kmssink
    Enter 9 to      : Exit application


#. Select desired resolution (Enter 1 or 2 ):

   .. code-block:: bash
  
    # ./pcie_host_app 
    Enter 1 to run  : MIPI-->filter2d-->pciesink--> displayonhost
    Enter 2 to run  : MIPI-->dpu-->pciesink--> displayonhost
    Enter 3 to run  : MIPI-->pciesink--> displayonhost
    Enter 4 to run  : RawVideofilefromHost-->pciesrc-->filter2d-->pciesink-->displayonhost
    Enter 5 to run  : RawVideofilefromHost--> pciesrc-->pciesink-->displayonhost
    Enter 6 to run  : RawVideofilefromHost--> pciesrc-->filter2d-->kmssink
    Enter 7 to run  : RawVideofilefromHost--> pciesrc-->dpu-->kmssink
    Enter 8 to run  : RawVideofilefromHost--> pciesrc-->kmssink
    Enter 9 to      : Exit application
    Enter your choice:2
    select the resolution 
    1. 3840x2160
    2. 1920x1080
    Enter your choice:

Run end-point application
>>>>>>>>>>>>>>>>>>>>>>>>>
1. Launch pcie-trd-nb1.ipynb jupyter notebook. (For MIPI use case modify 'res' variable same as one selected at host application). 

   **Note:** Endpoint application exits after running the usecase, Hence restart `pcie-trd-nb1.ipynb` jupyter notebook to relaunch the endpoint application.

2. The MIPI camera captured view with AI detection bounding box will start playing on the monitor.

,,,,,


Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file
except in compliance with the License.

You may obtain a copy of the License at
http://www.apache.org/licenses/LICENSE-2.0


Unless required by applicable law or agreed to in writing, software distributed under the
License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
either express or implied. See the License for the specific language governing permissions
and limitations under the License.
