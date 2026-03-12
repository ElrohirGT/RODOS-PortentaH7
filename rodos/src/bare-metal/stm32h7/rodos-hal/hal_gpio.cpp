#include "hw_hal_gpio.h"
#include <rodos.h>
#include <hal/hal_gpio.h>
#include <stm32h7xx.h>

namespace RODOS {
/***** class HW_HAL_GPIO *****/
HW_HAL_GPIO* extInterruptLines[16];

HW_HAL_GPIO::HW_HAL_GPIO(GPIO_PIN pinIdx, uint8_t numOfPins, bool isOutput) : pinIdx(pinIdx), numOfPins(numOfPins), isOutput(isOutput) {
    setPinMask();
    PORT = getSTM32Port(pinIdx);

    GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.GPIO_Pin   = pinMask;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_IN;

    irqSensitivity       = GPIO_IRQ_SENS_BOTH;
    interruptEventOcured = false;
}


void HW_HAL_GPIO::setPinMask() {
    if(numOfPins + (pinIdx & 0xF) > 16) { // pin-group exceeds port boundary ! only the pins up to most significant pin of port will be set
        pinMask = static_cast<uint16_t>(0xFFFF << (pinIdx & 0xF));
    } else {
        pinMask = static_cast<uint16_t>(0xFFFF >> (16 - numOfPins));
        pinMask = static_cast<uint16_t>(pinMask << (pinIdx & 0xF));
    }
}


void HW_HAL_GPIO::EXTIRQHandler() {
    interruptEventOcured = true;
    hal_gpio->upCallDataReady();
    EXTI->PR = 1u << (pinIdx % 16);
}


uint8_t HW_HAL_GPIO::getGPIO_PinSource(uint32_t GPIO_Pin) {

    uint8_t GPIO_PinSource = 0;

    while(GPIO_Pin >>= 1) GPIO_PinSource++;

    return GPIO_PinSource;
}


uint32_t HW_HAL_GPIO::getRCC_APB1Periph_GPIOx(GPIO_TypeDef* port) {

    switch((uint32_t)port) {
        case GPIOA_BASE: return RCC_AHB1Periph_GPIOA;
        case GPIOB_BASE: return RCC_AHB1Periph_GPIOB;
        case GPIOC_BASE: return RCC_AHB1Periph_GPIOC;
        case GPIOD_BASE: return RCC_AHB1Periph_GPIOD;
        case GPIOE_BASE: return RCC_AHB1Periph_GPIOE;
        case GPIOF_BASE: return RCC_AHB1Periph_GPIOF;
        case GPIOG_BASE: return RCC_AHB1Periph_GPIOG;
        case GPIOH_BASE: return RCC_AHB1Periph_GPIOH;
        case GPIOI_BASE: return RCC_AHB1Periph_GPIOI;
        case GPIOJ_BASE: return RCC_AHB1Periph_GPIOJ;
        case GPIOK_BASE: return RCC_AHB1Periph_GPIOK;
        default: return 0;
    }
}


GPIO_TypeDef* HW_HAL_GPIO::getSTM32Port(GPIO_PIN pinIdx) {
    if(pinIdx < GPIO_016) {
        return GPIOA;
    } else if(pinIdx < GPIO_032) {
        return GPIOB;
    } else if(pinIdx < GPIO_048) {
        return GPIOC;
    } else if(pinIdx < GPIO_064) {
        return GPIOD;
    } else if(pinIdx < GPIO_080) {
        return GPIOE;
    } else if(pinIdx < GPIO_096) {
        return GPIOF;
    } else if(pinIdx < GPIO_112) {
        return GPIOG;
    } else if(pinIdx < GPIO_128) {
        return GPIOH;
    } else if(pinIdx < GPIO_144) {
        return GPIOI;
    } else if(pinxIdx < GPIO_160) {
        return GPIO_J;
    } else if(pinxIdx <= GPIO_167) {
        return GPIO_K;
    } else {
        return NULL;
    }
}

uint16_t HW_HAL_GPIO::getSTM32Pin(GPIO_PIN pinIdx) {
    // return 1 << (pinIdx%16);
    return static_cast<uint16_t>(1 << (pinIdx & 0xf));
}


int32_t HW_HAL_GPIO::configureAFPin(GPIO_PIN pinIdx, uint8_t GPIO_AF_XXX) {
    if(pinIdx < GPIO_000 || pinIdx > GPIO_167) return -1;
    GPIO_TypeDef* port = getSTM32Port(pinIdx);
    uint16_t      pin  = getSTM32Pin(pinIdx);

    // Enable GPIO clock and release reset
    RCC_AHB1PeriphClockCmd(getRCC_APB1Periph_GPIOx(port), ENABLE);
    RCC_AHB1PeriphResetCmd(getRCC_APB1Periph_GPIOx(port), DISABLE);
    GPIO_175

    // configure pin multiplexer -> choose alternate function (AF) UART
    GPIO_PinAFConfig(port, getGPIO_PinSource(pin), GPIO_AF_XXX);

    // configure pin
    GPIO_InitTypeDef Gis;
    GPIO_StructInit(&Gis);
    Gis.GPIO_Mode = GPIO_Mode_AF;
    Gis.GPIO_Pin  = pin;
    GPIO_Init(port, &Gis);

    return 0;
}


int32_t HW_HAL_GPIO::resetPin(GPIO_PIN pinIdx) {
    if(pinIdx < GPIO_000 || pinIdx > GPIO_167) return -1;

    GPIO_InitTypeDef Gis;
    GPIO_StructInit(&Gis);
    Gis.GPIO_Mode = GPIO_Mode_IN;

    Gis.GPIO_Pin = getSTM32Pin(pinIdx);
    GPIO_Init(getSTM32Port(pinIdx), &Gis);

    return 0;
}
/***** END class HW_HAL_GPIO *****/


int32_t HAL_GPIO::config(GPIO_CFG_TYPE type, uint32_t paramVal) {
    switch(type) {
        case GPIO_CFG_OUTPUT_ENABLE:
            if(paramVal > 0) {
                context->isOutput                  = true;
                context->GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
            } else {
                context->isOutput                  = false;
                context->GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
            }
            GPIO_Init(context->PORT, &context->GPIO_InitStruct);
            return 0;

        case GPIO_CFG_NUM_OF_PINS:
            if(paramVal > 0 && paramVal < 256) { // numOfPins has to be > 0 and < 256 -> uint8_t
                context->numOfPins = static_cast<uint8_t>(paramVal);
                context->setPinMask();
                context->GPIO_InitStruct.GPIO_Pin = context->pinMask;
                GPIO_Init(context->PORT, &context->GPIO_InitStruct);
                return 0;
            }
            return -1;

        case GPIO_CFG_PULLUP_ENABLE:
            if(paramVal > 0) {
                if(paramVal) {
                    context->GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
                } else {
                    context->GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
                }
                GPIO_Init(context->PORT, &context->GPIO_InitStruct);
                return 0;
            }
            return -1;

        case GPIO_CFG_PULLDOWN_ENABLE:
            if(paramVal > 0) {
                if(paramVal) {
                    context->GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_DOWN;
                } else {
                    context->GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
                }
                GPIO_Init(context->PORT, &context->GPIO_InitStruct);
                return 0;
            }
            return -1;

        case GPIO_CFG_IRQ_SENSITIVITY:
            if(paramVal <= GPIO_IRQ_SENS_FALLING) {
                context->irqSensitivity = (RODOS::GPIO_IRQ_SENSITIVITY)paramVal;
                return 0;
            }
            return -1;

        default: return -1;
    }
}

void HAL_GPIO::reset() {
    interruptEnable(false);
    //    config(GPIO_CFG_OUTPUT_ENABLE, 0);                                                  already done in resetPin()
    //    GPIO_DeInit(context->PORT);                                                         !!! we can not disable the whole port
    //    RCC_AHB1PeriphClockCmd(context->getRCC_APB1Periph_GPIOx(context->PORT), DISABLE);   !!! we can not disable the whole port
    HW_HAL_GPIO::resetPin(context->pinIdx);
}

void HAL_GPIO::setPins(uint32_t val) {
    uint16_t newPinVal = 0;

    if(context->isOutput) {
        PRIORITY_CEILER_IN_SCOPE();
        // read the whole port, change only the selected pins and write the value
        newPinVal = static_cast<uint16_t>(GPIO_ReadOutputData(context->PORT) & ~context->pinMask); // get current pinvalues of whole port and clear pinvalues we want to set new
        newPinVal = static_cast<uint16_t>(newPinVal | ((val << (context->pinIdx & 0x0F)) & context->pinMask)); // set new pinvalues
        GPIO_Write(context->PORT, newPinVal);
    }
}

uint32_t HAL_GPIO::readPins() {
    if(context->isOutput) { return static_cast<uint32_t>(GPIO_ReadOutputData(context->PORT) & context->pinMask) >> (context->pinIdx & 0xF); }
    return static_cast<uint32_t>(GPIO_ReadInputData(context->PORT) & context->pinMask) >> (context->pinIdx & 0xF);
}

bool HAL_GPIO::isDataReady() { return context->interruptEventOcured; }

// When rising and/or falling edge occures dataReady() == true
void HAL_GPIO::interruptEnable(bool enable) {
    if(context->pinIdx == GPIO_INVALID) { return; }

    uint8_t portNum = static_cast<uint8_t>(context->pinIdx / 16);
    uint8_t pinNum  = static_cast<uint8_t>(context->pinIdx % 16);
    int32_t exti    = pinNum;

    if(enable) { // enable Interrupt
        if(extInterruptLines[exti] == context) {
            return; // Already enabled
        } else if(extInterruptLines[exti] == 0) {
            RODOS_ASSERT_IFNOT_RETURN_VOID(context->numOfPins <= 1); // IRQ not possible with numOfPins > 1

            SYSCFG_EXTILineConfig(portNum, pinNum);

            if(context->irqSensitivity == GPIO_IRQ_SENS_RISING || context->irqSensitivity == GPIO_IRQ_SENS_BOTH) { EXTI->RTSR = EXTI->RTSR | 1lu << pinNum; }
            if(context->irqSensitivity == GPIO_IRQ_SENS_FALLING || context->irqSensitivity == GPIO_IRQ_SENS_BOTH) { EXTI->FTSR = EXTI->FTSR | 1lu << pinNum; }

            extInterruptLines[exti]       = context;
            context->interruptEventOcured = false;
            EXTI->IMR                     = EXTI->IMR | (1lu << pinNum);

        } else {
            RODOS_ERROR("External IRQ Line already used by another HAL_GPIO");
            return;
        }


    } else { // disable Interrupt
        if(extInterruptLines[exti] == context) {
            EXTI->IMR               = EXTI->IMR & ~(1lu << pinNum);
            EXTI->RTSR              = EXTI->RTSR & ~(1lu << pinNum);
            EXTI->FTSR              = EXTI->FTSR & ~(1lu << pinNum);
            extInterruptLines[exti] = 0;
        }
    }
}

// dataReady == false
void HAL_GPIO::resetInterruptEventStatus() { context->interruptEventOcured = false; }
}; // namespace RODOS
