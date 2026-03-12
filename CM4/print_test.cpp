#include <rodos.h> // includes the RODOS API
static Application module01("PRINT_Test", 2001);

class PRINTTest : public StaticThread<> { // defines a Thread named HelloWorld
  void run() { // implements the code of the main task of the thread

    PRINTF("Setting up PRINT test:\n"); // print "hello World!"
    uint64_t iter = 0;
    while (1) {
      PRINTF("Iter: %llu\n", iter);
      iter++;
    }
  }
};

PRINTTest ledtest;
