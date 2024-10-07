## Compiling

### Preparing
**Debian:** `sudo apt-get install libncurses5-dev libasound2-dev`  
**Gentoo:** `sudo emerge media-sound/alsa-utils`  

### Compile
`git clone https://github.com/Volkanite/Atenzo.git`  
`git submodule update --init --progress`  
`cd mxml`  
`./configure`  
`cd ..`  
`./compile.sh`

### Running
`./atenzo`

NOTE 1: If using a custom kernel, be sure to add ftdi_sio.ko module (CONFIG_USB_SERIAL_FTDI_SIO).  
NOTE 2: If you get permission denied trying to open ttyUSB0, add yourself to the dialout group or you can also just run the program as root.  
