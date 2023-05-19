#!/bin/bash
#*******************************************************************************
# (C) Copyright 2023 Advanced Micro Devices, Inc.
# SPDX-License-Identifier: Apache-2.0
#**********************************************************************


ip=$(ip -4 addr show eth0 | grep -oE "inet ([0-9]{1,3}[\.]){3}[0-9]{1,3}" | cut -d ' ' -f2)
python_path=$(python3 -m site | grep packages |grep usr | sed 's/,//g' | sed 's/ //g'| sed 's/^.//;s/.$//')

if [ -z $ip ]; then
     echo "Cant find IP addr, please call /usr/bin/trd-dashboard.sh after assigning IP addr"
     exit 1
else
    echo "TRD Dashboard will be running at http://$ip:5006/trd-dashboard"
    bokeh serve --show --allow-websocket-origin=$ip:5006 $python_path/trd-dashboard
fi

