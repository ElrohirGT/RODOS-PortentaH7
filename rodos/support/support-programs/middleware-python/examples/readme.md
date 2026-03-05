
Examples of how Python programs can communicate with RODOS programs (in C++).
============================================================================


The communication between RODOS and Python takes place via the middleware
and the RODOS gateway.
We can use different link interfaces, in these examples we show UDP running
on Linux and Uarts, where RODOS is running on a Discovery board and Python
on a Linux PC.

Not only can middleware messages be exchanged, we have also implemented
a PRINTF interface.  PRINTF strings are converted to middleware messages
and sent through the middleware and gateway.
A Python program (the printer) receives these PRINTF messages (topics)
and displays them locally.  
The PRINTF printer has the ability to classify printf output by threads,
so printfs from different threads can be printed on different xterms.



Please test in this order:

1. examples-mw-printf
2. examples-on-udp
3. examples-discovery-mw-printf

Extra you may take a look to

- examples-python-side
- examples-rodos-side-discovery-baord
- examples-rodos-side-on-linux-udp


