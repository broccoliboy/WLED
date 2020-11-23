# usermod_i2c_encoder

This usermod enables the use of a [DUPPA I2CEncoder V2.1](https://www.tindie.com/products/saimon/i2cencoder-v21-connect-rotary-encoder-on-i2c-bus/) rotary encoder + pushbutton to control WLED.

## Features

- On/off
  - Integrade button switch turns the strip on and off
- Brightness adjust
  - Turn the encoder knob to adjust brightness
- Effect adjust (encoder LED turns red)
  - Hold the button for 1 second to switch operating mode to effect adjust mode
  - When in effect adjust mode the integrated LED turns red
  - Rotating the knob cycles through all the effects
- Reset
  - When WLED is off (brightness 0) hold the button to reset and load Preset 1. Preset 1 must be defined for this to work.

## Hardware

This usermod is intended to work with the I2CEncoder V2.1 with the following configuration:

- Rotary encoder: Illuminated RGB Encoder
  - This encoder includes a pushbutton switch and an internal RGB LED to illuminate the shaft and any know attached to it.
  - This is the encoder: https://www.sparkfun.com/products/15141
- Knob: Any knob works, but the black knob has a transparent ring that lets the internal LED light through for a nice glow.
- Connectors: any
- LEDs: none (this is separate from the LED included in the encoder above)

## Compiling

This usermod requires the `ArduinoDuPPaLib` and `Wire` libraries as well as the following build flags to configure the I2C encoder:

- `USERMOD_I2C_ENCODER`
  - Define this to enable this usermod.
- `I2C_ENCODER_INT_PIN`
  - Pin to use for encoder interrupt. This will be pulled HIGH by the DUPPA board until there is data to be transmitted, at which point it will go LOW. Default is `1`.
- `I2C_ENCODER_SDA_PIN`
  - Pin to use for i2c SDA. Default is `0`.
- `I2C_ENCODER_SCL_PIN`
  - Pin to use for i2c SCL. Default is `2`.
- `I2C_ENCODER_ADDRESS`
  - I2c address of the encoder. Default is `0x00`.

Default pin assignments are intended for ESP-01 boards.

See `platformio_override.ini` for example usage.

Warning: if this usermod is enabled and no i2c encoder is connected you will have problems! The usermod expects something to be connected to 
