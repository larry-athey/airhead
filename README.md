# Airhead
Air Still (or clone) upgrade that uses an SCR controller or SSR for the heating element and an ESP32 to make the whole thing smarter. Eliminates that constant full power on/off switching to the heating element and the possibility of scorching. Plus even more capabilities.

You may contact me directly at https://panhandleponics.com<br>
Subscribe to the official YouTube channel at https://www.youtube.com/@PanhandlePonics<br>

_**SIDE NOTE:** Save your money and just get a [$65 Air Still clone](https://www.amazon.com/VEVOR-Distiller-Countertop-Distilled-Stainless/dp/B0CN2SBJLT/) water distiller for this project._

_**This project began on October 22, 2024 and does not yet have an official release...**_<br><br>

Aside from Still Spirits brand Air Stills only being sold as an Ethanol Version and a Water Version, they both suffer the same design flaw where the temperature is managed the same way it is with an electric cooking appliance. Even the various Air Still clones out there work the same way, including the ones with adjustable temperature. They send full power to the heating element when heating up, shut it off when they're up to temperature, then repeat this cycle.

If you have any suspended solids in your wash, they will scorch and fuse to the bottom as they fall out of solution. This will add foul flavors to anything that you're distilling, especially gin. As with my [RPi-Smart-Still](https://github.com/larry-athey/rpi-smart-still) project, this is solved by replacing the relays or snap switches on the heating element with an SCR controller. As long as the heating element is always on at some level, heat will radiate upward and force most of the suspended solids to stay afloat. This will eliminate the possibility of scorching for the most part.

This project aims to solve that problem and add three user selectable modes:

1. Operate at a constant user selectable power level (1% to 100%)
2. Operate at a constant user selectable temperature (up to 100C/212F)
3. Perform timed runs with a user selectable starting and ending temperature
4. _Perhaps additional features will be added as time goes by_

This controller works with any (1) PWM controlled SCR board, or (2) voltage controlled SCR board, or (3) zero-crossing-trigger solid [state relay](https://www.amazon.com/gp/product/B08FR13GYR/). This controller puts out PWM, but you can use any opto-isolated [PWM to voltage convertor](https://www.amazon.com/dp/B0BG2G5FMX) to connect it to any voltage controlled SCR board _(one that has a knob to control power)_.

The brains of the operation is a [LilyGo T-Display-S3](https://www.amazon.com/gp/product/B0BLNKPL25/) ESP32 microcontroller with a built-in capacitive touch color LCD display. These ESP32 units are available with a case or you can buy them separately, there's no need for any 3D printing. The only wires coming out of the case are 4 wires that connect to an internal power supply in the bottom of the still. The USB-C port for the ESP32 is accessible for software updates without the need to take anything apart. Refer to the contents in the **/Instructions/** directory here for more information.

<img width="1024" src="https://github.com/user-attachments/assets/f4c37f88-75f2-4c23-9678-8c19a809ded0"><br><br>
<img width="1024" src="https://github.com/user-attachments/assets/57a78964-2d74-4d1c-a1bd-7bb5481609a5"><br><br>
<img width="1024" src="https://github.com/user-attachments/assets/d9d7145a-6dce-40cc-be6c-75748c23297c"><br><br>
<img width="1024" src="https://github.com/user-attachments/assets/83c0b198-6b32-4c0a-a362-4d68cfe766bc">
