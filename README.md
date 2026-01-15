# Airhead
Air Still (or clone) upgrade that uses an SCR board or SSR for heating element control and an ESP32 to make the whole thing smarter. Eliminates that constant full power on/off switching to the heating element and the possibility of scorching. Plus even more features.

You may contact me directly at https://panhandleponics.com<br>
Subscribe to the official YouTube channel at https://www.youtube.com/@PanhandlePonics<br>

_**SIDE NOTE:** Save your money and just get a [Vevor Air Still clone](https://www.amazon.com/dp/B0CN2SBJLT/) water distiller for this project. Add that to the cost to build this or purchase a kit from my website, and it still comes in less expensive than an original Still Spirits Air Still with the temperature control problems._

_**ALSO:** If you'd like a stand-alone WiFi enabled version of the temperature/power controller that I designed here, with a truckload more features, check out my [Boilermaker](https://github.com/larry-athey/boilermaker) project!_

**Facebook Idiot:** _Why would you bother?_<br>
**Me:** It's only a "bother" for somebody who couldn't do it on their best day. I'd rather have nicer things.

---

Aside from Still Spirits brand Air Stills only being sold as an Ethanol Version and a Water Version, they both suffer the same design flaw where the temperature is managed the same way it is with an electric cooking appliance. Even the various Air Still clones out there work the same way, including the ones with adjustable temperature. They send full power to the heating element when heating up, shut it off when they're up to temperature, then repeat this cycle. Thanks for the unstable yo-yo temperatures!

If you have any suspended solids in your wash, they will scorch and fuse to the bottom as they fall out of solution. This will add foul flavors to anything that you're distilling, especially gin. As with my [RPi-Smart-Still](https://github.com/larry-athey/rpi-smart-still) project, this problem is solved by replacing the relays or snap switches on the heating element with a custom low speed PWM _(0.20 Hz)_ and a solid state relay that never runs lower than 10% duty cycle.

As long as the heating element is always running even just a little bit, it will radiate heat upward and force most of the suspended solids to stay afloat. This will eliminate the possibility of scorching for the most part. Just don't try to fill your still with a thick goopy mash.

This project aims to solve that problem and add three user selectable modes:

1. Operate at a constant user selectable power level (10% to 100%)
2. Operate at a constant user selectable temperature (up to 100C/212F)
3. Perform timed runs with a user selectable starting and ending temperature
4. _Maybe I'll add WiFi to it someday_

The greatest benefit of the custom low speed PWM and SSR is that it doesn't contribute to premature heating element failure as with the high-speed on/off switching in SCR controllers. SCR phase-angle control generates harmonics _(distortions in the AC sine wave)_ which leads to uneven current distribution, physical vibration, and stress within a heating element. The Airhead is actually better for your boiler's longevity!

The technical definition of the temperature controller in the Airhead is a PI controller, there is no D _(derivative gain)_. The reason for this is because PID controllers will spend more time off than on once the boiler is up to temperature. This is bad for distillation because there's no thermal velocity to push the ethanol upward out of your wash. This temperature controller will never drop below 10% power.

The brains of the operation is a [LilyGo T-Display-S3](https://www.amazon.com/gp/product/B0BLNKPL25/) ESP32 microcontroller with a built-in capacitive touch color LCD display. The boiler control code is a port of the code from my [RPi-Smart-Still](https://github.com/larry-athey/rpi-smart-still) project. This means that even though this is a newer project, the code has already been refined and fine tuned for 2 years in advance.

Refer to the contents of the **/Diagrams/** and **/Instructions/** directories here to learn how to build this project.

_**NOTE:** Turn-key kits for those not comfortable with building this from scratch are now available from [my website](https://panhandleponics.com/technology-assisted-distillation/). There may be a short lead time on orders to allow for 3D printing of the 3D models or parts restocking. You may also drop-ship your air still clone to me to have the Airhead upgrade installed and tested with an actual run before I send it to you ready to roll! And it still works out to be less expensive than an original Air Still!_<br><br>

<img width="1024" src="https://github.com/user-attachments/assets/5718c5b2-e66c-4fb2-8cd2-e12219879c87"><br><br>
<img width="1024" src="https://github.com/user-attachments/assets/0a62e45b-1fc2-4e6d-bd1d-f9f94309f22b"><br><br>
<img width="1024" src="https://github.com/user-attachments/assets/bc287f2c-4181-4734-afa7-103756f2cf4e"><br><br>
<img width="1024" src="https://github.com/user-attachments/assets/df7dcee3-0d0e-4c08-ac75-7d823ccc1b14"><br><br>
<img width="1024" src="https://github.com/user-attachments/assets/52e759e5-1a44-4c1b-a983-03b44a5fd17b"><br>

