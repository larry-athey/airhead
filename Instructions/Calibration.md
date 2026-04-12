# Calibration

<img width="1024" src="./Airhead-Kit-3.jpg"><br>

Calibrating the correction factor of the Airhead temperature sensor is actually stupid-simple and only requires you to know your altitude above sea level. Water boils at 100C/212F at sea level and that temperature drops by 0.5C/1F for every 500 feet you are above sea level. So in my case where I'm 4000 feet above sea level, water boils at 95.5C/204F.

Keep in mind that an immersion thermometer isn't the most accurate way of comparing the readings to determine the correction factor. If you were to use one, an old analog unit that works using a bi-metal coil would be the most reliable one since they aren't affected by the EMF noise that comes from the heating element. A water boil test will always be the most reliable method.

## How to do a water boil calibration

1. Reset the Correction Factor to 0.0C
2. Fill the still with plain water
3. Switch to mode 2 and set the target temperature to 100C/212F
4. Put the lid back on, plug it in, and start the run
5. Make a note of the temperature when you start seeing the first drops of water coming out
6. Stop the run, go back to the configuration page and adjust the Correction Factor as needed
7. Retart the run, you may want to add a couple ice cubes to cool down the water first
8. Note the temperature when water drops start coming out again
9. Repeat until you have the sensor dialed in, it doesn't have to be dead accurate

Keep in mind that barrometric pressure can affect this, but not by a whole lot. You could easily see your boiling temperature go up or down by 0.5C/1F depending on the weather. Also keep in mind that some vapor will remain in the condenser every time you stop and restart the boiler, so you can easily ignore the first few drops after a restart.
