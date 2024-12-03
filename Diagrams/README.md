The schematic for the PSU and interface is just for the most basic connection using a solid state relay. If you intend to use an SCR board, there may or may not be additional components involved.

If you are using a PWM controlled SCR board, chances are that it already has an opto isolator onboard to protect the original PWM controller that it was designed for. No additional circuity should be needed.

If you are using a voltage controlled SCR board, you will need an optically isolated [PWM to voltage convertor](https://www.amazon.com/dp/B0BG2G5FMX) which will require an additional buck regulator due to its power supply requirements.

_**NOTE:** If you are going to be using an SCR board of any type, you will need uncomment the **SCR_OUT** constant in the ESP32 code in order to enable the 2 KHz PWM output. The default 0.20 Hz PWM used for SSR support is far too slow for use with any SCR board._
