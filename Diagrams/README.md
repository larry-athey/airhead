The schematic for the PSU and interface is just for the most basic connection using a solid state relay. If you intend to use an SCR board, there may or may not be additional components involved.

If you are using a PWM controlled SCR board, chances are that it already has an opto isolator onboard to protect the original PWM controller that it was designed for.

If you are using a voltage controlled SCR board, you will need an optically isolated [PWM to voltage convertor](https://www.amazon.com/dp/B0BG2G5FMX) which will require an additional buck regulator due to its voltage requirements.
