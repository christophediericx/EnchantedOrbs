# Enchanted Orbs: an Arduino based Magical Drop clone (エンチャントされた球)

## Components ##

* **A [Gameduino 1](http://excamera.com/sphinx/gameduino/) shield**.
  * VGA output (800x600, internal resolution is 400x300).
  * Audio output.
  * A Sprite / Tile based interface very remniscent of 8-16 bit era game console capabilities.

* **An Arduino Mega 2560**.
  * Lots of storage, plenty or RAM and I/O pins. 
  
> The game could very likely be ported to run on a stock UNO too, but that would require ripping out 
  (or trimming) some of the graphics and sound data (in order for the image to fit in 32KB).
  
* **A classic NES controller**.

> I plugged it into this [NES Controller Breadboard adapter](http://www.active-robots.com/nes-gamepad-controller-adapter) but that is a convenience
  consideration mostly. The NES controller is very easy to read out, which makes it a natural candidate for an Arduino setup.

* **A breadboard and some wires**.

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

## Features ##

### Title Screen ###

This screen is close to the Gameduino's limits as far as displaying graphical detail (on a single background plane) is concerned: 237 out of 255 available character tiles are in use (and this required some very considerate image construction - lining up exactly with 16x16 offsets).

![Title Screen](http://www.diericx.net/images/enchantedorbs4.png)

The online [Gameduino tools](http://gameduino.com/tools/) are a great help though (in order to break up a source image into the individual tile elements that get loaded into character mapped screen RAM). 

![Encoding the Title Screen](http://www.diericx.net/images/enchantedorbs5.png)

The "PRESS START" message is pulsed by (repeatedly) copying over relevant areas in character RAM. Also, with a quick script I converted 
a MIDI file of Franz Schubert's "Der Erlkönig" into a representation usable by the Gameduino's (fairly advanced) audio circuitry.

### Game Screen and Game Over Screen ###

The main game screen is primarily sprite-driven (using only basic sprites features, really). Typical Magical Drop game rules apply 
(including support for chains!). There are 255 (incrementally faster) levels to be cleared. The audio is currently limited to some
characteristic bleeps (produced using sawtooth sine waves).

![Game Screen](http://www.diericx.net/images/enchantedorbs6.png)

The "game over" screen basically reuses some of the assets (e.g. character sprites) used in other screens (and is very simple). 
A nice addition would be storing an actual highscore table in the Arduino's EEPROM.

![Game Over Screen](http://www.diericx.net/images/enchantedorbs7.png)

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
