# Airhead Operation

As you have probably noticed by the photos on the frant page of this GitHub repo, the Airhead has three operating modes. Each of the modes are user configurable and your last used settings will be restored each time you start up your still.

**Mode 1** maintains a constant power level with no power management whatsoever. You may adjust the power level from 10% to 100%. Both SCRs and SSRs behave unpredictably below 10% power, so this lower limit is intentionally imposed. This mode would be used for simmering liquids or distilling water if you set the power level to 100%. There is a safety feature that will shut down your still if you boil out all of the water. This kicks in at 103C/217F well before the internal circuit breaker trips.

**Mode 2** maintains a constant temperature and automatically adjusts the power. If you fill your still with room temperature solution and you set the temperature to 80C/176F, it may take 20 minutes to reach the minimum operating temperature. The power level starts out at 50% and progressively increases by 1% every minute until the temperature is reached. At that point, the power will fall back to one third (33%) and rest for 2 minutes before temperature management kicks in.
