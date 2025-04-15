#!/bin/bash

echo "==================================================================================="
echo "                    AMOS Desktop OS - Tabbed Windows VNC Demo                      "
echo "==================================================================================="
echo "This demonstration will show the Fluxbox-inspired tabbed window functionality"
echo "of the AMOS Desktop OS in a VNC viewer."
echo "==================================================================================="
echo "Starting VNC display..."

# Run the tabbed windows demo in VNC
python3 scripts/tabbed_windows_vnc_demo.py

echo "Demo complete."