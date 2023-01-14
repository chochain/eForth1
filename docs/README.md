Note: To compile Dr. Ting's ceForth_33.cpp
* comment out <tchar.h> - for multi-byte char only
* make sure 'long' is 32-bit (instead of 64-bit) i.e. gcc -m32
  + typedef int           S32;    // replace long
  + typedef unsigned int  U32;    // replace unsigned long
  + typedef long          S64;    // replace long long
  + typedef unsigned long U64;    // replace unsigned long long

