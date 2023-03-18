# eForth1 - eForth for Arduino UNO

**Dr. Chen-Hanson Ting**, the creator of **eForth** and one of the inspiring figures of Forth community, wrote:
> *In all these years, I have thought that the eForth Model is a good model useful for all different processors and microcontrollers, and for all different applications. It is a very simple model for anybody who like to learn Forth and to use it for their own applications.*

In 2011, Dr. Ting created <a href="https://chochain.github.io/eForth1/ref/328eForth.pdf" target="_blank">*328eForth*</a> to run Forth on Arduino UNO and wrote in his ceForth_33 document:
> *I was attracted to Arduino Uno Kit and ported eForth to it as 328eForth...writing to flash memory, I had to take over the bootload section which was monopolized by Arduino IDE...I extended Forth dictionary in the RAM memory. It worked. However, you had only 1.5KB of RAM memory left over for new Forth words, and you could not save these new words before you lost power. As I stated then, it was only a teaser to entice new people to try Forth on Arduino Uno.*

Before Dr. Ting conceded his fight with cancer in May, 2022, I've spent the last 11 months working with him expanding the concept of **"Forth without Forth"** - a new eForth model - he called. Traditionally, Forth is built with a set of core words in low-level assembly language and establish the rest of words with high-level Forth scripts which get boot-strapped on start-up time. Nowadays, build Forth without using Forth, he reasoned that Forth built entirely in high-level languages, specifically in C, can not only greatly simplify the virtual machine, utilizing operating system, but also encourage portability and optimization. Taking the concept forward, we completed new batch of eForths in Javascript, Java, C++, ported them to Windows, ESP32 in just a few months. Dr. Ting presented a video on November, 2021 to <a href="https://www.youtube.com/watch?v=bb5vi9kR1tE&t=827s" target="_blank">*Sillicon Valley Forth Interest Group*</a>. Later, we fancied creating a hardware version eForth with SystemVerilog and joint venture with Don & Demitri's CORE-I FPGA project of <a href="https://www.facebook.com/groups/1304548976637542" target="_blank">*AI & Robotics Group*</a> til Dr. Ting's eventual departure. The project is still active now.

Personnally, I enjoyed the beauty of working on something small and simple, so decided to pick up Dr. Ting's eForth Model and have fun with it. Since his *328eForth* was a teaser only, to move it forward, there are a few major things I need to make changes to Dr. Ting's implementation i.g. make C-coder friendly macro assembler, remove dependency on extra bootloader programmer, add EEPROM save/load, and support interrupts, ... So, here we go!

### What is eForth1?

* An eForth for Arduino UNO/Nano implemented in C.
* A .ino file that can be openned in Arduino IDE and load/run directly onto Arduino.
* Has 16-bit cells and stacks.
* Can read/write Arduino pins.
* Supports Arduino Timer and Pin Change Interrupts.
* Has C API to interface with user defined functions written in .ino.
* Can save/load app to/from EEPROM.
* Can be embeded with other Arduino applications.
* Become Turnkey system booting from saved EEPROM.

### How to install eForth1?

* From Arduino IDE's Library Manager
  >
  > \> Make sure you've hooked up one of Arduino Nano/Uno, or a development board that hosts ATmega328
  >
  > \> from *Arduino IDE > Tools > Manage* Libraries, enter FORTH in search box
  >
  > \> find eForth1 in the short list, select the latest version, and click the Install button
  >
  > \> from *Files > Examples*, find eForth1 in Examples from Custom Libraries at very buttom section
  >
  > \> load one of the eForth1 examples, such as *0_hello*
  >
  > \> open Serial Monitor, set baud rate to *115200*, and line ending to *Both NL & CR*
  >
  > \> hit compile and upload. You should see the 'ok' prompt
  >

* Or, from GitHub directly, if you prefer managing source codes manually
  >
  > \> git clone <a href="https://github.com/chochain/eForth1" target="_blank">*https://github.com/chochain/eForth1*</a> onto your local Sketch directory
  >
  > \> copy *examples/0_hello/0_hello.ino* from sub-directory, then rename it as *eforth1.ino*
  >
  > \> open *eforth1.ino* with Arduino IDE, and setup your Nano/Uno (or ATmega328) development board
  >
  > \> in *eforth1.ino*, change the <em>#include \<eforth1.h\></em> to <em>#include "./src/eforth1.h"</em>
  >
  > \> open Serial Monitor, set baud rate to *115200*, and line ending to *Both NL & CR*
  >
  > \> compile and upload, you should see the 'ok' prompt
  >
  <p/>

Hopefully, thing goes well and you get something like the snip below if eForth1 is uploaded successfully.

  * > <img src="https://chochain.github.io/eForth1/images/eforth1_init_snip.png" width=400><br/>
  
Now type **WORDS** in the input bar and hit \<return\> to list all the words supprted by eForth1. It is ready to serve your future fun projects.

  * > <img src="https://chochain.github.io/eForth1/images/eforth1_words_snip.png" width=400>

### Different from Dr. Ting's
  * Instead of direct GPIO port manipulation with byte read/write, eForth1 calls Arduino library functions i.g. PINMODE = pinMode, IN = digitalRead, OUT = digitalWrite, ... for familiarity to the IDE platform.
  * eForth1 supports multi-tasking through interrupts. It provides a base frequency at 0.01Hz using Timer2. You assign mulply of 10ms as ISR repetition trigger period. For example 500 means 500 x 10ms = every 5 seconds. Timer1 is left free for Servo or other libraries.
  * On this 16-bit system, CLOCK will return a double number (i.e. 32-bit) which takes 2 cells off stack. To calculate time difference, double arithmetic is needed, i.e. using DNEGATE, D+, or D- and the conversion words D>S, S>D.
  * DELAY takes a 16-bit value. So, max delay time is 32767ms. Longer delay will have to use loops. Also, DELAY does not interfer with interrupts (see demos below).

### Demos
  * LED blinker (assume you have a blue LED on pin 6, or try <a href="https://wokwi.com/projects/356793878308297729" target="_blank">*this Wokwi project*</a>)
    <pre>
    > 1 6 pinmode⏎                           \ set pin 6 for OUTPUT, i.e. pinMode(6, OUTPUT=1)
    > : blue 6 in 1 xor 6 out ;⏎             \ create a word to toggle the blue LED
    > : blink for blue 500 delay next ;⏎     \ create a word to blink (i.e. 500ms delay)
    > 9 blink⏎                               \ run 10 cycles (i.e. 9,8,7,...,2,1,0 to on/off 5 times)
    </pre>  

  * Timer Interrupt Service Routine (a red LED on pin 5)
    <pre>
    > 1 5 pinmode⏎                           \ set pin 5 for OUTPUT
    > : red 5 in 1 xor 5 out ;⏎              \ create an interrupt service routine (just a regular word)
    > ' red 20 tmisr⏎                        \ make the ISR ticked every 0.2 seconds (= 20 x 10ms)
    > 1 timer⏎                               \ enable timer, now you should see red LED blinking continuously
    > 19 blink⏎                              \ let's have them both blink (blue LED 10 times) 
    </pre>
    
    |||
    |:--|:--|
    |@htmlonly <iframe width="400" height="225" src="https://www.youtube.com/embed/--iLaLC5cG0?version=3&playlist=--iLaLC5cG0&loop=1&controls=0" title="" frameborder="0" allow="autoplay; picture-in-picture" allowfullscreen></iframe> @endhtmlonly|@htmlonly <iframe width="400" height="225" src="https://www.youtube.com/embed/gr3OVOcgF4Q?version=3&playlist=gr3OVOcgF4Q&loop=1&controls=0" title="" frameborder="0" allow="autoplay; picture-in-picture" allowfullscreen></iframe> @endhtmlonly|

  * Drives 8 Servos at <a href="https://wokwi.com/projects/356866133593965569" target="_blank">*this Wokwi project*</a>

### Benchmark
  * Classic 1 million cycles
    <pre>
    > : xx 999 for 34 drop next ;⏎           \ inner loop (put 34 on stack then drop it)
    > : yy 999 for xx next ;⏎                \ create the outer loop
    > : zz clock dnegate yy clock d+ ;⏎      \ CLOCK returns a double value
    > zz⏎                                    \ benchmark the 1000x1000 cycles
    > 24495 0 ok>                            \ 24495ms =~ 24.5us/cycle (with one blinking ISR running in the background)
    </pre>

### To Learn More About Forth?
If your programming language exposure has been with C, Java, or even Python so far, FORTH is quite **different**. Quote Nick: <em>"It's no functional or object oriented, it doesn't have type-checking, and it basically has zero syntax"</em>. No syntax? So, anyway, before you dive right into the deep-end, here's a good online materials.
* Interactive tutorial for FORTH primer. It teaches you how FORTH fundamentally works such as the numbers, the stack, and the dictionary.
  > <a href="https://skilldrick.github.io/easyforth/#introduction" target="_blank">*Easy Forth Tutorial by Nick Morgan*</a> with a <a href="https://wiki.forth-ev.de/lib/exe/fetch.php/en:projects:a-start-with-forth:05_easy_forth_v16_a5_withexp_comments.pdf?fbclid=IwAR0sHmgiDtnMRuQtJdVkhl9bmiitpgcjs4ZlIDVtlxrssMOmLBv0vesvmKQ" target="_blank">*Writeup*</a> by Juergen Pintaske.

To understand the philosophy of FORTH, excellent online e-books are here free for you.
* Timeless classic for the history, paths, and thoughts behind FORTH language.
  > <a href="http://home.iae.nl/users/mhx/sf.html" target="_blank">*Starting Forth by Leo Brodie*</a><br/>
  > <a href="http://thinking-forth.sourceforge.net" target="_blank">*Thinking Forth by Leo Brodie*</a>

### Performance Tuning (~15% faster)
* Check your Arduino IDE installed directory, say <em>C:\Users\myname\AppData\Local\Arduino...</em> on Windows or <em>/home/myname/Arduino/...</em> on Linux,
* Find the directory *hardware -> arduino -> avr*,
* With an editor, open the <em>'platform.txt'</em> file,
* Find all three -Os compiler options (i.g. compiler.c.flags=-c -g -Os ...)
  > change them to -O3 for speed, -Os (default) for smallest size, -O2 for somewhere in-between

### References to Dr. Ting's Original
* [*eForth and Zen*](https://chochain.github.io/eForth1/ref/1013_eForthAndZen.pdf)
* [*The Arduino controlled by eForth*](https://chochain.github.io/eForth1/ref/The_Arduino_controlled_by_eForth.pdf)
* [*328eForth mod for coinForth by D. Ruffer*](https://github.com/DRuffer/328eforth)
* [*ceForth_33.doc - original documentation*](https://chochain.github.io/eForth1/ref/ceForth_33.doc)
* [*ceForth_33.cpp - source code in C*](https://chochain.github.io/eForth1/ref/ceForth_33.cpp)
* [*eforth_328.ino - Arduino IDE teaser by Dr. Ting*](https://chochain.github.io/eForth1/ref/eforth_328.ino)

### For Projects small and large
* A tiny 8-bit sytem at [*eForth for STM8*](https://github.com/TG9541/stm8ef) or [*STM8 Programming*](https://github.com/TG9541/stm8ef/wiki/STM8S-Programming#flashing-the-stm8)
* A system with WiFi, and fancy stuffs at [*Esp32forth*](https://esp32forth.appspot.com/ESP32forth.html) or its [*GitHub site*](https://github.com/Esp32forth)
