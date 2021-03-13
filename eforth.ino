/******************************************************************************/
/* Arduino eForth.C, 1.0: For Atmega328 on arduino Uno                        */
/******************************************************************************/
/* Chen-Hanson Ting                                                           */
/* Version 2.0, 06nov11cht                                                    */
/* Compile new commands in RAM                                                */
/*                                                                            */
/* Version 1.0, 21sep11cht                                                    */
/* Adopted from eForth_11.c                                                   */
/* Compiled by Arduino as a sketch                                            */
/* Follow closely the original eForth model                                   */
/* Kernel has 32 primitives                                                   */
/* code[] array must be filled with rom.mif produced by cefMETA328            */
/* @, !, C@ and C! access RAM memory                                          */
/******************************************************************************/
#include <stdint.h>
#include "eforth.h"

extern int  code[];
U8   *cCode = (U8*)code;
int  rack[32]  = {0}, *R = rack;
int  stack[32] = {0}, *S = stack;

int  I, P, IP;
int  n, w, clock, phase, top;

const PROGMEM char SPC[] = " ";

void dumpStack(void)
{
    Serial.print(F("\nS=") );
    for (int n=0; n<=(S-stack); n++) {
        Serial.print(stack[n], HEX);
        Serial.print(SPC);
    } 
    Serial.println(top, HEX);
    Serial.print(F("R="));
    for (int n=0; n<=(R-rack); n++) {
        Serial.print(rack[n], HEX);
        Serial.print(SPC);
    } 
    Serial.println(F("")); 
}
void jump(void)     { clock |= 3;                  } 
void next(void)     { P=AT(P); IP+=2; jump();      }
void bye()          { exit(0);                     } 
void qrx(void)
{
    while (Serial.available()==0) {};
    push(Serial.read());
    push(0xffff);
}
void txsto(void)   { Serial.write((U8)top); pop();  } 
void emit(void)    { txsto();                       } 
void docon(void)   { push(AT(P));  P+=2;          }
void dolit(void)   { push(AT(IP)); IP+=2; next(); }
void dolist(void)  { *++R=IP; IP=P; next();         }
void exitt(void)   { IP=*R--; next();               } 
void execu(void)   { *++R=IP; P=top; pop(); jump(); } 
void donext(void)
{
    if(*R) { *R -= 1; IP=AT(IP); }
    else   { IP += 2; R--;         }
    next();
} 
void qbran(void)   { IP = top ? IP+2 : AT(IP); pop(); next(); }
void bran(void)    { IP = AT(IP); next();                     }
void store(void)   { data[top>>1] = *S--; pop();              }
void cstore(void)  { cData[top] = (char) *S--; pop();         }
void at(void)      { top = AT(top);     }
void cat(void)     { top = CAT(top);    }
void icat(void)    { top = BYTE(top);   } 
void iat(void)     { top = FETCH(top);  }
void istore(void)  { pop(); pop();      } 
void icstore(void) { pop(); pop();      }
void rfrom(void)   { push(*R--);        } 
void rat(void)     { push(*R);          }
void tor(void)     { *++R = top; pop(); } 
void rpsto(void)   { R = rack;          } 
void spsto(void)   { S = stack;         } 
void drop(void)    { pop();             } 
void dup(void)     { *++S = top;        } 
void swap(void)    { w = top; top = *S; *S = w; } 
void over(void)    { push(S[-1]);       } 
void zless(void)   { top = LOGICAL(top & 0x8000); }
void andd(void)    { top &= *S--;       } 
void orr(void)     { top |= *S--;       } 
void xorr(void)    { top ^= *S--;       }
void uplus(void)   { *S += top; top = LOWER(*S, top); } 
void nop(void)     { jump();            } 
void dovar(void)   { push(P);           }

void (*primitives[])(void) = {
    /* case 0 */ nop,
    /* case 1 */ bye, 
    /* case 2 */ qrx,    
    /* case 3 */ txsto,  
    /* case 4 */ docon, 	
    /* case 5 */ dolit,
    /* case 6 */ dolist,
    /* case 7 */ exitt,
    /* case 8 */ execu,
    /* case 9 */ donext,
    /* case 10 */ qbran,
    /* case 11 */ bran,
    /* case 12 */ store,
    /* case 13 */ at,
    /* case 14 */ cstore,
    /* case 15 */ cat,
    /* case 16 */ icat,
    /* case 17 */ iat,
    /* case 18 */ rfrom,
    /* case 19 */ rat,
    /* case 20 */ tor,
    /* case 21 */ dovar, 
    /* case 22 */ next,
    /* case 23 */ drop,
    /* case 24 */ dup,
    /* case 25 */ swap,
    /* case 26 */ over,
    /* case 27 */ zless,
    /* case 28 */ andd,
    /* case 29 */ orr,
    /* case 30 */ xorr,
    /* case 31 */ uplus,
    /* case 32 */ icat
};

void execute(U8 icode)
{
    if (icode < 33) {
        primitives[icode]();         // call primitive function
    }
    else {
        Serial.print (F("\nIllegal code="));
        Serial.print(icode, HEX);
        Serial.print(F(" P="));
        Serial.println(P, HEX );
    }
}
/* End of eforth.c */

void setup()
{ 
	clock = 0;
	P     = FETCH(0x480);
	IP    = 0;
	top   = 0;

    Serial.begin(9600);
	Serial.print(F("\nStart Arduino\n"));
}

void loop()
{
    static U16 I;

	if (!Serial.available()) {
        delay(100);
        return;
    }
    int n = Serial.read();
    switch((phase=clock&3)) {
    case 0: I = AT(P); P+=2;   break;          // fetch instruction
    case 1: execute(I & 0xff); break;
    case 2: execute(I >> 8);   break;
    case 3: jump();           
    }
    Serial.println(n, HEX);
    Serial.print(F("clock="));
    Serial.print(clock, HEX);
    Serial.print(F(" IP="));
    Serial.print(IP, HEX);
    Serial.print(F(" P="));
    Serial.print(P, HEX);
    Serial.print(F(" I="));
    Serial.print(I, HEX);
    
    dumpStack();
    
    clock += 1; 
}
