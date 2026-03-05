/*
 * hal_pwm.h
 *
 *  Created on: 21.06.2012
 *      Author: Michael Ruffer, Thomas Walter
 */
#pragma once

#include "hal/genericIO.h"


namespace RODOS {

enum PWM_PARAMETER_TYPE {
	PWM_PARAMETER_FREQUENCY,
	PWM_PARAMETER_INCREMENTS
};

enum PWM_IDX {      // STM32F4                  AVR32                   EFR32FG12
	PWM_IDX00 = 0,  // Timer 1 CH1 - PE9        PWM0 -                  PA0
	PWM_IDX01,      // Timer 1 CH2 - PE11       PWM1 -                  PA1
	PWM_IDX02,      // Timer 1 CH3 - PE13       PWM2 - Pin PB21         PA2
	PWM_IDX03,      // Timer 1 CH4 - PE14       PWM3 - Pin PB22         PA3
	PWM_IDX04,      // Timer 2 CH1 - PA0        PWM4 -                  PA4
	PWM_IDX05,      // Timer 2 CH2 - PA1        PWM5 -                  PA5
	PWM_IDX06,      // Timer 2 CH3 - PA2        PWM6 -                  PA6 -not availible
	PWM_IDX07,      // Timer 2 CH4 - PA3                                PA7 -not availible
	PWM_IDX08,      // Timer 3 CH1 - PA6                                PA8 -not availible
	PWM_IDX09,      // Timer 3 CH2 - PC7                                PA9 -not availible
	PWM_IDX10,      // Timer 3 CH3 - PB0                                PB6	-not availible
	PWM_IDX11,      // Timer 3 CH4 - PB1                                PB7	
	PWM_IDX12,      // Timer 4 CH1 - PB6                                PB8
	PWM_IDX13,      // Timer 4 CH2 - PB7                                PB9
	PWM_IDX14,      // Timer 4 CH3 - PB8                                PB10
	PWM_IDX15,      // Timer 4 CH4 - PB9                                PB11
	PWM_IDX16,   	//                                                  PB12 -UART
	PWM_IDX17,    	//                                                  PB13 -UART
	PWM_IDX18,    	//                                                  PB14 -Crystal
	PWM_IDX19,   	//                                                  PB15 -Crystal
	PWM_IDX20,   	//                                                  PC0
	PWM_IDX21,   	//                                                  PC1
	PWM_IDX22,   	//                                                  PC2
	PWM_IDX23,   	//                                                  PC3
	PWM_IDX24,   	//                                                  PC4
	PWM_IDX25,   	//                                                  PC5
	PWM_IDX26,   	//                                                  PC6
	PWM_IDX27,   	//                                                  PC7
	PWM_IDX28,   	//                                                  PC8
	PWM_IDX29,   	//                                                  PC9
	PWM_IDX30,   	//                                                  PC10
	PWM_IDX31,   	//                                                  PC11
	PWM_IDX32,   	//                                                  PD8
	PWM_IDX33,   	//                                                  PD9
	PWM_IDX34,   	//                                                  PD10
	PWM_IDX35,   	//                                                  PD11
	PWM_IDX36,   	//                                                  PD12 -I2C
	PWM_IDX37,   	//                                                  PD13 -I2C	
	PWM_IDX38,   	//                                                  PD14
	PWM_IDX39,   	//                                                  PD15
	PWM_IDX40,   	//                                                  PF0
	PWM_IDX41,   	//                                                  PF1
	PWM_IDX42,   	//                                                  PF2
	PWM_IDX43,   	//                                                  PF3
	PWM_IDX44,   	//                                                  PF4
	PWM_IDX45,   	//                                                  PF5
	PWM_IDX46,   	//                                                  PF6
	PWM_IDX47,   	//                                                  PF7
	PWM_IDX48,   	//                                                  PF8
	PWM_IDX49,   	//                                                  PF9
	PWM_IDX50,   	//                                                  PF10
	PWM_IDX51,   	//                                                  PF11 -not availible
	PWM_IDX52,   	//                                                  PF12 -not availible
	PWM_IDX53,   	//                                                  PF13 -not availible
};



class HW_HAL_PWM;

class HAL_PWM : public GenericIOInterface { ///< one object per pwm

	HW_HAL_PWM* context;

public:
	HAL_PWM(PWM_IDX idx);

	/**
	 * Initialize PWM without starting it -> start with "write(N)"
	 * To get exact PWM frequency you must choose frequency and increments so that timerclock/frequency/increments = integer (no fraction)
     * e.g. STM32F4 Timer 1: 168Mhz/1000/100 = 1680 :), but 168Mhz/1000/128 = 1315.5 -> PWM freq = 168MHz/1315/128 = 998.1Hz :(
     *
     * STM32F4  : - Tim1 (16 bit,168MHz), Tim2 (32 bit,84MHz), Tim3 (16 bit,84MHz), Tim4 (16 bit,84MHz)
     *            - PWMs of the same timer can only run with same settings
     * AVR32    : all PWMs are fixed to frequency = 125.9 Hz, increments = 512 !!!
	 *
	 * @param int frequency - frequency of PWM signal
	 * @param int increments - one PWM period is divided in N increments
	 */
	int32_t init(uint32_t frequency = 1000, uint32_t increments = 100);

	int32_t config(PWM_PARAMETER_TYPE type, int32_t paramVal = 0);

	void reset();

    bool isWriteFinished() 	{ return true; }

    /*** Extra only for PWM **/
    /**
     * Set pulse width of PWM signal
     * - write(increments/2) -> duty cycle 50%
     * - write(0) -> stop PWM-output
     * @param int pulseWidthInIncs
     */
    int32_t write(uint32_t pulseWidthInIncs);			// 0 ... +increments

};


}
