#!/bin/sh


TOOL=~/rk2918tools/

PATH=$BASE_DIR/tool:$PATH

echo "already[enter]"
read enter
sudo $TOOL/img-manager.py write boot boot.img
sudo $TOOL/img-manager.py write kernel kernel.img
sudo $TOOL/rkflashtool b
