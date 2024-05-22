///
/// @file eforth_rom.c
/// @brief eForth ROM (loaded in Arduino Flash Memory)
/// @attention 8K max ROM before changing FORTH_ROM_SZ in eforth_core.h 
///
#include "eforth_core.h"
const U32 forth_rom_sz PROGMEM = 0xecd;
const U32 forth_rom[] PROGMEM = {
0x0000b10e,0x504f4e03,0x04000100,0x49584504,0x00010154,0x4e45050c,0x02524554,0x03150001, // 0000 __.._NOP._.__EXIT__.__ENTER__.__
0x03455942,0x041f0001,0x59454b3f,0x27000104,0x494d4504,0x00010554,0x4f440530,0x0654494c, // 0020 BYE__.__?KEY__.'_EMIT__.0_DOLIT_
0x05390001,0x41564f44,0x00010752,0x42510743,0x434e4152,0x00010b48,0x5242064d,0x48434e41, // 0040 _.9_DOVAR__.C_QBRANCH__.M_BRANCH
0x5900010c,0x4e4f4406,0x0a545845,0x07640001,0x43455845,0x08455455,0x016f0001,0x00010d21, // 0060 __.Y_DONEXT__.d_EXECUTE__.o_!__.
0x212b027b,0x8100010e,0x010f4001,0x43028800,0x00011021,0x4043028e,0x95000111,0x123e5202, // 0080 {_+!__.__@__.__C!__.__C@__.__R>_
0x029c0001,0x01134052,0x3e02a300,0x00011452,0x524404aa,0x0115504f,0x4403b100,0x01165055, // 00a0 _.__R@__.__>R__.__DROP__.__DUP__
0x5304ba00,0x17504157,0x04c20001,0x5245564f,0xcb000118,0x544f5203,0xd4000119,0x43495004, // 00c0 .__SWAP__.__OVER__.__ROT__.__PIC
0x00011a4b,0x4e4103dc,0x00011b44,0x524f02e5,0xed00011c,0x524f5803,0xf400011d,0x564e4906, // 00e0 K__.__AND__.__OR__.__XOR__.__INV
0x1e545245,0x06fc0001,0x4948534c,0x011f5446,0x52060701,0x46494853,0x01012054,0x212b0112, // 0100 ERT__.__LSHIFT_____RSHIFT ____+!
0x011d0101,0x0101222d,0x232a0123,0x01290101,0x0101242f,0x4f4d032f,0x01012544,0x454e0635, // 0120 ____-"__#_*#__)_/$__/_MOD%__5_NE
0x45544147,0x3d010126,0x01273e01,0x3d014801,0x4e010128,0x01293c01,0x30025401,0x01012a3e, // 0140 GATE&__=_>'__H_=(__N_<)__T_0>*__
0x3d30025a,0x6101012b,0x2c3c3002,0x02680101,0x012d2b31,0x31026f01,0x01012e2d,0x443f0476, // 0160 Z_0=+__a_0<,__h_1+-__o_1-.__v_?D
0x012f5055,0x44057d01,0x48545045,0x86010130,0x31505202,0x02900101,0x01324c42,0x43049701, // 0180 UP/__}_DEPTH0____RP1____BL2____C
0x334c4c45,0x039e0101,0x34534241,0x03a70101,0x3558414d,0x03af0101,0x364e494d,0x06b70101, // 01a0 ELL3____ABS4____MAX5____MIN6____
0x48544957,0x01374e49,0x3e06bf01,0x45505055,0x01013852,0x4f4305ca,0x39544e55,0x02d50101, // 01c0 WITHIN7____>UPPER8____COUNT9____
0x013a3c55,0x5506df01,0x4f4d2f4d,0x01013b44,0x4d5503e6,0x01013c2a,0x2a4d02f1,0xf901013d, // 01e0 U<:____UM/MOD;____UM*<____M*=___
0x2b4d5503,0x0002013e,0x4d2f2a05,0x013f444f,0x2f040802,0x40444f4d,0x02120201,0x01412f2a, // 0200 _UM+>__._*/MOD?____/MOD@____*/A_
0x53031b02,0x0142443e,0x44032202,0x0143533e,0x44072a02,0x4147454e,0x01444554,0x44023202, // 0220 ___S>DB__"_D>SC__*_DNEGATED__2_D
0x0201452b,0x2d44023e,0x45020146,0x16213202,0x02000614,0x0d120d21,0x024c0201,0x14164032, // 0240 +E__>_D-F__E_2!___._!_____L_2@__
0x0006120f,0x010f2102,0x32045b02,0x18505544,0x6a020118,0x52443205,0x1515504f,0x05740201, // 0260 ___._!___[_2DUP____j_2DROP____t_
0x41575332,0x19141950,0x7f020112,0x564f3205,0x00065245,0x00061a03,0x02011a03,0x4543058c, // 0280 2SWAP________2OVER_.___.______CE
0x332b4c4c,0x9d020121,0x4c454305,0x22332d4c,0x05a80201,0x4c4c4543,0x01233353,0x3202b302, // 02a0 LL+3!____CELL-3"____CELLS3#____2
0x0200062b,0xbe020121,0x062d3202,0x01220200,0x3202c802,0x0100062a,0xd202011f,0x062f3202, // 02c0 +_._!____2-_._"____2*_.______2/_
0x01200100,0x5302dc02,0x00240630,0x03e60201,0x47405053,0x01ef0201,0x02011349,0x525405f7, // 02e0 ._ ____S0_$.____SP@G____I_____TR
0x48454341,0x04fd0201,0x45564153,0x07030149,0x414f4c04,0x03014a44,0x41430410,0x014b4c4c, // 0300 ACEH____SAVEI____LOADJ____CALLK_
0x27041903,0x06424954,0x03010020,0x41420422,0x20064553,0x2d030102,0x06504302,0x03010420, // 0320 ___'TIB_ .__"_BASE_ ___-_CP_ ___
0x4f430738,0x5845544e,0x06200654,0x04410301,0x5453414c,0x01082006,0x27054f03,0x45444f4d, // 0340 8_CONTEXT_ ___A_LAST_ ___O_'MODE
0x010a2006,0x27065a03,0x524f4241,0x0c200654,0x03660301,0x06444c48,0x03010e20,0x50530473, // 0360 _ ___Z_'ABORT_ ___f_HLD_ ___s_SP
0x20064e41,0x7d030110,0x4e493e03,0x01122006,0x23048803,0x06424954,0x03011420,0x6d740392, // 0380 AN_ ___}_>IN_ _____#TIB_ _____tm
0x16200670,0x039d0301,0x0459454b,0x01ab030b,0x3e05a703,0x52414843,0x1b7f0006,0x7f000616, // 03a0 p_ _____KEY________>CHAR_.____._
0x030b3732,0x000615c9,0xb203015f,0x52454804,0x0f3b8345,0x03cc0301,0x06444150,0x03010025, // 03c0 27_____._____HERE_;_____PAD_%.__
0x495403d7,0x0f278342,0x05e10301,0x564f4d43,0x030c1445,0x181118fd,0x122d1410,0xf5030a2d, // 03e0 __TIB_'_____CMOVE_________-_-___
0x03011515,0x4f4d04eb,0x24334556,0x1a040c14,0x0d180f18,0x12213314,0x040a2133,0x01151510, // 0400 ______MOVE3$_________3!_3!______
0x46040504,0x174c4c49,0x040c1714,0x106f8231,0x2d040a2d,0x04011515,0x49440522,0x06544947, // 0420 ___FILL_____1_o_-__-____"_DIGIT_
0x29180900,0x1b070006,0x30000621,0x39040121,0x54584507,0x54434152,0x17000006,0x3f84173b, // 0440 .__)_.__!_.0!__9_EXTRACT_.._;__?
0x02500401,0xdb83233c,0x010d7783,0x48046304,0x83444c4f,0x162e0f77,0x100d7783,0x016e0401, // 0460 __P_<#___w___c_HOLD_w_.__w____n_
0x0f328323,0x73845884,0x027f0401,0x81845323,0x97040b16,0x018e040c,0x53048b04,0x2c4e4749, // 0480 #_2__X_s____#S_____________SIGN,
0x06a8040b,0x73842d00,0x029a0401,0x83153e23,0xdb830f77,0x04012218,0x545303ab,0x34141652, // 04a0 ____.-_s____#>__w____"____STR__4
0x8e846684,0x849f8412,0xb90401ae,0x58454803,0x83100006,0x04010d32,0x454407cc,0x414d4943, // 04c0 _f___________HEX_.__2_____DECIMA
0x0a00064c,0x010d3283,0x4406d904,0x54494749,0x0638143f,0x06223000,0x29180900,0x0609050b, // 04e0 L_.__2_____DIGIT?_8_.0"_.__)____
0x16220700,0x290a0006,0x3a12161c,0x07ea0401,0x424d554e,0x833f5245,0x06140f32,0x39180000, // 0500 ._"__._)___:____NUMBER?_2___.._9
0x00061118,0x050b2824,0x17d0842f,0x182e172d,0x2d000611,0x13171428,0x21131722,0x6c050b2f, // 0520 ___.$(__/___-_.___.-(___"__!/__l
0x1416142e,0x0f328311,0x050bf184,0x32831762,0x1221230f,0x42050a2d,0x050b1315,0x0c17265e, // 0540 ._____2_____b__2_#!_-__B____^&__
0x12126b05,0x7a827a82,0x16000006,0x127a8212,0x010d3283,0x53050f05,0x45434150,0x05010532, // 0560 _k___z_z_..___z__2_____SPACE2___
0x48430576,0x17535241,0x35000006,0x92050c14,0x050a0516,0x05011590,0x50530681,0x53454341, // 0580 v_CHARS__..5______________SPACES
0x01878532,0x54049905,0x14455059,0x39c2050c,0x1b7f0006,0x7f000616,0x050b3732,0x000615c1, // 05a0 2______TYPE____9_.____._27_____.
0x050a055f,0x050115af,0x524302a6,0x050a0006,0x03c90501,0x12246f64,0x21391213,0x01141714, // 05c0 __________CR_.______do$___9!____
0x2403d305,0xd7857c22,0x03e20501,0x857c222e,0xab8539d7,0x02eb0501,0x8414522e,0x221812bd, // 05e0 ___$"|______."|__9______.R_____"
0xab85a085,0x03f70501,0x14522e55,0x8e846684,0x1812ae84,0x85a08522,0x070601ab,0x842e5502, // 0600 ________U.R__f______"________U._
0x848e8466,0x857c85ae,0x1c0601ab,0x32832e01,0x0a00060f,0x3b060b1d,0x84011f86,0x857c85bd, // 0620 f_____|______._2__.____;______|_
0x2c0601ab,0x860f3f01,0x4406012e,0x61702807,0x29657372,0x1810a183,0x060b1614,0xa1832ea5, // 0640 ___,_?__.__D_(parse)_________.__
0x0b283211,0x32147c06,0x2c221118,0x7b060b1e,0x67060a2d,0x00061512,0x12011600,0x83141718, // 0660 _2(__|_2__",___{-__g___.._______
0x111811a1,0x11a18322,0x060b2832,0x060b2c8e,0x060a2d9a,0x0c14167f,0x15129f06,0x18142d16, // 0680 ____"___2(___,___-___________-__
0x22121222,0x22121801,0x054c0601,0x4b434150,0x82141624,0x172d106f,0x0112f183,0x5005ab06, // 06a0 "__"___"__L_PACK$___o_-________P
0x45535241,0x83e58314,0x83210f8c,0x8c830f97,0x8612220f,0x0e8c8354,0x05be0601,0x454b4f54, // 06c0 ARSE______!______"__T_______TOKE
0xc486324e,0x361f0006,0x2133d183,0x0601b186,0x4f5704db,0xc4864452,0x2133d183,0x0601b186, // 06e0 N2___._6__3!______WORD____3!____
0x414e05f1,0x393e454d,0x1b1f0006,0x01070121,0x4d415305,0x82143f45,0x36070c6f,0x14381116, // 0700 __NAME>9_.__!____SAME?__o__6__8_
0x1116172d,0x172d1438,0x2f221212,0x1236070b,0x7a821415,0x070a0112,0x067a821c,0x07010000, // 0720 -___8_-___"/__6____z______z_..__
0x49460410,0x1617444e,0x0da18311,0x33140f16,0x160f1721,0x1681070b,0xff3f060f,0x5f5f061b, // 0740 __FIND_________3!_________?_____
0x5f06131b,0x0b1d1b5f,0x21337207,0x0cffff06,0x21337e07,0x2e0fa183,0x7e070b16,0x070c1687, // 0760 _________r3!_____~3!___.___~____
0x17151288,0x01172233,0x3392070b,0x0c223322,0x15125207,0x22331517,0x17078716,0x05410701, // 0780 ____3"_____3"3"__R____3"______A_
0x454d414e,0x8749833f,0x9f070146,0x14485e02,0x18171218,0xc3070b1d,0x05080006,0x0605322e, // 07a0 NAME?_I_F____^H__________.__.2__
0x01050800,0x5403ac07,0x05165041,0x012d1018,0x6b04c607,0x16504154,0x1d0d0006,0x0a000618, // 07c0 .______TAP____-____kTAP__.____._
0x070b1b1d,0x080006f5,0xf2070b1d,0x0cca8732,0xaf87f407,0x15171501,0xd2070116,0x43434106, // 07e0 ______._____2________________ACC
0x18545045,0x6f821821,0x22080b1d,0x3216ab83,0x5f000622,0x1d080b3a,0x080cca87,0x0cd7871f, // 0800 EPT_!__o___"___2"_._:___________
0x18150608,0xfc070122,0x50584506,0x88544345,0x0d828303,0x28080115,0x45555105,0xe5835952, // 0820 ____"____EXPECT________(_QUERY__
0x88800006,0x0d978303,0x00000615,0x010d8c83,0x41053808,0x54524f42,0x2f0f6d83,0x0860080b, // 0840 _.________.._____8_ABORT_m_/__`_
0x05520801,0x4f525245,0x397c8552,0x0006ab85,0xcc85053f,0x63085888,0x4e49240a,0x50524554, // 0860 __R_ERROR_|9___.?____X_c_$INTERP
0x87544552,0x080b2fa5,0x000611a6,0x080b1b40,0xef8515a4,0x6d6f630c,0x656c6970,0x6c6e6f20, // 0880 RET__/_____.@________compile onl
0xa5080c79,0x17850108,0x01af080b,0x88b1080c,0x81780869,0x8308065b,0x010d6083,0x2e03b308, // 08a0 y_______________i_x_[____`_____.
0xcc854b4f,0x83830806,0x0b280f60,0x0630e308,0x14360400,0x2e861a13,0x85d4080a,0x6f2005ef, // 08c0 OK______`_(___0_._6____.______ o
0x01203e6b,0x4504be08,0x864c4156,0x0b1116e1,0x6083fd08,0x080b2f0f,0x080c08fa,0xc28815eb, // 08e0 k> ____EVAL________`_/__________
0x04e60801,0x54495551,0x83802406,0xb5880d27,0xeb883e88,0x0910090c,0x832c0103,0x213316d1, // 0900 ____QUIT_$__'____>________,___3!
0x0d0d3b83,0x02190901,0xd1832c43,0x3b832d16,0x0901100d,0x4c410527,0x83544f4c,0x09010e3b, // 0920 _;______C,___-_;____'_ALLOT_;___
0x494c8735,0x41524554,0x0600064c,0x1b892a89,0x07410901,0x504d4f43,0x12454c49,0x2a891116, // 0940 5_LITERAL_.__*____A_COMPILE____*
0x0901142d,0x43240853,0x49504d4f,0xa587454c,0x9e090b2f,0x80000611,0x80090b1b,0x9d090c08, // 0960 -___S_$COMPILE__/_____._________
0x20061616,0x2d172900,0x01000611,0x090b1b28,0x2a891197,0x069d090c,0x891c0080,0x1785011b, // 0980 ___ .)_-__._(______*_____.______
0x89a6090b,0x69880149,0x3f076509,0x51494e55,0x87164555,0x090b2fa5,0x000639cb,0x7c851b1f, // 09a0 ____I__i_e_?UNIQUE___/___9_.___|
0xef85ab85,0x65722006,0x15666544,0x03aa0901,0x166e2c24,0xec090b0f,0x8716b289,0x0d3b8307, // 09c0 _____ reDef_____$,n___________;_
0x0d548316,0x49832233,0x010d170f,0xcf096988,0xe1862701,0x090ba587,0x698801fa,0x5d01f009, // 09e0 __T_3"_I_____i___'_________i___]
0x836e0906,0x09010d60,0x435b89fe,0x49504d4f,0x895d454c,0x011b89f2,0x3a01090a,0xd389e186, // 0a00 __n_`_____[COMPILE]________:____
0x0a01008a,0x893b811a,0xb588015b,0x830f5483,0x0a010d49,0x4e3e0525,0x2e454d41,0x00061116, // 0a20 _.____;_[____T__I___%_>NAME.___.
0x00061b7f,0x0a0b2920,0x350a013b,0x4d554404,0x0f328350,0x06d08414,0x06211f00,0x14241000, // 0a40 ___. )__;__5_DUMP_2_____._!_._$_
0x85a00a0c,0x100006cc,0x06186f82,0x0b860500,0x3a000614,0x950a0c05,0x11167c85,0x84100006, // 0a60 ______.__o__._____.:_____|___.__
0x00061758,0x05588410,0x132d0515,0x28080006,0x85950a0b,0x780a0a7c,0x857c8514,0x12ab857c, // 0a80 X__.__X___-__._(____|__x__|_|___
0x15630a0a,0x0d328312,0x054c0a01,0x44524f57,0x83cc8553,0x00000649,0x0f0da183,0xe90a0b2f, // 0aa0 __c___2___L_WORDS___I_..____/___
0x00063916,0x2d161b1f,0x0ea1832d,0x7c85ab85,0x22337c85,0x060fa183,0x0b274400,0xcc85e60a, // 0ac0 _9_.___--______|_|3"____.D'_____
0x83000006,0x0a0c0da1,0xab0a01bb,0x524f4606,0x86544547,0x2fa587e1,0x330b0b0b,0x3b831622, // 0ae0 _..__________FORGET____/___3"__;
0x83160f0d,0x54830d49,0x8801150d,0x05ec0a69,0x4444412e,0x16cc8552,0x00062e86,0x0b01053a, // 0b00 ____I__T____i___.ADDR____._.:___
0x4f2e030f,0x00061650,0x0b0b1b80,0x0f161558,0x1bff7f06,0x8a7c8516,0xab85393b,0x28ef0506, // 0b20 __.OP__.____X_________|_;9_____(
0x85550b0b,0x3921337c,0xab856f82,0x05220006,0x0b0c0121,0x01213358,0x06000616,0x690b0b28, // 0b40 __U_|3!9_o___."_!___X3!___._(__i
0x0f162d15,0x21332e86,0x00061601,0x0b0b2807,0x2d2d157e,0x2e860f16,0x05760006,0x0616012e, // 0b60 _-___.3!___._(__~_--___._.v_.___
0x0b280c00,0x2d15950b,0x2e860f16,0x056a0006,0x158b2133,0x00061601,0x0b0b280b,0x162d15ac, // 0b80 ._(____-___._.j_3!_____._(____-_
0x062e860f,0x33053f00,0x01158b21,0x0a000616,0xc30b0b28,0x0f162d15,0x00062e86,0x2133056e, // 0ba0 __._.?_3!_____._(____-___._.n_3!
0x1601158b,0x28090006,0x15e60b0b,0x1711162d,0x860f2d16,0x2a00062e,0x0f2d2105,0x062e8616, // 0bc0 _____._(____-____-__._.*_!-___._
0x8b056a00,0x7c850115,0x160f4983,0x16110c0b,0x2d160787,0x01000611,0x0b0c0b28,0x12181411, // 0be0 .j_____|_I_________-__._(_______
0x0a0c0b28,0x15ab8539,0x1516012d,0x0b0c2233,0x062e86ea,0x15053f00,0x210b012d,0x45455303, // 0c00 (___9___-___3"____._.?__-__!_SEE
0x158bf289,0x06161116,0x1e280100,0x8b340c0b,0x240c0c25,0x7c857a82,0x053b0006,0x851c0c01, // 0c20 ________._(___4_%__$_z_|_.;_____
0x41454841,0x0c5b8944,0x0006d183,0x011b8900,0x41853f0c,0x4e494147,0x890c5b89,0x520c011b, // 0c40 AHEAD_[____..____?_AGAIN_[_____R
0x47454285,0xd1834e49,0x85600c01,0x49544e55,0x0b5b894c,0x826b0c01,0x5b894649,0x06d1830b, // 0c60 _BEGIN____`_UNTIL_[___k_IF_[____
0x1b890000,0x84770c01,0x4e454854,0x0d17d183,0x84870c01,0x45534c45,0x8c17458c,0x930c018c, // 0c80 ..____w_THEN________ELSE_E______
0x49485785,0x7a8c454c,0xa00c0117,0x45485784,0x187a8c4e,0x86ac0c01,0x45504552,0x588c5441, // 0ca0 _WHILE_z_____WHEN_z_____REPEAT_X
0x0c018c8c,0x4f4683b7,0x145b8952,0x0c01d183,0x464183c5,0x458c1554,0x0117d183,0x4e84d10c, // 0cc0 ______FOR_[_______AFT__E_______N
0x89545845,0x1b890a5b,0x03de0c01,0x06222c24,0xf6862200,0x3b832139,0xeb0c010d,0x06222482, // 0ce0 EXT_[_______$,"_."__9!_;_____$"_
0xd183e685,0x01ef8c0d,0x2e82fc0c,0xef850622,0x8c0dd183,0x0a0d01ef,0x444f4304,0x89e18645, // 0d00 ___________."____________CODE___
0x0f5483d3,0x010d4983,0x4306180d,0x54414552,0x891d8d45,0x5b89075b,0x2a0d0101,0x454f4405, // 0d20 __T__I_____CREATE___[__[___*_DOE
0x83123e53,0x0f5483d1,0x14160787,0x00062e22,0x12101309,0x5b89102d,0x891b890c,0x0d01015b, // 0d40 S>____T_____"._.____-__[____[___
0x4156083c,0x42414952,0x318d454c,0x89000006,0x610d011b,0x4e4f4308,0x4e415453,0x061d8d54, // 0d60 <_VARIABLE_1_..____a_CONSTANT___
0x2a890600,0x0006d183,0x1b892104,0x890f5b89,0x1b89015b,0x09740d01,0x52415632,0x4c424149, // 0d80 .__*___._!___[__[_____t_2VARIABL
0x06318d45,0x89160000,0x011b891b,0x3209970d,0x534e4f43,0x544e4154,0x00061d8d,0x832a8906, // 0da0 E_1_.._________2CONSTANT___.__*_
0x040006d1,0x821b8921,0x015b895e,0x891b8917,0xae0d011b,0x06282e82,0xc4862900,0x0d01ab85, // 0dc0 __._!___^_[__________.(_.)______
0x065c81d4,0xf6860a00,0xe10d0115,0x00062881,0x82c48629,0xec0d017a,0x4d4f430c,0x454c4950, // 0de0 ____.________(_.)___z____COMPILE
0x4c4e4f2d,0x0f548359,0x00061116,0x10171c40,0x09f80d01,0x454d4d49,0x54414944,0x0f548345, // 0e00 -ONLY_T____.@_______IMMEDIATE_T_
0x00061116,0x10171c80,0x05130e01,0x434f4c43,0x0e014c4b,0x4950072b,0x444f4d4e,0x0e014d45, // 0e20 ___.________CLOCKL__+_PINMODEM__
0x414d0335,0x0e014e50,0x4e490241,0x490e014f,0x54554f03,0x500e0150,0x4e494103,0x580e0151, // 0e40 5_MAPN__A_INO__I_OUTP__P_AINQ__X
0x4d575003,0x600e0152,0x494d5405,0x01535253,0x5005680e,0x52534943,0x720e0154,0x4d495405, // 0e60 _PWMR__`_TMISRS__h_PCISRT__r_TIM
0x01555245,0x50057c0e,0x544e4943,0x860e0156,0x4c454405,0x4c425941,0x82a18345,0x82a1834f, // 0e80 ERU__|_PCINTV____DELAYBLE___O___
0x2c464c5e,0x0e0b1517,0x900e019d,0x4c4f4304,0xac0e0644,0x060d4983,0x5483ac0e,0x8308060d, // 0ea0 ^LF,_________COLD____I_____T____
0x060d6083,0x6d830809,0x89cc850d,0x00000008,0x00000000,0x00000000,0x00000000,0x00000000, // 0ec0 _`_____m_____...................
0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000, // 0ee0 ................................
};
