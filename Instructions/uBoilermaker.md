# μBoilermaker (micro Boilermaker) 

<img width="1024" alt="image" src="https://github.com/user-attachments/assets/f75d4ee9-a923-468e-96ad-13fdba60d6dc">

---

The μBoilermaker is 100% based on the Airhead, but is a stand-alone boiler controller for 120 volt resistive loads up to 1500 watts and uses PID temperature control, unlike the aggressive PI controller used by the Airhead. You can switch any Airhead into μBoilermaker mode by holding the Value+ button for 15 seconds after you have selected Mode 3. Use the same procedure to switch things back.

The reason for the μBoilermaker is to satisfy the numerous requests for a Boilermaker that isn't WiFi dependent. This isn't a one-to-one implementation of the full Boilermaker, but it provides a person the ability to have constant power, constant temperature, or timed runs with progressive temperature. It also uses a preset **Cruise Then Brew** mode and runs in PI control until 1C below the target temp.

Unlike the full Boilermaker, these are sold on my website as 120-volt-only and bench/counter-top-only. The size of the case only allows for 14 gauge grounded power cables and they only ship with an SSR-40DA solid state relay mounted to the same heat sink used in the Airhead. These are priced the same as the full Boilermaker with 120 volt pigtails, they're just a smaller version with a touch screen.

[Operation](https://github.com/larry-athey/airhead/blob/main/Instructions/Operation.md) of the μBoilermaker is completely identical to the Airhead, the only real difference are the options on the calibration page which are PID gain parameters. The target audience for these are people using stove-top stills with an electric hot plate, home beer brewers, and people who want far better control over an electric smoker, all without the need for a WiFi connection.

**How to run autotune:**

1. Fill the boiler to its normal full mark with water.
2. Select Mode 3.
3. Hold **Value+** for at least 5 seconds, then press **Value-** at the same time.
4. The test takes 5–15 minutes. Watch the screen (or serial output if connected) for progress.

After tuning completes, the new **P, I, and D** values will be saved. You can manually tweak them on the calibration screen if needed.

**What is PID? (simple version)**
PID is a control method that tries to reach and hold your exact target temperature smoothly:

- **P (Proportional)** — How strongly it reacts to being off-target.
- **I (Integral)** — Corrects small long-term errors (like slowly drifting a bit low).
- **D (Derivative)** — Looks at how fast the temperature is changing and helps prevent overshooting.

**Realistic expectations:**

- PID gives very steady temperature — great for mashing, sous-vide style control, or electric smokers.
- It is **not** as thermally aggressive as the normal Airhead PI mode. Once at temperature it will pulse the heater more gently, so you may see slightly slower vapor production in distillation.
- Autotune results are a good starting point, but every boiler/setup is different. Feel free to tweak the numbers ±20–50% and test.

If the autotune ever returns weird values (P=0, negative D, etc.), just preheat more and try again.

_**NOTE:** If you are using an electric hot plate, you will want to bypass its internal temperature control dial or it will interrupt your power once you reach its maximum temperature. It's also recommended that you maintain good ventilation under it because most also have a thermal snap switch that will cut out if it overheats internally. The μBoilermaker does not work with inductive cooktops!_

---

<img width="1024" alt="image" src="https://github.com/user-attachments/assets/ad1b7c8b-17d2-45b3-ac26-9599273d4303">

---

<img width="1024" alt="image" src="https://github.com/user-attachments/assets/b1b1e016-f67e-454a-8b3b-8ba2851cc8a9">

---

<img width="1024" alt="image" src="https://github.com/user-attachments/assets/9ea3ed5d-9d75-43d8-9a72-76bc0e5d447a">

---

<img width="1024" alt="image" src="https://github.com/user-attachments/assets/8ea0da46-e472-467c-ab58-342c18f8e705">

