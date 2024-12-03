# Airhead
Air Still (or clone) upgrade that uses an SCR controller or SSR for heating element control and an ESP32 to make the whole thing smarter. Eliminates that constant full power on/off switching to the heating element and the possibility of scorching. Plus even more capabilities.

You may contact me directly at https://panhandleponics.com<br>
Subscribe to the official YouTube channel at https://www.youtube.com/@PanhandlePonics<br>

_**SIDE NOTE:** Save your money and just get a [$65 Air Still clone](https://www.amazon.com/VEVOR-Distiller-Countertop-Distilled-Stainless/dp/B0CN2SBJLT/) water distiller for this project. Add that to the cost to build this or purchase a kit from my website, and it still comes in less expensive than an original Still Spirits Air Still with the temperature control problems._

_**This project began on October 22, 2024 and does not yet have an official release...**_<br><br>

Aside from Still Spirits brand Air Stills only being sold as an Ethanol Version and a Water Version, they both suffer the same design flaw where the temperature is managed the same way it is with an electric cooking appliance. Even the various Air Still clones out there work the same way, including the ones with adjustable temperature. They send full power to the heating element when heating up, shut it off when they're up to temperature, then repeat this cycle.

If you have any suspended solids in your wash, they will scorch and fuse to the bottom as they fall out of solution. This will add foul flavors to anything that you're distilling, especially gin. As with my [RPi-Smart-Still](https://github.com/larry-athey/rpi-smart-still) project, this problem is solved by replacing the relays or snap switches on the heating element with an SCR controller or an SSR running as a basic PID controller _(temperature controlled 0.20 Hz PWM)_.

As long as the heating element is always hotter than the liquid, heat will radiate upward and force most of the suspended solids to stay afloat. This will eliminate the possibility of scorching for the most part.

This project aims to solve that problem and add three user selectable modes:

1. Operate at a constant user selectable power level (10% to 100%)
2. Operate at a constant user selectable temperature (up to 100C/212F)
3. Perform timed runs with a user selectable starting and ending temperature
4. _Perhaps additional features will be added as time goes by_

This controller works with any (1) PWM controlled SCR board, or (2) voltage controlled SCR board, or (3) zero-crossing trigger [solid state relay](https://www.amazon.com/gp/product/B08FR13GYR/). This controller puts out PWM, but you can use any opto-isolated [PWM to voltage convertor](https://www.amazon.com/dp/B0BG2G5FMX) to connect it to any voltage controlled SCR board _(one that has a knob to control power)_.

It's totally up to you which one to use. The SSR is easiest to set up and doesn't require a heat sink if the load is less than 1200 watts _(the heating element used here is only 700 watts)_.

The brains of the operation is a [LilyGo T-Display-S3](https://www.amazon.com/gp/product/B0BLNKPL25/) ESP32 microcontroller with a built-in capacitive touch color LCD display. The boiler control code is a port of the code from my [RPi-Smart-Still](https://github.com/larry-athey/rpi-smart-still) project. This means that even though this is a newer project, the code has already been refined and fine tuned for 2 years.

Refer to the contents of the **/Diagrams/** and **/Instructions/** directories here to learn how to build this project.<br><br>

<img width="1024" src="https://github.com/user-attachments/assets/5718c5b2-e66c-4fb2-8cd2-e12219879c87"><br><br>
<img width="1024" src="https://github.com/user-attachments/assets/0a62e45b-1fc2-4e6d-bd1d-f9f94309f22b"><br><br>
<img width="1024" src="https://github.com/user-attachments/assets/bc287f2c-4181-4734-afa7-103756f2cf4e"><br><br>
<img width="1024" src="https://github.com/user-attachments/assets/df7dcee3-0d0e-4c08-ac75-7d823ccc1b14"><br><br>
<img width="1024" src="https://github.com/user-attachments/assets/52e759e5-1a44-4c1b-a983-03b44a5fd17b"><br>

