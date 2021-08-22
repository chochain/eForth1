# eForthUNO
<a href="https://chochain.github.io/eForthUNO/html/index.html">eForth on Arduino UNO</a>

> 'rfcomm -a'   to find bluetooth device connection

## emacs
> * use M-x serial-term mode connect to /dev/rfcommX, baudrate 9600/N/8/1
> * use ^C^j to set line-mode (instead of default char-mode)

## minicom -D /dev/rfcommX -b 9600
> ^AZ to config
> * U to add Carrage Return
> * T.Tx line delay to 30ms, T.Tx char delay to 1ms
> * W to line wrap 
> * O.software line control=Yes
> * P to send file

## StandardCplusplus with ceForth.cpp
> * 43476 bytes (141%) Flash and 1749B (85%) RAM
