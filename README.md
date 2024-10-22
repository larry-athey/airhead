# Airhead
 Air Still (or clone) upgrade that uses an SCR controller for the heating element and an ESP32 to make the whole thing smarter. Eliminates that constant full power on/off switching to the heating element and the possibility of scorching. Plus even more capabilities.

You may contact me directly at https://panhandleponics.com<br>
Subscribe to the official YouTube channel at https://www.youtube.com/@PanhandlePonics<br>

Aside from the Still Spirits brand Air Stills only available as an Ethanol Version and a Water Version, they both suffer the same design flaw where the temperature is managed the same way it is with an electric cooking appliance. Even the various Air Still clones out there work the same way, including the ones with adjustable temperature. They send full power to the heating element when heating up, shut it off when they're up to temperature, then repeat this cycle.

If you have any suspended solids in your wash, they will scorch and fuse to the bottom as they fall out of solution. This will add foul flavors to anything that you're distilling, especially gin. As with my [RPi-Smart-Still](https://github.com/larry-athey/rpi-smart-still) project, this is solved by replacing the relays or snap switches on the heating element with an SCR controller. As long as the element is always on at some level, heat will radiate upward and force most suspended solids to stay afloat. This will eliminate the possibility of scorching for the most part.
