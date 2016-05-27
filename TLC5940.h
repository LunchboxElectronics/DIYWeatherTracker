
#ifndef __TLC5940__
#define __TLC5940__

//#define PIN_BLANK GPIO_Pin_3 //D4 GPIO B
#define PIN_BLANK   GPIO_Pin_4 //D3 GPIO B PinSource 4 Timer3 - on 1, off 4096
#define PIN_GSCLK   GPIO_Pin_6 //D1 GPIO B PinSource 6 Timer4 - PWM Clock, 1 on 1 off

#define PIN_XLAT    GPIO_Pin_3 //D4 GPIO B PinSource3 - pulse XLAT to store channel data

#define BLANK_COUNT 4096
#define GSCLK_FREQ 30000000

#include "application.h"
//include for good measure, even though this works without these
#include <stm32f2xx.h>
#include <stm32f2xx_gpio.h>

#ifdef __cplusplus
extern "C" {
#endif


class TLC5940 {
    
    private:

        //Photon, sysclock is 120mhz. If clock is 30mhz, then prescaled is set to (120 / 30) - 1 = 4 - 1, so 1/4 of 120mhz.
        //Since the main clock pulses on, then one off, then technically it's pulsing at 15mhz.
        const uint8_t GSCLK_PRESCALER = (uint8_t) ((SystemCoreClock / GSCLK_FREQ) - 1);
        static const uint8_t NUM_TLCS = 3;

        void initGSTimer();
        void initBlankTimer();
        void initGPIO();
        void initSPI();

    public:
        //uint8_t needsXlat;

        uint8_t gsData[NUM_TLCS * 24];
        
        TLC5940() {
        };
        
        
        void init();
        void set(uint8_t channel, uint16_t value);
        void update();
        void clear();
        void xlat();
        
};


#ifdef __cplusplus
}
#endif

#endif
