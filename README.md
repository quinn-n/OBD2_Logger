## OBD2 Logger

Logs automotive OBD2 data to an SD card, or to a serial connection, or both depending on what's available.

Configured to run out-of-the-box with existing [CAN shields](https://www.amazon.ca/gp/product/B076DSQFXH/).

What data is logged can be configured by setting `logged_pids`. If `logged_pids` is empty (`{}`) the logger will record all supported PIDs.

### Requirements

This project makes use of a few third-party Arduino libraries to work.

- [OBD2](https://github.com/sandeepmistry/arduino-OBD2), installable by searching `OBD2` in the library manager.
- [ArduinoSTL](https://github.com/mike-matera/ArduinoSTL), installable by searching `ArduinoSTL` in the library manager.
