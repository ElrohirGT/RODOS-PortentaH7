#include <rodos.h> // includes the RODOS API
static Application module01("LED_Test", 2001);

// HAL_GPIO ledBlue(GPIO_007);

class LEDTest : public StaticThread<> { // defines a Thread named HelloWorld
  // void init() { ledBlue.init(true); }
  void run() { // implements the code of the main task of the thread
    // const uint16_t LED_B_Pin = GPIO_PIN_7;
    // const uint16_t LED_B_GPIO_Port = GPIOK;
    //
    // GPIO_InitTypeDef GPIO_InitStructure;
    // GPIO_InitStructure.Pin = LED_B_Pin;
    // GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
    // GPIO_InitStructure.Speed = GPIO_SPEED_LOW;
    // HAL_GPIO_Init(LED_B_GPIO_Port, &GPIO_InitStructure);
    // HAL_GPIO_WritePin(LED_B_GPIO_Port, LED_B_Pin, 1);

    while (1) {
      PRINTF("Hello World!\n"); // print "hello World!"
      // RODOS::Thread::suspendCallerUntil(NOW() + 750 * MILLISECONDS);
      // ledBlue.setPins(1);
      // RODOS::Thread::suspendCallerUntil(NOW() + 750 * MILLISECONDS);
      // ledBlue.setPins(0);
    }
  }
};

LEDTest ledtest;
