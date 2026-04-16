# Firmware Updating

As of the Airhead v1.0.2a source code, end users can now update their firmware over-the-air using a web browser. The initial plan was to provide a USB firmware update utility, but the one that Espressif provides doesn't work like the Arduino IDE does and requires the user to perform circus tricks and learn to use a cryptic program that no non-techie will ever bother with.

To put the Airhead into firmware update mode, turn on the power then press and hold the **Value-** button as soon as the screen lights up. When you see the banner on the screen that says **"Firmare Update Mode"** you can let go of the button.

The Airhead will start up a WiFi access point with the SSID **"Airhead-Firmware-Updater"** and the password to connect is **"12345678"**. Keep in mind that the Airhead is working without an onboard antenna, so the reliable WiFi range is limited to just a few feet.

Start your web browser and go to **http://192.168.4.1**

If you are unable to connect, check to make sure that your device pulled an IP address from the Airhead. If it does not pull one, you will need to manually configure a static IP address on your device. Here are settings that will work for you if you encounter this problem.

Address: 192.168.4.100<br>
Subnet Mask: 255.255.255.0<br>
Gateway: 192.168.4.1<br>
DNS: 192.168.4.1<br>

Click on the button to browse to your **airhead-firmware.bin** file and click the blue button to start the upload.

Once the upload progress bar has completed, the Airhead will reboot and will be running the new firmware.
