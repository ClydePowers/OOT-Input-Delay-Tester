OOT-Input-Lag-Tester

![alt text](https://github.com/ClydePowers/OOT-Input-Delay-Tester/blob/master/GC-Schematic.png?raw=true " Logo Title Text 1")
You can also use a Photodiode/resistor and wire it up in the same way, value of the resistor depends on your Photodiode/Transistor/Resistor (I used 10k Ohm)

Installation:
1. Wire everything up as you can see in the schematic
2. Adjust the BRIGHTNESS Value in the Firmware.ino
3. Upload the Firmware.ino to your arduino and open the Serial Monitor in the Arduino IDE
4. start gz and go into a dark place (like ganon's tower oob), stand right infront of a wall (so it takes 5frames for the deku nut to light up the screen)
5. tape the Diode/Transistor to the top left of your screen
6. throw a deku nut
7. the Serial Monitor should show you your input delay and average delay now
8. repeat to get an average time (atleast 25 times)

If the sketch isnt working right off the bat you need to adjust the value from "int BRIGHTNESS" to a lower or higher number (upto 1023) this depend on the Photodiode/transistor/resistor and the resistor you are using!

The reason I choose a Transistor over a Diode is that I couldn't get any measurements with a Diode on a LCD Monitor, tho a Diode worked very well on my CRT.

Demonstration Video: https://www.youtube.com/watch?v=MzTeuVLUS5I&t
