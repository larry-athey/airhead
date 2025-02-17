# Airhead Turn-Key Kit

For those who aren't up to component level electronics work or 3D printing, [Airhead turn-key kits](https://panhandleponics.com/technology-assisted-distillation/) are available from my website for $140 plus shipping in the USA where the USPS flat-rate box coverage area applies. The interface board will soon be a printed circuit board rather than a handmade one, I'm just waiting to order a bulk supply of printed circuit boards for three of my projects.

Please keep in mind that I'm not running a full blown mail-order operation. I only have three 3D printers and only one that is large enough to print the Airhead spacer ring. Each of these halves take 9 hours to print, so I'm only capable of building one per day. My other two printers are pretty much dedicated to printing [Boilermaker](https://github.com/larry-athey/boilermaker) cases.

<img width="1024" src="./Airhead-Kit-1.jpg"><br>

<img width="1024" src="./Airhead-Kit-2.jpg"><br>

<img width="1024" src="./Airhead-Kit-3.jpg"><br>

# Assembly Instructions

_The following instructions are for **modifying** an Air Still (or clone). Personally, I would recommend just gutting the unit and rewiring the unit from scratch according to the [Airhead Implementation](https://github.com/larry-athey/airhead/blob/main/Diagrams/Schematic_Airhead-Implementation.pdf) schematic without the temperature snap switch or the circuit breaker that kicks if it boils dry. The Airhead Code already has protection built-in that shuts down the run if the boiler temperature exceeds 103C/217F. It's clear that these devices are not built to any solid standards, so it's really not possible to create a one-size-fits-all set of modification instructions._

In these examples, I'm using a cheap $65 [Vevor water distiller](https://www.amazon.com/dp/B0CN2SBJLT). No need for an expensive original Air Still for this project. For the sake of simplicity, I will be setting up this Air Still clone to use a solid state relay as a simlified PID controller rather than an SCR board. You can also use a solid state relay like an SCR board at a much lower frequency, refer to the source code comments. Anyway, the first step is to turn your Air Still upside down and remove the bottom cover.

Referring to the photo **Air-Still-Default-Wiring.jpg** here. The white wires are connected to the thermal control snap switch and are the AC neutral connection to the heating element. The blue wires are for an LED power indicator on the front of the still. The red wires are from the AC hot side and go to a circuit breaker on the front of the still and then come back to connect to the other side of the heating element. Lastly, the yellow wires are the earth ground of the AC connection and connect to the body of the boiler vessel.

<img width="1024" src="./Air-Still-Default-Wiring.jpg"><br>

You will now need to remove that aluminum bracket so you can remove the power indicator, the button for the reset breaker, and the snap switch that currently controls the temperature. We will not be removing the reset breaker, but it will likely never be used since the Airhead brain will shut down the unit if it ever goes over 103C/217F. You can clip the wires to the power indicator and strip them back since they will be used to power the Airhead controller system. The wires to the snap switch will be connected to the solid state relay.

<img width="1024" src="./Air-Still-Lobotomized.jpg"><br>

As you can see in the photo **Air-Still-Lobotomized.jpg** above, everything that I mentioned has been removed and the aluminum bracket has been put back in place. I slid it back as far as it would go since the reset breaker will likely never need to be used unless something goes seriously wrong. We have the blue wires that will be connected to the Airhead power supply and interface board, and the white wires which will be connected to the load side of the solid state relay. You may or may not need to extend the white wires.

_**NOTE:** Depending on your Air Still (or clone) model, the manufacturer may have put the fan on the switched side of the heating element. This is an absolutely stupid plan, the fan should run continuously. The temperature control should only run the heating element. This will require a simple wiring modification if yours has this problem. Lucky me, this unit actually has that exact design flaw._

<img width="1024" src="./Airhead-Case-Back-Fitting.jpg"><br>

As demonstrated in the photo **Airhead-Case-Back-Fitting.jpg** above, you need to align the back part of your Airhead case over the hole that the reset button was removed from. It's best if you set your spacer ring in place to get your vertical alignment correct. You should mark the two screw holes using a pencil, or small paint brush and paint, or nail polish if you happen to have that at home. Carefully drill the holes while avoiding damage to the boiler vessel inside. Then attach the case with nuts and bolts, paint the threads to lock the nuts in place.

<img width="1024" src="./Airhead-Brain-Installed.jpg"><br>

As you can see in the photo **Airhead-Brain-Installed.jpg** above, the Airhead brain is installed in its case and it doesn't interfere with the seating of the spacer ring. This is just a dry fitting test before we procede with the wiring. Please keep in mind that the wiring steps that follow are using a prototype power supply and interface board. The Airhead kits that I will be selling on my website will come with an actual custom designed printed circuit board. I just don't have them at this time since they have to be ordered from China in bulk.

_**NOTE:** If you are using the same Air Still clone that I'm using, you will need a 6mm-1.00x70mm bolt to reattach the bottom after the spacer ring is in place. Other brands may use a different method to attach the bottom, I have another brand that uses 3 screws._

<img width="1024" src="./Air-Still-Wiring-Fix-1.jpg"><br>

As you can see in the photo **Air-Still-Wiring-Fix-1.jpg** above, the bottom connection of the heating element has one of the blue wires and a white wire connected to it. If we were to leave things as is and connect the two white wires from the snap switch to the solid state relay, the entire system would never turn on. If it did, the entire system including the fan would turn off as soon as it's up to temperature. This is the stupid plan that I was referring to. Cut that white wire connected from the element to the fan power socket.

_**NOTE:** Don't panic, the goal here is just to have all of the neutral lines connected together, except for the neutral side of the heatng element._

<img width="1024" src="./Air-Still-Wiring-Fix-2.jpg"><br>

Looking at the photo **Air-Still-Wiring-Fix-2.jpg** above, you can see that the white wire that was clipped has been extended about 4 inches. This connects to one of the screws on the load side of the solid state relay. The blue wire was removed the the bottom side of the heating element, cut in half, and then joined together with the two white wires that were connected to the snap switch that was removed. The blue wire with the eyelet connector on the end connects to the other screw on the load side of the solid state relay.

<img width="1024" src="./Airhead-Interface+PSU.jpg"><br>

As you can see in the photo **Airhead-Interface+PSU.jpg** above, the prototype interface and PSU board has been attached to one of the spacer ring halves. The two blue wires with the stripped ends have been connected to the AC input terminals. The Airhead brain ribbon cable and the DS18B20 temperature sensor have also been connected. The temperature sensor has been installed under a copper heat sink clip against the aluminum bracket. Thermal transfer compound shouldn't be used in this case.

Airhead brain connector, left to right:
1. Temperature, ESP32 GPIO 13
2. PWM, ESP32 GPIO 1
3. +5 volts
4. Ground

Temperature sensor connector, left to right:
1. +5 volts
2. Data
3. Ground

PWM/SSR connector, left to right:
1. +3.3 volt PWM output
2. Ground

<img width="1024" src="./Airhead-SSR-Installed.jpg"><br>

Finally, as shown in the photo **Airhead-SSR-Installed.jpg** above, the solid state relay is installed and wired in. The heat sink isn't really necessary, this SSR just came with one so I went ahead and used it to show that there actually is room for one. The documentation for these states that a heat sink isn't needed until a 1200 watt load is on it. The heating element used in these appliances is only 700 watts.

_**NOTE:** If you think your temperature sensor is picking up too much heat from the heating element in the location shown in the photos, move it over where the snap switch was installed. That region of the aluminum bracket is further away from the heating element and has more free air around the mounting point. This will tame down its readings a little bit and may be beneficial to your use of the still. If you applied thermal transfer compound, you may also want to try wiping it off test things again._
