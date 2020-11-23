#pragma once

#include "wled.h"
#include <Wire.h>
#include <i2cEncoderLibV2.h>

#ifndef I2C_ENCODER_INT_PIN
    #define I2C_ENCODER_INT_PIN 1
#endif

#ifndef I2C_ENCODER_SDA_PIN
    #define I2C_ENCODER_SDA_PIN 0
#endif

#ifndef I2C_ENCODER_SCL_PIN
    #define I2C_ENCODER_SCL_PIN 2
#endif

#ifndef I2C_ENCODER_ADDRESS
    #define I2C_ENCODER_ADDRESS 0x00
#endif

class I2cEncoder : public Usermod
{
private:
    i2cEncoderLibV2 * encoder_p;
    bool encoderButtonDown = false;
    uint32_t buttonPressStartTime = 0; // millis when button was pressed
    uint32_t buttonPressDuration = 0;
    const uint32_t buttonLongPressThreshold = 1000; // duration threshold for long press (millis)
    bool wasLongButtonPress = false;

    // encoderMode keeps track of what function the encoder is controlling
    // 0 = brightness
    // 1 = effect
    uint8_t encoderMode = 0;
    // encoderModes keeps track of what color the encoder LED should be for each mode
    const uint32_t encoderModes[2] = {0x0000FF, 0xFF0000};
    uint32_t lastInteractionTime = 0;
    const uint32_t modeResetTimeout = 30000; // timeout for reseting mode to 0
    const uint32_t brightnessDelta = 16;

    void updateBrightness(int8_t deltaBrightness)
    {
        // set new brightness constrained between 0 and 255
        bri = constrain(bri + deltaBrightness, 0, 255);
        colorUpdated(NOTIFIER_CALL_MODE_BUTTON);
    }

    void updateEffect(int8_t deltaEffect)
    {
        // set new effect with rollover at 0 and MODE_COUNT
        effectCurrent = (effectCurrent + MODE_COUNT + deltaEffect) % MODE_COUNT;
        colorUpdated(NOTIFIER_CALL_MODE_FX_CHANGED);
    }

    void setEncoderMode(uint8_t mode)
    {
        // set new mode and update encoder LED color
        encoderMode = mode;
        encoder_p->writeRGBCode(encoderModes[encoderMode]);
    }

    void handleEncoderShortButtonPress()
    {
        // on short button press
        toggleOnOff();
        colorUpdated(NOTIFIER_CALL_MODE_BUTTON);
        setEncoderMode(0);
    }

    void handlEncoderLongButtonPress()
    {
        // on long button press (> 1 second)
        if (encoderMode == 0 && bri == 0)
        {
            // if strip is off and long press, reset to preset 1
            applyPreset(1);
            colorUpdated(NOTIFIER_CALL_MODE_FX_CHANGED);
        }
        else
        {
            // otherwise cycle encoder operating modes
            setEncoderMode((encoderMode + 1) % (sizeof(encoderModes) / sizeof(encoderModes[0])));
        }

        buttonPressStartTime = millis();
        wasLongButtonPress = true;
    }

    void encoderRotated(i2cEncoderLibV2 *obj)
    {
        // callback when the encoder is rotated
        // adjust brightness or effect number depending on current operating mode
        switch (encoderMode)
        {
            case 0:
            {
                updateBrightness(obj->readStatus(i2cEncoderLibV2::RINC) ? brightnessDelta : -brightnessDelta);
                break;
            }
            case 1:
            {
                updateEffect(obj->readStatus(i2cEncoderLibV2::RINC) ? 1 : -1);
                break;
            }
        }

        lastInteractionTime = millis();
    }

    void encoderButtonPush(i2cEncoderLibV2 *obj)
    {
        // callback when the encoder integrated button is pressed
        encoderButtonDown = true;
        buttonPressStartTime = lastInteractionTime = millis();
    }

    void encoderButtonRelease(i2cEncoderLibV2 *obj)
    {
        // callback when the encoder integrated button is released
        encoderButtonDown = false;
        if (!wasLongButtonPress)
        {
            handleEncoderShortButtonPress();
        }
        wasLongButtonPress = false;
        buttonPressDuration = 0;
        lastInteractionTime = millis();
    }

public:

    I2cEncoder()
    {
        // create new i2cEncoderLibV2 class instance which interfaces with the i2c encoder hardware
        encoder_p = new i2cEncoderLibV2(I2C_ENCODER_ADDRESS);
    }

    void setup()
    {
        // configure interrupt pin as input
        pinMode(I2C_ENCODER_INT_PIN, INPUT);
        
        // initialize Wire library for I2C communication
        Wire.begin(I2C_ENCODER_SDA_PIN, I2C_ENCODER_SCL_PIN);

        // set up encoder options
        encoder_p->reset();
        encoder_p->begin(
            i2cEncoderLibV2::INT_DATA
            | i2cEncoderLibV2::WRAP_ENABLE
            | i2cEncoderLibV2::DIRE_RIGHT
            | i2cEncoderLibV2::IPUP_ENABLE
            | i2cEncoderLibV2::RMOD_X1 
            | i2cEncoderLibV2::RGB_ENCODER
        );

        encoder_p->writeCounter(0); /* Reset the counter value */
        encoder_p->writeMax(255);   /* Set the maximum threshold*/
        encoder_p->writeMin(0);     /* Set the minimum threshold */
        encoder_p->writeStep(1);    /* Set the step to 1*/
        encoder_p->writeAntibouncingPeriod(5);
        encoder_p->writeFadeRGB(1); // Fade enabled with 1ms step
        encoder_p->writeInterruptConfig(
            i2cEncoderLibV2::RINC 
            | i2cEncoderLibV2::RDEC 
            | i2cEncoderLibV2::PUSHP 
            | i2cEncoderLibV2::PUSHR
        );

        setEncoderMode(0);
    }

    void loop()
    {
        // check if interrupt pin is low, indicating data is available on I2C bus
        if (digitalRead(I2C_ENCODER_INT_PIN) == LOW)
        {
            // update encoder status by reading i2c bus
            if (encoder_p->updateStatus())
            {
                if (encoder_p->readStatus(i2cEncoderLibV2::RINC) || encoder_p->readStatus(i2cEncoderLibV2::RDEC))
                {
                    encoderRotated(encoder_p);
                }

                if (encoder_p->readStatus(i2cEncoderLibV2::PUSHP)) {
                    encoderButtonPush(encoder_p);
                }

                if (encoder_p->readStatus(i2cEncoderLibV2::PUSHR)) {
                    encoderButtonRelease(encoder_p);
                }
            }
        }

        // check if encoder button is down and continue to count up time it has been down
        if (encoderButtonDown)
        {
            buttonPressDuration = millis() - buttonPressStartTime;
        }

        // check if button press duration exceeds long press threshold
        if (buttonPressDuration > buttonLongPressThreshold)
        {
            handlEncoderLongButtonPress();
        }

        // check for inactivity timeout and return encoder to default mode if it's not there already
        if (encoderMode != 0 && millis() - lastInteractionTime > modeResetTimeout)
        {
            setEncoderMode(0);
        }
    }

};