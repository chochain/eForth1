///
/// @file eforth_rom.c
/// @brief eForth ROM (loaded in Arduino Flash Memory)
/// @attention 8K max ROM before changing FORTH_ROM_SZ in eforth_core.h 
///
#include "eforth_core.h"
const U32 forth_rom_sz PROGMEM = 0xd7d;
const U32 forth_rom[] PROGMEM = {
0x0000610d,0x504f4e03,0x04000800,0x45594203,0x0c000801,0x454b3f04,0x00080259,0x4d450414, // 0000 _a.._NOP._.__BYE__.__?KEY__.__EM
0x08035449,0x44051d00,0x54494c4f,0x26000804,0x564f4405,0x08055241,0x45053000,0x5245544e, // 0020 IT__.__DOLIT__.&_DOVAR__.0_ENTER
0x3a000807,0x49584504,0x00080854,0x42510744,0x434e4152,0x00080a48,0x5242064d,0x48434e41, // 0040 __.:_EXIT__.D_QBRANCH__.M_BRANCH
0x5900080b,0x4e4f4406,0x09545845,0x07640008,0x43455845,0x06455455,0x016f0008,0x00080c21, // 0060 __.Y_DONEXT__.d_EXECUTE__.o_!__.
0x212b027b,0x8100080d,0x080e4001,0x43028800,0x00080f21,0x4043028e,0x95000810,0x113e5202, // 0080 {_+!__.__@__.__C!__.__C@__.__R>_
0x029c0008,0x08124052,0x3e02a300,0x00081352,0x524404aa,0x0814504f,0x4403b100,0x08155055, // 00a0 _.__R@__.__>R__.__DROP__.__DUP__
0x5304ba00,0x16504157,0x04c20008,0x5245564f,0xcb000817,0x544f5203,0xd4000818,0x43495004, // 00c0 .__SWAP__.__OVER__.__ROT__.__PIC
0x0008194b,0x4e4103dc,0x00081a44,0x524f02e5,0xed00081b,0x524f5803,0xf400081c,0x564e4906, // 00e0 K__.__AND__.__OR__.__XOR__.__INV
0x1d545245,0x06fc0008,0x4948534c,0x081e5446,0x52060701,0x46494853,0x01081f54,0x202b0112, // 0100 ERT__.__LSHIFT_____RSHIFT_____+ 
0x011d0108,0x0108212d,0x222a0123,0x01290108,0x0108232f,0x4f4d032f,0x01082444,0x454e0635, // 0120 ____-!__#_*"__)_/#__/_MOD$__5_NE
0x45544147,0x3d010825,0x08263e01,0x3d014801,0x4e010827,0x08283c01,0x30025401,0x0108293e, // 0140 GATE%__=_>&__H_='__N_<(__T_0>)__
0x3d30025a,0x6101082a,0x2b3c3002,0x02680108,0x082c2b31,0x31026f01,0x01082d2d,0x443f0476, // 0160 Z_0=*__a_0<+__h_1+,__o_1--__v_?D
0x082e5055,0x44057d01,0x48545045,0x8601082f,0x30505202,0x02900108,0x08314c42,0x43049701, // 0180 UP.__}_DEPTH/____RP0____BL1____C
0x324c4c45,0x059e0108,0x4c4c4543,0x0108332b,0x454305a7,0x342d4c4c,0x05b10108,0x4c4c4543, // 01a0 ELL2____CELL+3____CELL-4____CELL
0x01083553,0x424103bb,0x01083653,0x414d03c5,0x01083758,0x494d03cd,0x0108384e,0x495706d5, // 01c0 S5____ABS6____MAX7____MIN8____WI
0x4e494854,0xdd010839,0x50553e06,0x3a524550,0x05e80108,0x4e554f43,0x01083b54,0x3c5502f3, // 01e0 THIN9____>UPPER:____COUNT;____U<
0xfd01083c,0x2f4d5506,0x3d444f4d,0x03040208,0x3e2a4d55,0x020f0208,0x083f2a4d,0x55031702, // 0200 <____UM/MOD=____UM*>____M*?____U
0x08402b4d,0x2a051e02,0x444f4d2f,0x26020841,0x4f4d2f04,0x02084244,0x2f2a0230,0x39020843, // 0220 M+@____*/MODA__&_/MODB__0_*/C__9
0x443e5303,0x40020844,0x533e4403,0x48020845,0x454e4407,0x45544147,0x50020846,0x472b4402, // 0240 _S>DD__@_D>SE__H_DNEGATEF__P_D+G
0x025c0208,0x08482d44,0x32046302,0x49505544,0x056a0208,0x4f524432,0x02084a50,0x21320273, // 0260 ____D-H__c_2DUPI__j_2DROPJ__s_2!
0x7d02084b,0x4c403202,0x04840208,0x42495427,0x08002004,0x42048b02,0x04455341,0x02080220, // 0280 K__}_2@L____'TIB_ .____BASE_ ___
0x50430296,0x08042004,0x4307a102,0x45544e4f,0x20045458,0xaa020806,0x53414c04,0x08200454, // 02a0 __CP_ _____CONTEXT_ _____LAST_ _
0x05b80208,0x444f4d27,0x0a200445,0x06c30208,0x4f424127,0x20045452,0xcf02080c,0x444c4803, // 02c0 ____'MODE_ _____'ABORT_ _____HLD
0x080e2004,0x5304dc02,0x044e4150,0x02081020,0x493e03e6,0x1220044e,0x04f10208,0x42495423, // 02e0 _ _____SPAN_ _____>IN_ _____#TIB
0x08142004,0x7403fb02,0x2004706d,0x06030816,0x59454b03,0x14030a02,0x05100308,0x4148433e, // 0300 _ _____tmp_ _____KEY________>CHA
0x7f000452,0x0004151a,0x0a39317f,0x04143203,0x03085f00,0x4548041b,0xa4824552,0x3503080e, // 0320 R_.____._19__2__._____HERE_____5
0x44415003,0x08002504,0x54034003,0x90824249,0x4a03080e,0x4f4d4305,0x0b134556,0x10176603, // 0340 _PAD_%.__@_TIB_____J_CMOVE___f__
0x2c130f17,0x03092c11,0x03084a5e,0x4f4d0454,0x23324556,0x80030b13,0x0c170e17,0x33113313, // 0360 ___,_,__^J__T_MOVE2#_________3_3
0x4a780309,0x046d0308,0x4c4c4946,0x0b161316,0x0f499503,0x9203092c,0x8703084a,0x47494405, // 0380 __xJ__m_FILL______I_,___J____DIG
0x00045449,0x04281709,0x201a0700,0x20300004,0x079c0308,0x52545845,0x04544341,0x3d160000, // 03a0 IT_.__(_.__ _.0 ____EXTRACT_.._=
0x08a28316,0x3c02b303,0x82448323,0x03080ce0,0x4f4804c6,0xe082444c,0x82152d0e,0x080f0ce0, // 03c0 _______<#_D_______HOLD___-______
0x2301d103,0x830e9b82,0x08d683bb,0x2302e203,0x15e48353,0x0bfa030a,0x0308f103,0x495304ee, // 03e0 ___#___________#S_____________SI
0x0a2b4e47,0x00040b04,0x08d6832d,0x2302fd03,0xe082143e,0x1744830e,0x0e040821,0x52545303, // 0400 GN+____.-______#>_____D_!____STR
0x83361315,0x11f183c9,0x11840284,0x031c0408,0x04584548,0x9b821000,0x2f04080c,0x43454407, // 0420 __6_____________HEX_.______/_DEC
0x4c414d49,0x820a0004,0x04080c9b,0x4944063c,0x3f544947,0x00043a13,0x00042130,0x0a281709, // 0440 IMAL_.______<_DIGIT?_:_.0!_.__(_
0x00046c04,0x04152107,0x1b280a00,0x083c1115,0x4e074d04,0x45424d55,0x9b823f52,0x0004130e, // 0460 _l_._!__._(___<__M_NUMBER?_____.
0x173b1700,0x24000410,0x92040a27,0x2c163384,0x10172d16,0x272d0004,0x21121613,0x2e201216, // 0480 ._;___.$'____3_,_-___.-'___!__ .
0x2dcd040a,0x10131513,0x840e9b82,0xc5040a54,0x0e9b8216,0x2c112022,0x14a50409,0xc1040a12, // 04a0 ___-________T_______" _,________
0x040b1625,0x4a1111cc,0x0000044a,0x114a1115,0x080c9b82,0x53057204,0x45434150,0x04080331, // 04c0 %______JJ_..__J______r_SPACE1___
0x484305d6,0x16535241,0x37000004,0xf2040b13,0x04090315,0x040814f0,0x505306e1,0x53454341, // 04e0 __CHARS__..7______________SPACES
0x08e78431,0x5404f904,0x13455059,0x3b22050b,0x1a7f0004,0x7f000415,0x050a3931,0x00041421, // 0500 1______TYPE___";_.____._19__!__.
0x0509035f,0x0508140f,0x52430206,0x030a0004,0x03290508,0x11246f64,0x203b1112,0x08131613, // 0520 __________CR_.____)_do$___; ____
0x24033305,0x37857c22,0x03420508,0x857c222e,0x0b853b37,0x024b0508,0x8413522e,0x21171120, // 0540 _3_$"|_7__B_."|_7;____K_.R__ __!
0x0b850085,0x03570508,0x13522e55,0xf183c983,0x17111184,0x85008521,0x6705080b,0x832e5502, // 0560 _.____W_U.R_________!_.____g_U._
0x84f183c9,0x85dc8411,0x7c05080b,0x9b822e01,0x0a00040e,0x9b050a1c,0x84087f85,0x85dc8420, // 0580 ___________|_.____._________ ___
0x8c05080b,0x850e3f01,0xa405088e,0x61702807,0x29657372,0x170f0a83,0x060a1513,0x0a832d05, // 05a0 _____?_______(parse)_________-__
0x0a273110,0x3113dc05,0x2b211017,0xdb050a1d,0xc705092c,0x00041411,0x11081500,0x83131617, // 05c0 _1'____1__!+____,______.._______
0x1017100a,0x100a8321,0x050a2731,0x050a2bee,0x05092cfa,0x0b1315df,0x1411ff05,0x17132c15, // 05e0 ____!___1'___+___,___________,__
0x21111121,0x21111708,0x05ac0508,0x4b434150,0x49131524,0x83162c0f,0x0608115a,0x4150050b, // 0600 !__!___!____PACK$__I_,__Z_____PA
0x13455352,0xf5824e83,0x0083200e,0x0ef5820e,0xb4851121,0x080df582,0x54051d06,0x4e454b4f, // 0620 RSE__N___ _.____!__________TOKEN
0x04238631,0x83381f00,0x1186333a,0x043a0608,0x44524f57,0x3a832386,0x08118633,0x4e054f06, // 0640 1_#_._8_:3____:_WORD_#_:3____O_N
0x3e454d41,0x1f00043b,0x0608201a,0x4153055e,0x133f454d,0x91060b49,0x133a1015,0x1015162c, // 0660 AME>;_.__ __^_SAME?_I_____:_,___
0x162c133a,0x2e211111,0x1191060a,0x114a1314,0x78060908,0x0000044a,0x046d0608,0x444e4946, // 0680 :_,___!.______J____xJ_..__m_FIND
0x83101516,0x0e150c0a,0x0e163313,0xd8060a15,0x3f040e15,0x5f041aff,0x04121a5f,0x1c1a5f5f, // 06a0 _________3_________?____________
0x33ca060a,0x0bffff04,0x8333d506,0x152d0e0a,0x86d5060a,0xde060b73,0x34161411,0x060a0816, // 06c0 ___3______3___-_____s______4____
0x0b3434e6,0x1411ab06,0x15341416,0x08166486,0x4e059b06,0x3f454d41,0xa086b282,0x02f20608, // 06e0 _44_______4__d_____NAME?________
0x1713485e,0x1c171611,0x0416070a,0x2d030800,0x00040331,0x06080308,0x415403ff,0x17031550, // 0700 ^H__________.__-1__.______TAP___
0x07082c0f,0x546b0419,0x04155041,0x171c0d00,0x1c0a0004,0x48070a1a,0x1c080004,0x3145070a, // 0720 _,____kTAP__.____._____H_.____E1
0x070b1d87,0x08028747,0x15141614,0x06250708,0x45434341,0x20175450,0x0a1c4917,0x14837407, // 0740 ____G_________%_ACCEPT_ _I___t__
0x04213115,0x0a3c5f00,0x1d876f07,0x8771070b,0x59070b2a,0x08211714,0x45064f07,0x43455058, // 0760 _1!_._<__o____q_*__Y__!__O_EXPEC
0x82568754,0x08140ceb,0x51057a07,0x59524555,0x00044e83,0x83568780,0x04140c00,0xf5820000, // 0780 T_V______z_QUERY_N_.__V_.___..__
0x8a07080c,0x4f424105,0xd6825452,0x070a2e0e,0x070806b2,0x524505a4,0x84524f52,0x0b853bdc, // 07a0 _____ABORT___.________ERROR__;__
0x033f0004,0xaa872c85,0x240ab507,0x45544e49,0x45525052,0x2ef88654,0x10f8070a,0x1a400004, // 07c0 _.?__,_____$INTERPRET__._____.@_
0x14f6070a,0x630c4f85,0x69706d6f,0x6f20656c,0x0b796c6e,0x0806f707,0x080a7a84,0x080b0801, // 07e0 _____O_compile only______z______
0x07bb8703,0x045b81ca,0xc982d507,0x0508080c,0x4b4f2e03,0x07042c85,0x0ec982d5,0x35080a27, // 0800 ______[__________.OK_,______'__5
0x0400042f,0x19121338,0x08098e85,0x054f8526,0x3e6b6f20,0x10080820,0x41564504,0x1540864c, // 0820 /_._8_______&_O_ ok> ____EVAL_@_
0x4f080a10,0x2e0ec982,0x064c080a,0x143d080b,0x08081488,0x55510438,0x24045449,0x0c908280, // 0840 ___O___.__L___=_____8_QUIT_$____
0x90870788,0x080b3d88,0x01550862,0x153a832c,0x0ca48233,0x6b08080c,0x832c4302,0x822c153a, // 0860 _____=__b_U_,_:_3______k_C,_:_,_
0x080f0ca4,0x4c877808,0x52455449,0x00044c41,0x887b8804,0x8608086d,0x4c4c4105,0xa482544f, // 0880 _____x_LITERAL_.__{_m____ALLOT__
0x9808080d,0x4d4f4307,0x454c4950,0x88101511,0x08132c7b,0x2408a408,0x504d4f43,0x86454c49, // 08a0 _____COMPILE____{,_____$COMPILE_
0x080a2ef8,0x000410ef,0x080a1a80,0x080b06d1,0x041515ee,0x16280020,0x0004102c,0x0a1a2708, // 08c0 _._____.____________ .(_,__._'__
0x8810e808,0xee080b7b,0x1b008004,0x84086d88,0xf7080a7a,0x87088e88,0x07b608bb,0x494e553f, // 08e0 ____{_____.__m__z___________?UNI
0x15455551,0x0a2ef886,0x043b1c09,0x841a1f00,0x850b85dc,0x7220064f,0x66654465,0xfb080814, // 0900 QUE___.___;_._______O_ reDef____
0x6e2c2403,0x090a0e15,0x1503893c,0xa4826486,0xbd82150c,0xb282340c,0x080c160e,0x2009bb87, // 0920 _$,n____<____d_______4_________ 
0x40862701,0x090af886,0xbb87084a,0x5d014009,0x82bf0804,0x09080cc9,0x435b894e,0x49504d4f, // 0940 _'_@____J____@_]________N_[COMPI
0x895d454c,0x086d8842,0x3a015909,0x24894086,0x09085089,0x883b816a,0x078808ac,0x820ebd82, // 0960 LE]_B_m__Y_:_@_$_P__j_;_________
0x09080cb2,0x4e3e0575,0x2d454d41,0x00041015,0x00041a7f,0x090a2820,0x8509088b,0x4d554404, // 0980 ____u_>NAME-___.___. (_______DUM
0x0e9b8250,0x04338413,0x04201f00,0x13231000,0x85ef090b,0x1000042c,0x00041749,0x136b8505, // 09a0 P_____3_._ _._#_____,_._I__.__k_
0x033a0004,0x84e4090b,0x041015dc,0xbb831000,0x10000416,0x1403bb83,0x04122c03,0x0a270800, // 09c0 _.:_________._____.______,__._'_
0xdc84e409,0x13c70909,0xdc84dc84,0x09110b85,0x1114b309,0x080c9b82,0x2e039c09,0xb282504f, // 09e0 ___________________________.OP__
0x0a0a150e,0x64861525,0x04102c15,0x0a270800,0x1310200a,0x0a271117,0x853b1f0a,0x1508140b, // 0a00 ____%__d_,__._'__ ____'___;_____
0x0a0b3414,0x08141400,0x2e05fa09,0x52444441,0x85152c85,0x3a00048e,0x2a0a0803,0x45455303, // 0a20 _4__.______.ADDR_,____.:___*_SEE
0x308a4289,0x04151015,0x1d270800,0x15cb0a0a,0x1a800004,0x14690a0a,0x7f040e15,0xdc841aff, // 0a40 _B_0____._'______.____i_________
0x853b8b89,0x0a0b330b,0x000415c8,0x0a0a2704,0x152c147b,0x338e850e,0x15c80a0b,0x270b0004, // 0a60 __;__3_____._'__{_,____3_____._'
0x14930a0a,0x850e152c,0x6a00048e,0x308a3303,0x15c80a0b,0x270a0004,0x14ab0a0a,0x850e152c, // 0a80 ____,_____.j_3_0_____._'____,___
0x3f00048e,0x308a3303,0x15c80a0b,0x27090004,0x14c30a0a,0x850e152c,0x7200048e,0x308a3303, // 0aa0 __.?_3_0_____._'____,_____.r_3_0
0x84c80a0b,0x2cfe89dc,0x14440a0b,0x04dc8414,0x08033b00,0x57053c0a,0x5344524f,0xb2822c85, // 0ac0 _______,__D_____.;___<_WORDS_,__
0x83000004,0x2e0e0c0a,0x15130b0a,0x1f00043b,0x2c2c151a,0x850d0a83,0x84dc840b,0x0a8334dc, // 0ae0 _..____.____;_.___,,_________4__
0x4400040e,0x100b0a26,0x00042c85,0x0c0a8300,0x08e60a0b,0x4606d60a,0x4547524f,0x86408654, // 0b00 __.D&____,_..__________FORGET_@_
0x0b0a2ef8,0x82153434,0x150e0ca4,0x820cb282,0x08140cbd,0x160bbb87,0x45484185,0xac884441, // 0b20 _.__44___________________AHEAD__
0x043a830b,0x6d880000,0x85380b08,0x49414741,0x0bac884e,0x0b086d88,0x4542854b,0x834e4947, // 0b40 __:_.._m__8_AGAIN____m__K_BEGIN_
0x590b083a,0x544e5585,0xac884c49,0x640b080a,0x88464982,0x3a830aac,0x88000004,0x700b086d, // 0b60 :__Y_UNTIL_____d_IF____:_.._m__p
0x45485484,0x163a834e,0x800b080c,0x534c4584,0x163e8b45,0x0b08858b,0x4857858c,0x8b454c49, // 0b80 _THEN_:______ELSE_>_______WHILE_
0x0b081673,0x48578499,0x738b4e45,0xa50b0817,0x50455286,0x8b544145,0x08858b51,0x4683b00b, // 0ba0 s_____WHEN_s_____REPEAT_Q______F
0xac88524f,0x083a8313,0x4183be0b,0x8b145446,0x163a833e,0x84ca0b08,0x5458454e,0x8809ac88, // 0bc0 OR____:____AFT__>_:_____NEXT____
0xd70b086d,0x222c2403,0x86220004,0x82203b54,0x0b080ca4,0x222482e4,0x83468504,0xe88b0c3a, // 0be0 m____$,"_."_T; _______$"__F_:___
0x82f50b08,0x8504222e,0x0c3a834f,0x0c08e88b,0x4f430403,0x40864544,0xbd822489,0x0cb2820e, // 0c00 ____."__O_:_______CODE_@_$______
0x06110c08,0x41455243,0x168c4554,0x8805ac88,0x0c0808ac,0x41560823,0x42414952,0x2a8c454c, // 0c20 ____CREATE__________#_VARIABLE_*
0x88000004,0x350c086d,0x4e4f4308,0x4e415453,0x04168c54,0x7b880400,0x88080004,0x086d887b, // 0c40 _.._m__5_CONSTANT___.__{_.__{_m_
0x2e82480c,0x29000428,0x0b852386,0x81620c08,0x0a00045c,0x08145486,0x28816f0c,0x86290004, // 0c60 _H_.(_.)_#____b___.__T___o_(_.)_
0x0c084a23,0x4f430c7a,0x4c49504d,0x4e4f2d45,0xbd82594c,0x0410150e,0x161b4000,0x850c080f, // 0c80 #J__z_COMPILE-ONLY______.@______
0x4d4d4909,0x41494445,0xbd824554,0x0410150e,0x161b8000,0xa00c080f,0x4f4c4305,0x084d4b43, // 0ca0 _IMMEDIATE______.________CLOCKM_
0x5007b80c,0x4f4d4e49,0x084e4544,0x4d03c20c,0x084f5041,0x4902ce0c,0x0c08504e,0x554f03d6, // 0cc0 ___PINMODEN____MAPO____INP____OU
0x0c085154,0x494103dd,0x0c08524e,0x575003e5,0x0c08534d,0x4d5405ed,0x54525349,0x05f50c08, // 0ce0 TQ____AINR____PWMS____TMISRT____
0x53494350,0x0c085552,0x495405ff,0x5652454d,0x05090d08,0x4e494350,0x0d085754,0x52540513, // 0d00 PCISRU____TIMERV____PCINTW____TR
0x58454341,0x041d0d08,0x45564153,0x270d0859,0x414f4c04,0x0d085a44,0x41430430,0x085b4c4c, // 0d20 ACEX____SAVEY__'_LOADZ__0_CALL[_
0x4405390d,0x59414c45,0x83474d44,0x0a834b0a,0x2b484d4c,0x0d0a1416,0x420d084e,0x4c4f4304, // 0d40 _9_DELAYDMG__K__LMH+____N__B_COL
0x5c0d0444,0x040cb282,0xbd825c0d,0xd507040c,0x040cc982,0xd6825a08,0x882c850c,0x0000005a, // 0d60 D____________________Z____,_Z...
};

