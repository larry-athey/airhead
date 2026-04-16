//------------------------------------------------------------------------------------------------
// Airhead - Air Still Brain | (CopyLeft) 2024-Present | Larry Athey (https://panhandleponics.com)
//
// Due to the design of the LilyGo T-Display S3, USB firmware updates are cumbersome and require a
// person to open up the Airhead to hold down the boot button to receive the firmware update. This
// is an unacceptable pain in the ass for most people, so over-the-air programming is used instead.
//
// The Arduino IDE gets around this by programmatically banging on specific serial communications
// signals in precisely timed instances which you absolutely aren't going to easily do by hand.
//
// Keep in mind that if the LilyGo T-Display S3 doesn't have an antenna connected to it, the WiFi
// range is extremely limited. Firmware updates should only be done with the Airhead within a few
// feet of your computer or phone.
//
// Inline functions used for modular unit organization
//------------------------------------------------------------------------------------------------
#include "WiFi.h"                // WiFi networking support library
#include "WebServer.h"           // Basic web server library
#include "ESPmDNS.h"             // Bonjour/ZeroConf support library
#include "Update.h"              // ESP32 live firmware update library
//------------------------------------------------------------------------------------------------
WebServer server(80);
//------------------------------------------------------------------------------------------------
const char* ap_ssid       = "Airhead-Firmware-Updater";
const char* ap_password   = "12345678";

const char* http_username = "admin";
const char* http_password = "admin";
//------------------------------------------------------------------------------------------------
// Login page (simple form)
const char loginIndex[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>Airhead-Firmware-Updater</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body { background:#3498db; font-family:sans-serif; color:#333; text-align:center; padding:20px; }
    form { background:#fff; max-width:320px; margin:40px auto; padding:30px; border-radius:8px; box-shadow:0 0 10px rgba(0,0,0,0.1); }
    input { width:100%; padding:12px; margin:10px 0; border-radius:4px; border:1px solid #ddd; font-size:16px; box-sizing:border-box; }
    input[type=submit] { background:#3498db; color:white; border:none; cursor:pointer; }
    h1 { color:#2c3e50; }
  </style>
</head>
<body>
  <form action='/serverIndex' method='get'>
    <h1>OTA Updater</h1>
    <input name='userid' placeholder='Username' required><br>
    <input name='pwd' type='password' placeholder='Password' required><br>
    <input type='submit' value='Login'>
  </form>
</body>
</html>
)rawliteral";
//------------------------------------------------------------------------------------------------
// Upload page with inline vanilla JS (no jQuery, no external CDN)
const char serverIndex[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>Airhead-Firmware-Updater/title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body { background:#3498db; font-family:sans-serif; color:#333; text-align:center; padding:20px; }
    form { background:#fff; max-width:360px; margin:40px auto; padding:30px; border-radius:8px; box-shadow:0 0 10px rgba(0,0,0,0.1); }
    #file-input { display:block; width:100%; padding:12px; margin:15px 0; border:2px dashed #3498db; border-radius:6px; cursor:pointer; background:#f9f9f9; }
    input[type=submit] { background:#3498db; color:white; border:none; padding:14px; font-size:16px; cursor:pointer; width:100%; border-radius:4px; }
    #prgbar { background:#f1f1f1; border-radius:10px; height:12px; margin:20px 0; overflow:hidden; }
    #bar { background:#3498db; width:0%; height:100%; transition:width 0.3s; }
    #prg { font-size:14px; margin:10px 0; }
  </style>
</head>
<body>
  <form id="upload_form" method="POST" enctype="multipart/form-data">
    <h1>Firmware Update</h1>
    <label id="file-input" for="file">Choose .bin file...</label>
    <input type="file" name="update" id="file" style="display:none" required>
    <input type="submit" value="Upload & Update Firmware">
    <div id="prg">Progress: 0%</div>
    <div id="prgbar"><div id="bar"></div></div>
  </form>

  <script>
    const fileInput = document.getElementById('file');
    const fileLabel = document.getElementById('file-input');
    const form      = document.getElementById('upload_form');
    const prg       = document.getElementById('prg');
    const bar       = document.getElementById('bar');

    fileInput.addEventListener('change', () => {
      const fileName = fileInput.files[0] ? fileInput.files[0].name : 'Choose .bin file...';
      fileLabel.textContent = fileName;
    });

    form.addEventListener('submit', (e) => {
      e.preventDefault();

      const data = new FormData(form);
      const xhr = new XMLHttpRequest();

      xhr.open('POST', '/update', true);

      xhr.upload.addEventListener('progress', (evt) => {
        if (evt.lengthComputable) {
          const percent = Math.round((evt.loaded / evt.total) * 100);
          prg.textContent = `Progress: ${percent}%`;
          bar.style.width = percent + '%';
        }
      });

      xhr.onload = () => {
        if (xhr.status === 200) {
          prg.textContent = 'Update successful! Rebooting...';
          setTimeout(() => location.reload(), 3000);
        } else {
          prg.textContent = 'Update failed!';
        }
      };

      xhr.onerror = () => { prg.textContent = 'Upload error!'; };

      xhr.send(data);
    });
  </script>
</body>
</html>
)rawliteral";
//------------------------------------------------------------------------------------------------