#!/bin/sh


TOOL=~/rk2918tools/

PATH=$BASE_DIR/tool:$PATH

echo "already[enter]"
read enter
$TOOL/img-manager.py write boot boot.img.new
$TOOL/img-manager.py write kernel kernel.img
$TOOL/rkflashtool b
