# Instructions

In these examples, I'm using a cheap $65 [Vevor water distiller](https://www.amazon.com/dp/B0CN2SBJLT). No need for an expensive original Air Still for this project. For the sake of simplification, I will be setting up this still to use a solid state relay rather than an SCR controller. So, the first step is to turn your Air Still upside down and remove the bottom cover.

Referring to the photo **Air-Still-Default-Wiring.jpg** here. The white wires are connected to the thermal control snap switch and are the AC neutral connection to the heating element. The blue wires are for an LED power indicator on the front of the still. The red wires are from the AC hot side and go to a circuit breaker on the front of the still and then come back to connect to the other side of the heating element. Lastly, the yellow wires are the earth ground of the AC connection and connect to the body of the boiler vessel.

<img width="1024" src="./Air-Still-Default-Wiring.jpg"><br>

You will first need to remove that aluminum bracket so you can remove the power indicator, the button for the reset breaker, and the snap switch that currently controls the temperature. We will not be removing the reset breaker, but it will likely never be used since the Airhead controller will shut down the unit if it ever goes over 103C/217F. You can clip the wires to the power indicator and strip them since they will be used to power the Airhead controller. The wires to the snap switch will be connected to the solid state relay.
