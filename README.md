# Enchanted Orbs

A fast Arduino / Gameduino based Magical Drop clone, played with an NES controller!

Please refer to [this blog article](http://www.diericx.net/post/enchanted-orbs-arduino-magical-drop-clone/) for more detail.

![Enchanted Orbs](https://github.com/christophediericx/EnchantedOrbs/blob/master/Images/EnchantedOrbs.png)

## Components ##

* A [Gameduino 1](http://excamera.com/sphinx/gameduino/) shield.
* An Arduino Mega 2560.
* A classic NES controller.
* A breadboard and some wires.

## Wiring ##

![Breadboard Setup](http://www.diericx.net/images/enchantedorbs2.png)

Wiring is straightforward. For reference purposes, here is a [fritzing diagram](http://www.diericx.net/downloads/enchantedorbs.fzz):

![Wiring](http://www.diericx.net/images/enchantedorbs3.png)

In the layout above, the NES Controller is attached through breadboard pins A10-A15:

| Breadboard     | NES Controller   |                    |
| -------------- | ---------------- | ------------------ |
| A10            | Clock            |                    | 
| A11            | Latch            |                    |
| A12            | Data1            |                    |
| A13            | Data2            | (optional, unused) |
| A14            | +5V              |                    |
| A15            | GND              |                    |

## Gameplay Video ##

## Font and Art Credits ##

I used the following fonts:

* [Pete Klassen Ringbearer](http://www.thehutt.de/tolkien/fonts/ringbearer/readme.html)
* [PressStart2P](http://www.dafont.com/press-start-2p.font)
* [Zingot Games Bitmap Font Pack](http://opengameart.org/content/bitmap-font-pack)

and the following art (all from [opengameart.org](http://opengameart.org)):

* [Title Screen Background Art](http://opengameart.org/content/castle-platformer)
* [Main Hero Character](http://opengameart.org/content/16x16-8-bit-rpg-character-set)
* [Game Screen Play Field Border](http://opengameart.org/content/golden-and-emerald-border)
* [Orbs](http://opengameart.org/content/magic-orbs)
