# ECE 528/L - Robotics and Embedded Systems with Lab
**CSU Northridge**

**Department of Electrical and Computer Engineering**

## PMOD COLOR Driver
This driver is used to demonstrate I2C communication with the PMOD COLOR module.

* PMOD COLOR: Color Sensor Module - [Product Link](https://digilent.com/shop/pmod-color-color-sensor-module/)

The example main program will sample the PMOD COLOR module every 50 ms. It will print the detected color values in hexadecimal format on the serial terminal. The `PMOD_Color_Display.py` Python test script can be used to read the color values on the serial terminal and display the detected color on a Pygame window. The script assumes that Windows is being used.

To run the `PMOD_Color_Display.py` Python script, the following must be installed:
* Python 3 - [Download Page Link](https://www.python.org/downloads/)
* Pygame - [Reference Page](https://www.pygame.org/wiki/GettingStarted) - This Python library can be installed using the following command in the Command Prompt: `python3 -m pip install -U pygame --user`
* Pyserial - [Reference Page](https://pypi.org/project/pyserial/)
