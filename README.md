# pi-whiteboard

To compile project, run `qmake` and then `make`.

The final binary can be found at `build/pi-whiteboard`.

The program takes two optional arguments to specify the SCL and SDA pins, like so: `pi-whiteboard [scl_pin] [sda_pin]`. The default, if no arguments are given, is equivalent to `pi-whiteboard 0 1`.
