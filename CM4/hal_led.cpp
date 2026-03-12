#include <rodos.h> // includes the RODOS API
#include <stm32h7xx.h>
static Application module01("LED_Test", 2002);

class LEDTest : public StaticThread<> { // defines a Thread named HelloWorld
  void run() { // implements the code of the main task of the thread

    // NOTE: GPIO Obtained from FreeRTOS example
    uint16_t LED_B_Pin = GPIO_PIN_7;
    GPIO_TypeDef *LED_B_GPIO_Port = GPIOK;

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.Pin = LED_B_Pin;
    GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructure.Speed = GPIO_SPEED_LOW;
    HAL_GPIO_Init(LED_B_GPIO_Port, &GPIO_InitStructure);
    HAL_GPIO_WritePin(LED_B_GPIO_Port, LED_B_Pin, GPIO_PIN_SET);

    while (1) {
      // PRINTF("Hello World!\n"); // print "hello World!"
      HAL_GPIO_TogglePin(LED_B_GPIO_Port, LED_B_Pin);
      RODOS::Thread::suspendCallerUntil(NOW() + 750 * MILLISECONDS);
    }
  }
};

LEDTest ledtest;
