#include <rodos.h> // includes the RODOS API
static Application module01("LED_Test", 2003);

HAL_GPIO ledBlue(GPIO_167);

class LEDTest : public StaticThread<> { // defines a Thread named HelloWorld
  void init() { ledBlue.init(true); }
  void run() { // implements the code of the main task of the thread

    while (1) {
      RODOS::Thread::suspendCallerUntil(NOW() + 750 * MILLISECONDS);
      ledBlue.setPins(1);
      RODOS::Thread::suspendCallerUntil(NOW() + 750 * MILLISECONDS);
      ledBlue.setPins(0);
    }
  }
};

LEDTest ledtest;
