# orilamp

This repository holds the Arduino code for the Ori lamp I made, alongside with some other useful resources.  
This lamp is a RGB one, that is powered with USB and that has buttons to change various parameters regarding the lamp (color, effect...)

<p align="center">
<img src="https://github.com/ailothaen/orilamp/blob/master/resources/1.jpg?raw=true" alt="1.jpg" width="200">
<img src="https://github.com/ailothaen/orilamp/blob/master/resources/2.jpg?raw=true" alt="2.jpg" width="200">
<img src="https://github.com/ailothaen/orilamp/blob/master/resources/3.jpg?raw=true" alt="3.jpg" width="200">
</p>

For more pictures and context, [click here](https://todo.com)

This work can be decomposed in 4 parts: model, box, electronics, and coding.


## Model

The Ori on the box was made by a 3D FDM printer. I have access to pretty good printers at my workplace, so I was able to use good materials with good quality (Polycarbonate with 60 µm precision).

The 3D model was originally made by Julien Kaspar, check it out here: https://sketchfab.com/3d-models/ori-c97e93c9d89f4a36988252b1cf2f3737  
I asked for someone to modify the hands to remove one finger, because I do not like seeing Ori with 4 fingers while he actually has 3. 👀


## Box

Nothing really significant to tell here, just 6 wooden panels assembled together and painted in black. The size of box is 20×12×6 cm.


## Electronics

Here is the schematic diagram.

![circuit.png](https://github.com/ailothaen/orilamp/blob/master/resources/circuit.png?raw=true)

(The diagram says "Arduino Nano", but it's actually an Arduino Nano Every)

The power source is an USB cable, plugged into the USB port of the Arduino. The switch is here as a convenient on/off button (I put the switch on the V+ USB cable)

The three pushbuttons are used to control various parameters of the lamp (see below).  
I used 330 Ω resistors for the [pull-down](https://www.seeedstudio.com/blog/2020/02/21/pull-up-resistor-vs-pull-down-differences-arduino-guide/) because this is what I had (and it worked when I tried 😛), but it is probably better to use another value.

Here is what it looks like inside (with everything soldered to a copper breadboard):

![inside.jpg](https://github.com/ailothaen/orilamp/blob/master/resources/inside.jpg?raw=true)


## Coding

The card handling all the logic is an Arduino Nano Every.  
The Arduino sketch is in the file `sketch.c`. One library was used (RGBConverter), to make easy the calculations about RGB/HSL conversions.

The "inputs" of the program are the three buttons. In the code, they are called:
- **H** button (controlling the Hue)
- **L** button (controlling the Lightness)
- **E** button (controlling the Effect)

There are also two other functions:
- **S** function (Saturation), by holding **H** and pressing **L**
- **C** function (Chance – changing speed and "chance" of effects), by holding **E** and pressing **L**

![manual.jpg](https://github.com/ailothaen/orilamp/blob/master/resources/manual.png?raw=true)

These "letters" are used conventionally in the whole code.

Additionally, there are two other "hidden functions":
- When holding **L** on startup, the lamp parameters (hue, effect...) reset to default values
- When holding **E** on startup, the messages on serial output are enabled (useful for debugging). Note that, when you open a serial terminal and click Connect, [it restarts the controller](https://forum.arduino.cc/t/serial-port-question/283481), so you have to **hold E while clicking Connect** (I say that because that really confused me)

The hue, brightness, saturation, effects and chance values are hardcoded; depending of the LED model you take, you may want to edit these values, especially the Hue (as most LEDs have a "favorite color zone" where they are more powerful)
