
#include "TLC5940.h"

void TLC5940::init() {
    clear();
    initGPIO();
    initBlankTimer();
    initGSTimer();
    initSPI();
}


void TLC5940::initGPIO() {
    
    //enable GPIO B clock
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

    //Set up XLAT pin for GPIO commands
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Pin = PIN_XLAT; 
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    //Blank as Alternate function (User the timer to pulse once every 4096 counts)
    //GSCLK as Alternate function (this is going to be a PWM off timer4)
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Pin = PIN_BLANK | PIN_GSCLK;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource4, GPIO_AF_TIM3);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_TIM4);
}

void TLC5940::initBlankTimer() {
    
    //blank timer uses timer3
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    
    //counts up to 4096 period in the same scaler as the gsclk (twice, as the clock counts one, then pulses one)
    TIM_TimeBaseInitTypeDef timerInitStructure;
    timerInitStructure.TIM_Prescaler = GSCLK_PRESCALER;
	timerInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	timerInitStructure.TIM_Period = BLANK_COUNT * 2;
	timerInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	timerInitStructure.TIM_RepetitionCounter = 0;

	TIM_TimeBaseInit(TIM3, &timerInitStructure);
	TIM_Cmd(TIM3, ENABLE);
	
	//pulse that blank pin one the count - 1 is reached
	TIM_OCInitTypeDef outputChannelInit = {0,};
    outputChannelInit.TIM_OCMode = TIM_OCMode_PWM1;
    outputChannelInit.TIM_Pulse = BLANK_COUNT * 2 - 1;
    outputChannelInit.TIM_OutputState = TIM_OutputState_Enable;
    outputChannelInit.TIM_OCPolarity = TIM_OCPolarity_Low;
 
    TIM_OC1Init(TIM3, &outputChannelInit);
    TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable);
}

void TLC5940::initGSTimer() {
    
    //enable GSCLK on timer4
    //will pulse pin ever second clock
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
    
    TIM_TimeBaseInitTypeDef timerInitStructure;
    timerInitStructure.TIM_Prescaler = GSCLK_PRESCALER;
	timerInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	timerInitStructure.TIM_Period = 1;
	timerInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	timerInitStructure.TIM_RepetitionCounter = 0;

	TIM_TimeBaseInit(TIM4, &timerInitStructure);
	TIM_Cmd(TIM4, ENABLE);
	
	//pulse that clock
	TIM_OCInitTypeDef outputChannelInit = {0,};
    outputChannelInit.TIM_OCMode = TIM_OCMode_PWM1;
    outputChannelInit.TIM_Pulse = 1;
    outputChannelInit.TIM_OutputState = TIM_OutputState_Enable;
    outputChannelInit.TIM_OCPolarity = TIM_OCPolarity_High;
 
    TIM_OC1Init(TIM4, &outputChannelInit);
    TIM_OC1PreloadConfig(TIM4, TIM_OCPreload_Enable);
 
}

void TLC5940::initSPI() {
    //can't use STM32 setup of SPI, so just using normal SPI
    //MODE3 gives the most stable output, where as 0 is the most erractic
    SPI.begin();
    SPI.setBitOrder(MSBFIRST);
    SPI.setDataMode(SPI_MODE3);
    //The bigger the clock div, the more stable the output
    SPI.setClockDivider(SPI_CLOCK_DIV16);
}

void TLC5940::clear() {
    //clear all the pointers to 0
    for (uint8_t* p = gsData; p < gsData + NUM_TLCS * 24; p++) {
        *p = 0x00;
    }
}

void TLC5940::set(uint8_t channel, uint16_t val) {
    //the location of the first byte
    uint8_t* startByte = gsData + ((((uint16_t)channel) * 3) >> 1);

    if (channel & 1) {
        //channel is odd, 4 LSB in first byte, 8 MSB in second byte
        *startByte = (*startByte & 0x0F) | (val << 4);
        *(++startByte) = val >> 4; 
    } else {
        //channel is even, first 8 bits are stored in startbyte, 4 MSB in second byte
        *(startByte++) = val;
        *startByte = (*startByte & 0xF0) | (val >> 8);
    }

}

void TLC5940::update() {
    //for each value, send it over the SPI (was going to turn this into a pointer loop)
    for (int i = NUM_TLCS * 24 - 1; i >= 0; i--) {
        SPI.transfer(gsData[i]);
    }
    xlat();
}

void TLC5940::xlat() {
    //the XLAT write function. The microseconds delay needs tweaking
    GPIO_WriteBit(GPIOB, PIN_XLAT, Bit_SET);
    delayMicroseconds(4);
    GPIO_WriteBit(GPIOB, PIN_XLAT, Bit_RESET);
}


