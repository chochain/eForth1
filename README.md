# eForth1 - eForth for Arduino UNO

In 2011, Dr. Chen-Hanson Ting created [328eForth](https://chochain.github.io/eForth1/ref/328eForth.pdf) to run Forth on Arduino UNO. Wrote in his ceForth_33 document, the creator of eForth, and one of the inspiring figures of Forth community noted:
> *In 2011, I was attracted to Arduino Uno Kit and ported eForth to it as 328eForth...writing to flash memory, I had to take over the bootload section which was monopolized by Arduino IDE...I extended Forth dictionary in the RAM memory. It worked. However, you had only 1.5KB of RAM memory left over for new Forth words, and you could not save these new words before you lost power. As I stated then, it was only a teaser to entice new people to try Forth on Arduino Uno.*

Before Dr. Ting conceded his fight with cancer in May, 2022, I've spent the last 11 months working with him expanding the concept of "Forth without Forth" - a new eForth model - he called. Traditionally, Forth is built with a set of core words in low-level assembly language and establish the rest of words with high-level Forth scripts which get boot-strapped on start-up time. With the proliferation of modern languages, build Forth without using Forth, he reasoned that Forth built entirely in high-level languages can not only greatly simplify the virtual machine, taking advantage of modern languages and operating system, but also encourage portability and optimization. In a few months, we completed new batch of eForths in Javascript, Java, C++, ported them to Windows, ESP32, which Dr. Ting presented his video on November, 2021 to [Sillicon Valley Forth Interest Group](https://www.youtube.com/watch?v=bb5vi9kR1tE&t=827s). Later, we focused on SystemVerilog's edition with Don & Demitri's CORE-I FPGA project of [AI & Robotics Group](https://www.facebook.com/groups/1304548976637542) til his eventual departure.

In Dr. Ting's own word:
> *In all these years, I have thought that the eForth Model is a good model useful for all different processors and microcontrollers, and for all different applications. It is a very simple model for anybody who like to learn Forth and to use it for their own applications.*

I enjoy the beauty of working on something small and simple, so decided to pick up Dr. Ting's eForth Model and have fun with it. However, to make it useful, there are a few things that I need to overcome first i.e. make macro assembler C-coder friendly, remove dependency on extra bootloader programmer, add EEPROM save/load, and support interrupts. So, here we go!

### What is eForth1?

* An eForth for Arduino UNO/Nano implemented in C.
* A .ino file that can be openned in Arduino IDE and load/run directly onto Arduino.
* Has 16-bit cells and stacks.
* Can read/write Arduino pins.
* Supports Arduino Interrupts.
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
  > \> git clone *https://github.com/chochain/eForth1* onto your local Sketch directory
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

### Demos
  * LED blinker (assume you have a blue LED on pin 6)
    <pre>
    > : toggle 6 in 1 xor 6 out ;⏎           \ create a word to toggle the blue LED
    > : blink for toggle 500 delay next ;⏎   \ create a word to blink
    > 9 blink⏎                               \ run 10 cycles (i.e.9,8,7,...,2,1,0 to on/off 5 times)
    </pre>  

  * Timer Interrupt (LED on pin 5)
    <pre>
    > : my_isr 5 in 1 xor 5 out ;⏎           \ create an interrupt service routine (just a regular word)
    > ' my_isr 2 tmr⏎                        \ make the ISR ticked every 0.2 seconds (2 x 0.1 seconds)
    > 1 tmre⏎                                \ enable timer, now you should see red LED blinking
    > 19 blink⏎                              \ let's have them both blink (blue LED 10 times) 
    </pre>
    
    |||
    |:--|:--|
    |@htmlonly <iframe width="400" height="225" src="https://www.youtube.com/embed/--iLaLC5cG0?version=3&playlist=--iLaLC5cG0&loop=1&controls=0" title="" frameborder="0" allow="autoplay; picture-in-picture" allowfullscreen></iframe> @endhtmlonly|@htmlonly <iframe width="400" height="225" src="https://www.youtube.com/embed/gr3OVOcgF4Q?version=3&playlist=gr3OVOcgF4Q&loop=1&controls=0" title="" frameborder="0" allow="autoplay; picture-in-picture" allowfullscreen></iframe> @endhtmlonly|

### To Learn More About Forth?
If your programming language exposure has been with C, Java, or even Python so far, FORTH is quite **different**. Quote Nick: <em>"It's no functional or object oriented, it doesn't have type-checking, and it basically has zero syntax"</em>. No syntax? So, anyway, before you dive right into the deep-end, here's a good online materials.
* Interactive tutorial for FORTH primer. It teaches you how FORTH fundamentally works such as the numbers, the stack, and the dictionary.
  > <a href="https://skilldrick.github.io/easyforth/#introduction" target="_blank">Easy Forth Tutorial by Nick Morgan</a> with a [Writeup by Juergen Pintaske](https://wiki.forth-ev.de/lib/exe/fetch.php/en:projects:a-start-with-forth:05_easy_forth_v16_a5_withexp_comments.pdf?fbclid=IwAR0sHmgiDtnMRuQtJdVkhl9bmiitpgcjs4ZlIDVtlxrssMOmLBv0vesvmKQ")

To understand the philosophy of FORTH, excellent online e-books are here free for you.
* Timeless classic for the history, paths, and thoughts behind FORTH language.
  > <a href="http://home.iae.nl/users/mhx/sf.html" target="_blank">Starting Forth by Leo Brodie</a><br/>
  > <a href="http://thinking-forth.sourceforge.net" target="_blank">Thinking Forth by Leo Brodie</a>

### Performance Tuning
* Check your Arduino IDE installed directory, say <em>C:\Users\myname\AppData\Local\Arduino...</em> on Windows or <em>/home/myname/Arduino/...</em> on Linux,
* Find the directory *hardware -> arduino -> avr*,
* With an editor, open the <em>'platform.txt'</em> file,
* Find all three -Os compiler options (i.g. compiler.c.flags=-c -g -Os ...)
  > change them to -O3 for speed, -Os (default) for smallest size, -O2 for somewhere in-between

### References to Dr. Ting's Original
* [328eForth documentation - table of content](https://chochain.github.io/eForth1/ref/328eForth_contents.pdf) and [full doc](https://chochain.github.io/eForth1/ref/328eForth.pdf)
* [328eForth mod for coinForth by D. Ruffer](https://github.com/DRuffer/328eforth)
* [ceForth_33.doc - original documentation](https://chochain.github.io/eForth1/ref/ceForth_33.doc)
* [ceForth_33.cpp - in C, source assembler + VM](https://chochain.github.io/eForth1/ref/ceForth_33.cpp)
* [eforth_328.ino - teaser by Dr. Ting for Arduino IDE](https://chochain.github.io/eForth1/ref/eforth_328.ino)
* [eForth for STM8 - for even smaller apps](https://github.com/TG9541/stm8ef) and [STM8 Programming](https://github.com/TG9541/stm8ef/wiki/STM8S-Programming#flashing-the-stm8)
* [ESP32Forth for ESP32 - for larger/fancier apps](https://github.com/Esp32forth)
