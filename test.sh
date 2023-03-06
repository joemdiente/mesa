#!/bin/sh
#should be copied to ./mesa/<build>/ directory
make
sudo cp ./mesa/demo/mesa-demo-vsc7514 /srv/tftp
sudo cp ./mesa/demo/mipsel_vsc7514_pcb123-own_board.mfi /srv/tftp

