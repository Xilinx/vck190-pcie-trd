#!/bin/sh

if [ -f "$QMAKE_PATH" ]; then
    $QMAKE_PATH
    make
    mv app1 pcie_host_app
else 
    echo "PLEASE SET QMAKE_PATH to installed qt's qmake path"
fi
