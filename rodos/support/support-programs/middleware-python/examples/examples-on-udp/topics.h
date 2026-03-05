#pragma once
#include <stdint.h>

// ########## The Python interface shall refer to this data and definitions

const int32_t  udpPortNr       = -50000; // < 0 means allow multiple receivers
const uint32_t topicIdPosition = 1002;
const uint32_t topicIdMyTime   = 1003;


// data from linux to rodos
struct Position {  // python struct def  "20siddd" . see https://docs.python.org/3/library/struct.html
  char name[20];
  int32_t cnt;
  double x, y, z;
};

// data rodos to linux
struct MyTime {  // python struct def  "qi" . see https://docs.python.org/3/library/struct.html
  int64_t timeNow;
  int32_t msgIndex;
};


