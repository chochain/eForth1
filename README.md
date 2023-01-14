# eForth1 - eForth for Arduino

Wrote in his ceForth_33 document, Dr. Chen-Hanson Ting, the creator of eForth, and one of the inspiring figures of Forth community noted:
> *In 2011, I was attracted to Arduino Uno Kit and ported eForth to it as 328eForth...writing to flash memory, I had to take over the bootload section which was monopolized by Arduino IDE...I extended Forth dictionary in the RAM memory. It worked. However, you had only 1.5KB of RAM memory left over for new Forth words, and you could not save these new words before you lost power. As I stated then, it was only a teaser to entice new people to try Forth on Arduino Uno.*

Before Dr. Ting conceded his fight with cancer in May, 2022, I've spent the last 11 months working with him expanding the concept of "Forth without Forth" - a new eForth model - he called. Traditionally, Forth is built with a set of core words in low-level assembly language and establish the rest of words with high-level Forth scripts which get boot-strapped on start-up time. With the proliferation of modern languages, build Forth without using Forth, he reasoned that Forth built entirely in high-level languages can not only greatly simplify the virtual machine, taking advantage of modern languages and operating system, but also encourage portability and optimization. In a few months, we completed new batch of eForths in Javascript, Java, C++, ported them to Windows, ESP32, which Dr. Ting presented his video on November, 2021 to [Sillicon Valley Forth Interest Group](https://www.youtube.com/watch?v=bb5vi9kR1tE&t=827s). Later, we focused on SystemVerilog's edition with Don & Demitri's CORE-I FPGA project of [AI & Robotics Group](https://www.facebook.com/groups/1304548976637542) til his eventual departure.

In Dr. Ting's own word:
> *In all these years, I have thought that the eForth Model is a good model useful for all different processors and microcontrollers, and for all different applications. It is a very simple model for anybody who like to learn Forth and to use it for their own applications.*

To enjoy the beauty of working on something small and simple, I've decided to pick up Dr. Ting's eForth Model and have fun with it. So, here we go!

### What is eForth1?

* An eForth for Arduino UNO implemented in C.
* A .ino file that can be openned in Arduino IDE and load/run directly onto Arduino UNO.
* Can be embeded with other Arduino applications.
* Has 16-bit cells and stacks. Double means 32-bit.
* Can read/write Arduino pins.
* Supports Arduino Interrupts.
* Can save/load app to/from EEPROM.
* Can autorun from saved EEPROM.

### How to install eForth1?

* From Arduino IDE's Library Manager
> <br/>
> \> Make sure you've hooked up one of Arduino Nano/Uno, or a development board that hosts ATmega328
>
> \> from Arduino IDE > Tools > Manage Libraries, enter FORTH in search box
>
> \> find eForth1 in the short list, select the latest version, and click the Install button
>
> \> from Files > Examples, find eForth1 in Examples from Custom Libraries at very buttom section
>
> \> load one of the nanoForth examples, such as 0_hello
>
> \> open Serial Monitor, set baud rate to 115200, and line ending to 'Both NL & CR'
>
> \> hit compile and upload. You should see the 'ok' prompt
>
> \> in Serial Monitor input bar atop, type WORDS and hit <return>. See what eForth1 says.<br/><br/>

* or, from GitHub directly, if you prefer managing source codes manually
> <br/>
> \> git clone https://github.com/chochain/eForth1 onto your local Sketch directory
>
> \> copy examples/0_hello/0_heloo.ino from sub-directory, then rename it as eforth1.ino<br/>
>
> \> open eforth1.ino with Arduino IDE, and setup your Nano/Uno (or ATmega328) development board
>
> \> in nanoFORTH.ino, change the #include <eforth1.h> to #include "./src/eforth1.h"
>
> \> open Serial Monitor, set baud rate to 115200, and line ending to 'Both NL & CR'
>
> \> compile and upload, you should see the 'ok' prompt
>
> \> in Serial Monitor input bar atop, type WORDS and hit <return>. See what eForth1 says.<br/><br/>

* Hopefully, thing goes well and you get something like this if eForth1 is uploaded successfully 
> <br/>
> |screen shot|
> |:--|
> |@image html images/eforth1_init_snip.png|
<br/>

### To Learn More About Forth?
If your programming language exposure has been with C, Java, or even Python so far, FORTH is quite **different**. Quote Nick: <em>"It's no functional or object oriented, it doesn't have type-checking, and it basically has zero syntax"</em>. No syntax? So, anyway, before you dive right into the deep-end, here's a good online materials.
* Interactive tutorial for FORTH primer. It teaches you how FORTH fundamentally works such as the numbers, the stack, and the dictionary.
  > <a href="https://skilldrick.github.io/easyforth/#introduction" target="_blank">Easy Forth Tutorial by Nick Morgan</a>

To understand the philosophy of FORTH, excellent online e-books are here free for you.
* Timeless classic for the history, paths, and thoughts behind FORTH language.
  > <a href="http://home.iae.nl/users/mhx/sf.html" target="_blank">Starting Forth by Leo Brodie</a><br/>
  > <a href="http://thinking-forth.sourceforge.net" target="_blank">Thinking Forth by Leo Brodie</a>

### References to Dr. Ting's Original
* [ceForth_33.doc - original documentation](https://chochain.github.io/eForth1/docs/ceForth_33.doc)
* [ceForth_33.cpp - source assembler + VM](https://chochain.github.io/eForth1/docs/ceForth_33.cpp)
