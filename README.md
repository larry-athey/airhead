# Airhead
Air Still (or clone) upgrade that uses an SCR controller for the heating element and an ESP32 to make the whole thing smarter. Eliminates that constant full power on/off switching to the heating element and the possibility of scorching. Plus even more capabilities.

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

This project works with a very common SCR controller _(shown below)_ that is available from numerous Amazon sellers. This project uses an ESP32 with a built-in touch screen display to replace the interface that comes with their unit. Here is a [link](https://www.amazon.com/gp/product/B076VKJM42/) to this unit, but please feel free to shop around to see if others are selling it for a lower price, I got mine for $20. They're all the same, no matter what the brand name is. There's a 4 conductor ribbon cable that plugs into the controller interface, so you can just unplug their interface and put it back in the box.

<img width="1024" src="https://github.com/user-attachments/assets/7daa8ebd-7aca-4462-8457-f68c4f77a3b1"><br>

The brains of the operation is a [LilyGo T-Display-S3](https://www.amazon.com/gp/product/B0BLNKPL25/) ESP32 microcontroller with a built-in capacitive touch color LCD display. These ESP32 units are available with a case or you can buy them separately, there's no need for any 3D printing. The only wires coming out of the left side are the ribbon cable from the SCR controller and a temperature sensor cable. The USB-C port for the ESP32 is accessible for software updates without the need to take anything apart. Refer to the photos and PDF files in the **/Instructions/** directory here for more details.

<img width="1024" src="https://github.com/user-attachments/assets/0b984446-51cc-459e-bba1-18f2396f57d7">
