/*
 * Copyright (c) 2022 Uni Wuerzburg, Wuerzburg
 * All rights reserved.
 *
 * @author Konstantin Winkel, Tom Baumann
 */

#include <new>
#include "rodos.h"
#include "misc-rodos-funcs.h"

#include "hal/hal_pwm.h"
#include "em_gpio.h"
#include "em_timer.h"
#include "em_device.h"
#include "em_cmu.h"


#ifndef NO_RODOS_NAMESPACE
namespace RODOS {
#endif

typedef enum
{
    TIM_CHAN0,
    TIM_CHAN1,
    TIM_CHAN2,
    TIM_CHAN3
} TIM_CHAN_TypeDef;

typedef enum{
    TIM1_CHAN0 = 0,
    TIM1_CHAN1,
    TIM1_CHAN2,
    TIM1_CHAN3,
    WTIM0_CHAN0,
    WTIM0_CHAN1,
    WTIM0_CHAN2,
    WTIM1_CHAN0,
    WTIM1_CHAN1,
    WTIM1_CHAN2,
    WTIM1_CHAN3
} HW_TIM_CHAN_Typedef;

//Pin to timer and channel map
static uint16_t P2TCM[54]{
    0b00000011111, //PA0
    0b00000011111, //PA1
    0b00000111111, //PA2
    0b00000111111, //PA3
    0b00001111111, //PA4
    0b00001111111, //PA5
    0b00001110000, //PA6
    0b00001110000, //PA7
    0b00001110000, //PA8
    0b00001110000, //PA9
    0b00001110000, //PB6
    0b00001110000, //PB7
    0b00001110000, //PB8
    0b00001110000, //PB9
    0b00001110000, //PB10
    0b00001111111, //PB11
    0b00011111111, //PB12
    0b00011111111, //PB13
    0b00111111111, //PB14
    0b00111111111, //PB15
    0b01111110000, //PC0
    0b01111110000, //PC1
    0b11111110000, //PC2
    0b11111110000, //PC3
    0b11111110000, //PC4
    0b11111110000, //PC5
    0b11111111111, //PC6
    0b11111111111, //PC7
    0b11111111111, //PC8
    0b11111111111, //PC9
    0b11111111111, //PC10
    0b11111111111, //PC11
    0b11111100000, //PD8
    0b11111101111, //PD9
    0b11111101111, //PD10
    0b11111101111, //PD11
    0b11110001111, //PD12
    0b11110001111, //PD13
    0b11110001111, //PD14
    0b11110001111, //PD15
    0b11110001111, //PF0
    0b11110001111, //PF1
    0b11110001111, //PF2
    0b11110001111, //PF3
    0b11110001111, //PF4
    0b11110001111, //PF5
    0b11110001111, //PF6
    0b11110001111, //PF7
    0b11100000000, //PF8
    0b11100000000, //PF9
    0b11000000000, //PF10
    0b11000000000, //PF11
    0b10000000000, //PF12
    0b10000000000, //PF13
};

struct ReserveOutput{
    bool reserved = false;
    HW_TIM_CHAN_Typedef output;
};

class HAL_PWM_WATCHDOG{
    public:             
        HAL_PWM_WATCHDOG();
                                  //T1C0  T1C1   T1C2   T1C3   WT0C0  WT0C1  WT0C2  WT1C0  WT1C1  WT1C2  WT1C3 
        bool CHANNEL_IN_USE[11] = {false, false, false, false, false, false, false, false, false, false, false};
        ReserveOutput reserveTimerChannel(PWM_IDX idx);
};

HAL_PWM_WATCHDOG::HAL_PWM_WATCHDOG(){}

static HAL_PWM_WATCHDOG pwmWatchdog;

class HW_HAL_PWM{
public:
    HW_HAL_PWM(PWM_IDX idx);

    uint32_t max_counter_increments; // maximum counter value of timer
	uint32_t increments; // Used increments for input duty cycle (max. max_counter_increments)
	uint32_t Frequency; //  Frequency of timer
	uint32_t duty_cycle_in_increments; // current Duty cycle in increments
	float duty_cycle; // current Duty cycle (duty_cycle_in_increments/increments)
	TIM_CHAN_TypeDef channel; // timer channel
    TIMER_TypeDef* timer; // Timer
    CMU_Clock_TypeDef timer_clock;
	uint32_t timerClk; // Timer clk
    uint16_t GPIO_Pin; // Timer gpio pin
    GPIO_Port_TypeDef GPIO_Port; // Timer GPIO port
	int PwmIdx; // IDX of pwm instance

    unsigned long TIMER_ROUTE_LOC;

	void updateSettings(); // update configuration (eg. duty cycle change)

    void PortAndPinFromIDX();
    void setTimerAndChannel(HW_TIM_CHAN_Typedef tc);
    void setTimerRouteLoc(HW_TIM_CHAN_Typedef tc);
};

HAL_PWM::HAL_PWM(PWM_IDX idx) {

	context = new (xmalloc(sizeof(HW_HAL_PWM))) HW_HAL_PWM(idx);
}


int32_t HAL_PWM::init(uint32_t frequency, uint32_t increments) {

    
    if ((static_cast<int>(context->PwmIdx) < static_cast<int>(PWM_IDX00)) || (static_cast<int>(context->PwmIdx) > static_cast<int>(PWM_IDX53))) {
        return -1;
    }

    ReserveOutput result = pwmWatchdog.reserveTimerChannel((PWM_IDX)context->PwmIdx);
    if(result.reserved == false){
        PRINTF("ERROR: NO FREE TIMER FOUND \n");
        return -2;
    }

    context->setTimerAndChannel(result.output);
    context->PortAndPinFromIDX();
    context->setTimerRouteLoc(result.output);

    static char pwmInit[4] = { 0, 0, 0, 0 };

    char timerIdx = 1;

    context->timerClk = CMU_ClockFreqGet(context->timer_clock);
    context->increments = increments;
    context->Frequency = frequency;

	/**** configure GPIO for timer-Use ****/
    CMU_ClockEnable(cmuClock_GPIO, true);
    // Enable clock for TIMER module
    CMU_ClockEnable(context->timer_clock, true);


    // Configure TIMER1 Compare/Capture for output compare
    // Use PWM mode, which sets output on overflow and clears on compare events
    TIMER_InitCC_TypeDef timerCCInit = TIMER_INITCC_DEFAULT;
    timerCCInit.mode = timerCCModePWM;

    TIMER_InitCC(context->timer, context->channel, &timerCCInit);

    context->timer->ROUTELOC0 |=  context->TIMER_ROUTE_LOC;
    
    if(context->timer == TIMER1) timerIdx = 1;
    else if (context->timer == WTIMER0) timerIdx = 2;
    else if (context->timer == WTIMER1) timerIdx = 3;

	/* reset timer before new init, only if it's not already used as PWM source */
	if (pwmInit[timerIdx - 1] == 0) {
		//TIM_DeInit(context->timer);
		pwmInit[timerIdx - 1] = 1;
	}

    GPIO_PinModeSet(context->GPIO_Port, context->GPIO_Pin, gpioModePushPull, 0);

    context->updateSettings();
    // Initialize the timer
    TIMER_Init_TypeDef timerInit = TIMER_INIT_DEFAULT;
    TIMER_Init(context->timer, &timerInit);
    return 0;
}

int32_t HAL_PWM::config(PWM_PARAMETER_TYPE type, int32_t paramVal) {
    if ((static_cast<int>(context->PwmIdx) < static_cast<int>(PWM_IDX00)) || (static_cast<int>(context->PwmIdx) > static_cast<int>(PWM_IDX15))) {
        return -1;
    }

    switch (type) {
	case PWM_PARAMETER_INCREMENTS:{
	    //TODO: Implement
		//context->increments = static_cast<uint32_t>(paramVal);
		break;
	}
	case PWM_PARAMETER_FREQUENCY:
		// TODO: Implement
		//context->Frequency = static_cast<uint32_t>(paramVal);
		break;
	}

	context->updateSettings();
    // Initialize the timer
    TIMER_Init_TypeDef timerInit = TIMER_INIT_DEFAULT;
    TIMER_Init(context->timer, &timerInit);
	return 0;
}

void HAL_PWM::reset() {
	if ((static_cast<int>(context->PwmIdx) < static_cast<int>(PWM_IDX00)) || (static_cast<int>(context->PwmIdx) > static_cast<int>(PWM_IDX15))) {
        return;
    }

	// Reset Timer
	// FIXME: This disables all channels of this timer!
    TIMER_Reset(context->timer);
    GPIO_PinModeSet(context->GPIO_Port, context->GPIO_Pin, gpioModeDisabled, 0);

}

int32_t HAL_PWM::write(uint32_t pulseWidthInIncs) {
	if ((static_cast<int>(context->PwmIdx) < static_cast<int>(PWM_IDX00)) || (static_cast<int>(context->PwmIdx) > static_cast<int>(PWM_IDX53))) {
        return -1;
    }

    pulseWidthInIncs = min(context->increments, pulseWidthInIncs);

	if (pulseWidthInIncs <= 0){ // set output = 0
        switch (context->channel) {
            case TIM_CHAN0:
                context->timer->ROUTEPEN &= ~(WTIMER_ROUTEPEN_CC0PEN);
                break;
            case TIM_CHAN1:
                context->timer->ROUTEPEN &= ~(WTIMER_ROUTEPEN_CC1PEN);
                break;
            case TIM_CHAN2:
                context->timer->ROUTEPEN &= ~(WTIMER_ROUTEPEN_CC2PEN);
                break;
            case TIM_CHAN3:
                if(context->timer != WTIMER0) context->timer->ROUTEPEN &= ~(WTIMER_ROUTEPEN_CC3PEN);
                break;
        }
        return 0;
	}

	context->duty_cycle_in_increments = pulseWidthInIncs;
    context->duty_cycle = static_cast<float>(context->duty_cycle_in_increments) / context->increments;
    uint32_t timer_compare_value = context->duty_cycle*context->max_counter_increments;

    switch (context->channel) {
    case TIM_CHAN0:
        context->timer->ROUTEPEN |= WTIMER_ROUTEPEN_CC0PEN;
        TIMER_CompareBufSet(context->timer, 0, timer_compare_value);
        break;
    case TIM_CHAN1:
        context->timer->ROUTEPEN |= WTIMER_ROUTEPEN_CC1PEN;
        TIMER_CompareBufSet(context->timer, 1, timer_compare_value);
        break;
    case TIM_CHAN2:
        context->timer->ROUTEPEN |= WTIMER_ROUTEPEN_CC2PEN;
        TIMER_CompareBufSet(context->timer, 2, timer_compare_value);
        break;
    case TIM_CHAN3:
        if(context->timer != WTIMER0){
            context->timer->ROUTEPEN |= WTIMER_ROUTEPEN_CC3PEN;
            TIMER_CompareBufSet(context->timer, 3, timer_compare_value);
        }
        break;
    }

	upCallWriteFinished();

	return 0;
}

HW_HAL_PWM::HW_HAL_PWM(PWM_IDX idx){
    increments = 2;
    duty_cycle_in_increments = 1;
    Frequency = 0;
    PwmIdx = -1;
    timer = NULL;
    channel = TIM_CHAN1;
    timerClk = 0;
    GPIO_Pin = 0;


    if ((static_cast<unsigned int>(idx) > static_cast<unsigned int>(PWM_IDX53))) {
        RODOS_ERROR("Invalid PWM Index\n");
        return;
    }

    PwmIdx = idx; 
}

void HW_HAL_PWM::updateSettings(){
    if (Frequency == 0){
        // End, so that output is 0
        return;
    }else{
        // normal pwm mode
    }
    // Set top value to overflow at the desired PWM_FREQ frequency
    TIMER_TopSet(timer, CMU_ClockFreqGet(timer_clock) / Frequency);
    max_counter_increments = TIMER_TopGet(timer);

    increments = min(max_counter_increments, increments);

    duty_cycle_in_increments = 0; // default duty cycle = minimum = 0
    duty_cycle = static_cast<float>(duty_cycle_in_increments) / increments;
    uint32_t timer_compare_value = duty_cycle*static_cast<float>(max_counter_increments);

    switch (channel) {
    case TIM_CHAN0:
        // Set compare value for initial duty cycle
        TIMER_CompareSet(timer, 0, timer_compare_value);
        break; // channel 1 init
    case TIM_CHAN1:
        TIMER_CompareSet(timer, 1, timer_compare_value);
        break; // channel 2 init
    case TIM_CHAN2:
        TIMER_CompareSet(timer, 2, timer_compare_value);
        break; // channel 3 init
    case TIM_CHAN3:
        if(timer != WTIMER0) TIMER_CompareSet(timer, 3, timer_compare_value);
        break; // channel 4 init
    default:
        return;
    }

}

void HW_HAL_PWM::setTimerAndChannel(HW_TIM_CHAN_Typedef tc){

    if(tc < 4){
        timer = TIMER1;
        timer_clock = cmuClock_TIMER1;
        channel = (TIM_CHAN_TypeDef) tc;
        return;
    }

    if(tc < 7){
        timer = WTIMER0;
        timer_clock = cmuClock_WTIMER0;
        channel = (TIM_CHAN_TypeDef)(tc - 4);
        return;
    }

    if(tc < 11){
        timer = WTIMER1;
        timer_clock = cmuClock_WTIMER1;
        channel = (TIM_CHAN_TypeDef)(tc - 7);
        return;
    }
}

void HW_HAL_PWM::PortAndPinFromIDX(){
    if(PwmIdx < 10){
        GPIO_Port = gpioPortA;
        GPIO_Pin = PwmIdx;
        return;
    }

    if(PwmIdx < 20){
        GPIO_Port = gpioPortB;
        GPIO_Pin = PwmIdx - 10 + 6; //PB starts at 6
        return;
    }

    if(PwmIdx < 32){
        GPIO_Port = gpioPortC;
        GPIO_Pin = PwmIdx - 20;
        return;
    }

    if(PwmIdx < 40){
        GPIO_Port = gpioPortD;
        GPIO_Pin = PwmIdx - 32 + 8; //PD starts at 8
        return;
    }

    if(PwmIdx < 53){
        GPIO_Port = gpioPortF;
        GPIO_Pin = PwmIdx - 40;
    }
}

void HW_HAL_PWM::setTimerRouteLoc(HW_TIM_CHAN_Typedef tc){
    unsigned long  LOC;
    uint8_t shift;

    switch(tc){
        case TIM1_CHAN0:  LOC = 0;  shift = 0;  break;
        case TIM1_CHAN1:  LOC = 31; shift = 8;  break;
        case TIM1_CHAN2:  LOC = 30; shift = 16; break;
        case TIM1_CHAN3:  LOC = 29; shift = 24; break;
        case WTIM0_CHAN0: LOC = 0;  shift = 0;  break;
        case WTIM0_CHAN1: LOC = 0;  shift = 8;  break;
        case WTIM0_CHAN2: LOC = 0;  shift = 16; break;
        case WTIM1_CHAN0: LOC = 0;  shift = 0;  break;
        case WTIM1_CHAN1: LOC = 0;  shift = 8;  break;
        case WTIM1_CHAN2: LOC = 0;  shift = 16; break;
        case WTIM1_CHAN3: LOC = 0;  shift = 24; break;
    }

    for(int i = 0; i < PwmIdx; i++){
        if(getbit(P2TCM[i], tc)){
            LOC++;
            if(LOC >= 32) LOC = 0;
        }
    }

    TIMER_ROUTE_LOC = LOC << shift;
}

ReserveOutput HAL_PWM_WATCHDOG::reserveTimerChannel(PWM_IDX idx){
    ReserveOutput result;
    uint32_t currentPinMap = P2TCM[idx];
    
    for(int i = 0; i < 11; i++){

        if(getbit(currentPinMap, i) == true && CHANNEL_IN_USE[i] == false){
            CHANNEL_IN_USE[i] = true;
            result.reserved = true;
            result.output = (HW_TIM_CHAN_Typedef) i;    
            break;
        }
    }

    return result;
}

#ifndef NO_RODOS_NAMESPACE
}
#endif
