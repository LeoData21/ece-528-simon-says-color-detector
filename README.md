# Simon Says : Color Detector
## Using the Digilent PMOD Color Sensor

It shows a random 4-color sequence, and the user must scan the same colors in the same order using real objects and the PMOD Color Sensor. The MSP432 checks each input and gives success or failure feedback through LEDs and motors

## MSP432 <--> Color Sensor Pinout Table
| Signal | MSP432 Pin | Direction | Voltage | Description   |
|------  |------------|---------- |---------|-------------  |
| SDA    | P6.4       | I/O       | 3.3V    | I2C data line |
| SCL    | P6.5       | Output    | 3.3V    | I2C clock     |
| INT    | —          | Input     | 3.3V    | Interrupt from sensor |
| EN     | P8.3       | Output    | 3.3V    | Sensor enable |
| VDD    | —          | —         | 3.3V    | Power         |
| GND    | —          | —         | 0V      | Ground        |
