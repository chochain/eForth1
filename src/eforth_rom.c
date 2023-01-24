///
/// @file eforth_rom.c
/// @brief eForth ROM (loaded in Arduino Flash Memory)
/// @attention 8K max ROM before changing FORTH_ROM_SZ in eforth_core.h 
///
#include "eforth_core.h"
const U32 forth_rom_sz PROGMEM = 0x1078;
const U32 forth_rom[] PROGMEM = {
0x00105307,0x27040000,0x04424954,0x00062000,0x53414204,0x20020445,0x43020010,0x20040450, // 0000 _S_..._'TIB_. _._BASE__ _._CP__ 
0x4307001a,0x45544e4f,0x06045458,0x04002220,0x5453414c,0x2f200804,0x4d270500,0x0445444f, // 0020 _._CONTEXT__ "._LAST__ /._'MODE_
0x0039200a,0x42412706,0x0454524f,0x0044200c,0x444c4803,0x50200e04,0x50530400,0x10044e41, // 0040 _ 9._'ABORT__ D._HLD__ P._SPAN__
0x03005920,0x044e493e,0x00632012,0x49542304,0x20140442,0x7403006c,0x1604706d,0x02007620, // 0060  Y._>IN__ c._#TIB__ l._tmp__ v._
0x20044c42,0x04007f00,0x4c4c4543,0x87000204,0x4f4e0300,0x00910050,0x45594203,0x03009801, // 0080 BL_ ._._CELL__._._NOP._._BYE__._
0x0258523f,0x5403009f,0xa6032158,0x4f440500,0x044e4f43,0x440500ad,0x54494c4f,0x0500b605, // 00a0 ?RX__._TX!__._DOCON__._DOLIT__._
0x41564f44,0x00bf0652,0x4c4f4406,0x07545349,0x450500c8,0x5245544e,0x0400d207,0x54495845, // 00c0 DOVAR__._DOLIST__._ENTER__._EXIT
0x0700db08,0x43455845,0x09455455,0x440600e3,0x58454e4f,0x00ee0a54,0x52425107,0x48434e41, // 00e0 __._EXECUTE__._DONEXT__._QBRANCH
0x0600f80b,0x4e415242,0x030c4843,0x0d210101,0x2b02010d,0x01120e21,0x180f4001,0x21430201, // 0100 __._BRANCH____!____+!____@____C!
0x02011d10,0x23114043,0x3e520201,0x02012912,0x2f134052,0x523e0201,0x04013514,0x504f5244, // 0120 ____C@_#__R>_)__R@_/__>R_5__DROP
0x03013b15,0x16505544,0x53040143,0x17504157,0x4f04014a,0x18524556,0x52030152,0x5a19544f, // 0140 _;__DUP_C__SWAP_J__OVER_R__ROT_Z
0x49500401,0x611a4b43,0x4e410301,0x01691b44,0x1c524f02,0x58030170,0x761d524f,0x4e490601, // 0160 __PICK_a__AND_i__OR_p__XOR_v__IN
0x54524556,0x06017d1e,0x4948534c,0x871f5446,0x53520601,0x54464948,0x01019120,0x019b212b, // 0180 VERT_}__LSHIFT____RSHIFT ___+!__
0xa0222d01,0x232a0101,0x2f0101a5,0x0301aa24,0x25444f4d,0x4e0601af,0x54414745,0x01b62645, // 01a0 _-"___*#___/$___MOD%___NEGATE&__
0xc0273e01,0x283d0101,0x3c0101c5,0x0201ca29,0xcf2a3e30,0x3d300201,0x0201d52b,0xdb2c3c30, // 01c0 _>'___=(___<)___0>*___0=+___0<,_
0x2b310201,0x0201e12d,0xe72e2d31,0x443f0401,0xed2f5055,0x45440501,0x30485450,0x550201f5, // 01e0 __1+-___1-.___?DUP/___DEPTH0___U
0x01fe313c,0x2f4d5506,0x32444f4d,0x55030204,0x0e332a4d,0x2a4d0202,0x07021534,0x47454e44, // 0200 <1___UM/MOD2___UM*3___M*4___DNEG
0x35455441,0x4402021b,0x0226362b,0x372d4402,0x5202022c,0x02324350,0x43525403,0x04023844, // 0220 ATE5___D+6&__D-7,__RPC2__TRCD8__
0x45524548,0x1a001d07,0x3f00e001,0x41500302,0x02440744,0x002000bc,0x00e0019d,0x4305024d, // 0240 HERE__.___.?__PAD_D__. .___.M__C
0x2b4c4c45,0x9d008c07,0x5e00e001,0x45430502,0x072d4c4c,0x01a2008c,0x026d00e0,0x4c454305, // 0260 ELL+__.___.^__CELL-__.___.m__CEL
0x8c07534c,0xe001a700,0x04027c00,0x50554432,0x57015707,0x8b00e001,0x44320502,0x07504f52, // 0280 LS__.___.|__2DUP_W_W__.___2DROP_
0x01400140,0x029900e0,0x533e4403,0x5701de07,0x7a01de01,0xbb010001,0xe001bd02,0x0302a800, // 02a0 @_@__.___D>S___W___z_.______.___
0x07443e53,0x01de0147,0x02d40100,0xffff00bc,0x02d8010a,0x000000bc,0x02bf00e0,0x4d2f2a05, // 02c0 S>D_G___.____._______..._.___*/M
0x3807444f,0x2c021801,0xe0020b01,0x0402dc00,0x444f4d2f,0xac029007,0xb3013801,0xe0012c01, // 02e0 OD_8___,____.___/MOD_____8___,__
0x0202ef00,0xe2072f2a,0x40014f02,0x0300e001,0x21320203,0x38014707,0x9d008c01,0x2c010f01, // 0300 .___*/___O_@__.___2!_G_8__.____,
0xe0010f01,0x02031100,0x47074032,0x1a013801,0x8c012c01,0x1a019d00,0x2700e001,0x49570603, // 0320 ____.___2@_G_8___,__._____.'__WI
0x4e494854,0xa2015707,0xa2013801,0x01012c01,0x3d00e002,0x4f430503,0x07544e55,0x01e40147, // 0340 THIN_W___8___,____.=__COUNT_G___
0x0126014f,0x035500e0,0x53424103,0xde014707,0x77010001,0xe001bd03,0x03036800,0x0758414d, // 0360 O_&__.U__ABS_G___._w____.h__MAX_
0x01cc0290,0x038a0100,0x0140014f,0x037b00e0,0x4e494d03,0xc2029007,0x9f010001,0x40014f03, // 0380 ____.___O_@__.{__MIN_____.___O_@
0x9000e001,0x4d430503,0x0745564f,0x010a0138,0x015703c2,0x01570126,0x01380120,0x012c01e4, // 03a0 __.___CMOVE_8_____W_&_W_ _8___,_
0x00f501e4,0x029f03b2,0x03a500e0,0x564f4d04,0x008c0745,0x013801ac,0x03ec010a,0x011a0157, // 03c0 ___._____.___MOVE__.__8_____W___
0x010f0157,0x02640138,0x0264012c,0x03dc00f5,0x00e0029f,0x460403cc,0x074c4c49,0x0138014f, // 03e0 W___8_d_,_d__._____.___FILL_O_8_
0x010a014f,0x0290040c,0x01e40120,0x040600f5,0x00e0029f,0x480303f6,0xbc075845,0x15001000, // 0400 O_______ ____._____.___HEX__._._
0xe0010f00,0x07041600,0x49434544,0x074c414d,0x000a00bc,0x010f0015,0x042700e0,0x47494405, // 0420 .___.___DECIMAL__._._.___.'__DIG
0xbc075449,0x57000900,0xbc01cc01,0x6d000700,0xbc019d01,0x9d003000,0x3c00e001,0x58450704, // 0440 IT__._.W____._.m____.0.___.<__EX
0x43415254,0x00bc0754,0x014f0000,0x014f020b,0x00e00442,0x3c02045d,0x02510723,0x010f0054, // 0460 TRACT__...O___O_B__.]__<#_Q_T.__
0x047600e0,0x4c4f4804,0x00540744,0x01ea011a,0x00540147,0x0120010f,0x048400e0,0x15072301, // 0480 _.v__HOLD_T.____G_T.__ __.___#__
0x65011a00,0xe0048904,0x02049c00,0x9e075323,0x00014704,0x0a04bb01,0xe004af01,0x0404ab00, // 04a0 .__e____.___#S___G_.________.___
0x4e474953,0x0001de07,0xbc04d101,0x89002d00,0xbf00e004,0x3e230204,0x54014007,0x51011a00, // 04c0 SIGN___.____.-.___.___#>_@_T.__Q
0xa2015702,0xd500e001,0x553e0604,0x52455050,0xbc014707,0xbc006100,0x44007b00,0x07010003, // 04e0 _W____.___>UPPER_G__.a._.{.D_.__
0x5f00bc05,0xe0016d00,0x0604e900,0x49474944,0x38073f54,0xbc04f001,0xa2003000,0x0900bc01, // 0500 __._.m__.___DIGIT?_8____.0.___._
0xcc015700,0x39010001,0x0700bc05,0x4701a200,0x0a00bc01,0x7301cc00,0x2c014701,0xe0020101, // 0520 .W___._9__._.__G__._.__s_G_,____
0x07050b00,0x424d554e,0x073f5245,0x011a0015,0x00bc0138,0x01570000,0x0157035b,0x00bc0126, // 0540 .___NUMBER?__.__8__...W_[_W_&__.
0x01c70024,0x05720100,0x014f041a,0x014f01e4,0x015701ea,0x00bc0126,0x01c7002d,0x014f0138, // 0560 $.__._r___O___O___W_&__.-.__8_O_
0x01a20132,0x0132014f,0x01f2019d,0x05d40100,0x013801ea,0x01380147,0x00150126,0x0512011a, // 0580 2___O_2_____._____8_G_8_&__.____
0x05c60100,0x0015014f,0x01a7011a,0x012c019d,0x00f501e4,0x01400594,0x01000132,0x01bd05c0, // 05a0 .___O__.______,____.__@_2_._____
0x010a014f,0x012c05d2,0x029f012c,0x00bc029f,0x01470000,0x029f012c,0x0015012c,0x00e0010f, // 05c0 O_____,_,______...G_,___,__.___.
0x54030543,0x0b074249,0xe0011a00,0x0405e200,0x59454b3f,0xe000a307,0x0305ef00,0x0759454b, // 05e0 C__TIB__.___.___?KEY__._.___KEY_
0x010005f4,0x00e00600,0x450405fb,0x0754494d,0x00e000aa,0x5e02060a,0x01380748,0x012c0157, // 0600 __._.__.___EMIT__._.___^H_8_W_,_
0x0157014f,0x0100017a,0x00bc063c,0x060f0008,0x008201ea,0x00bc060f,0x060f0008,0x061600e0, // 0620 O_W_z_._<__._._____.___._.___.__
0x41505305,0x82074543,0xe0060f00,0x05064000,0x52414843,0x014f0753,0x000000bc,0x0138037f, // 0640 _SPACE__.___.@__CHARS_O__...__8_
0x0668010a,0x060f0147,0x066400f5,0x00e00140,0x3e05064f,0x52414843,0x7f00bc07,0x47016d00, // 0660 __h_G____.d_@__.O__>CHAR__._.m_G
0x7f00bc01,0x44008200,0x93010003,0xbc014006,0xe0005f00,0x06067200,0x43415053,0x82075345, // 0680 __._._.D_.___@__._._.r__SPACES__
0xe0065500,0x04069700,0x45505954,0x0a013807,0x5b06d101,0x7f00bc03,0x47016d00,0x7f00bc01, // 06a0 .U__.___TYPE_8_____[__._.m_G__._
0x44008200,0xcf010003,0xbc014006,0x0f005f00,0xb300f506,0xe0014006,0x0206a700,0xbc075243, // 06c0 ._.D_.___@__._.___.__@__.___CR__
0x0f000a00,0xdb00e006,0x6f640306,0x012c0724,0x012c0132,0x019d035b,0x014f0138,0x00e00138, // 06e0 ._.___.___do$_,_2_,_[___8_O_8__.
0x240306e9,0xed077c22,0x0200e006,0x222e0307,0x06ed077c,0x06ac035b,0x070d00e0,0x07522e02, // 0700 ___$"|____.___."|___[____.___.R_
0x01470138,0x036c0138,0x04ae0479,0x04c4012c,0x012c04d8,0x01a20157,0x06ac069e,0x071c00e0, // 0720 8_G_8_l_y___,_____,_W________.__
0x522e5503,0x79013807,0xd804ae04,0x57012c04,0x9e01a201,0xe006ac06,0x02074000,0x79072e55, // 0740 _U.R_8_y_____,_W________.@__U._y
0xd804ae04,0xac064604,0x5b00e006,0x072e0107,0x011a0015,0x000a00bc,0x0100017a,0x075e0782, // 0760 _____F____.[__.__.___._.z_.___^_
0x014700e0,0x036c0138,0x04ae0479,0x04c4012c,0x064604d8,0x00e006ac,0x3f01076d,0x6f011a07, // 0780 _.G_8_l_y___,_____F____.m__?___o
0x9a00e007,0x70280707,0x65737261,0x007a0729,0x01570120,0x01470138,0x08360100,0x007a01ea, // 07a0 __.___(parse)_z. _W_8_G_._6___z.
0x00820126,0x010001c7,0x013807f0,0x01570082,0x01a20126,0x018401de,0x07ee0100,0x00f501e4, // 07c0 &__.__.___8__.W_&_______.______.
0x012c07cc,0x00bc0140,0x01470000,0x012c00e0,0x014f0157,0x007a0138,0x01570126,0x01a20126, // 07e0 __,_@__...G__.,_W_O_8_z.&_W_&___
0x0126007a,0x01c70082,0x080e0100,0x010001de,0x01e40820,0x07f600f5,0x01380147,0x082a010a, // 0800 z.&__.__._____._ ____.__G_8___*_
0x0140012c,0x01e40147,0x01570138,0x012c01a2,0x01a2012c,0x015700e0,0x01a2012c,0x07a500e0, // 0820 ,_@_G___8_W___,_,____.W_,____.__
0x43415005,0x4707244b,0x90013801,0xe4012002,0xab014f01,0xe0012c03,0x05084000,0x53524150, // 0840 _PACK$_G_8___ ___O___,__.@__PARS
0x01380745,0x006705e6,0x019d011a,0x011a0071,0x011a0067,0x012c01a2,0x006707ad,0x00e00115, // 0860 E_8___g.____q.__g.____,___g.___.
0x5405085b,0x4e454b4f,0x61008207,0x1f00bc08,0x44039400,0x46026402,0x8200e008,0x4f570408, // 0880 [__TOKEN__.a__._.__D_d_F__.___WO
0x61074452,0x64024408,0xe0084602,0x05089d00,0x454d414e,0x035b073e,0x001f00bc,0x019d016d, // 08a0 RD_a_D_d_F__.___NAME>_[__._.m___
0x08af00e0,0x4d415305,0x38073f45,0x0a029001,0x47090301,0xf0012601,0xe4013804,0x47014f01, // 08c0 _.___SAME?_8_______G_&___8___O_G
0xf0012601,0xe4013804,0x2c014f01,0xa2012c01,0x0001f201,0x2c090301,0x38014001,0x2c029f01, // 08e0 _&___8___O_,_,_____.___,_@_8___,
0xf500e001,0x9f08d300,0x0000bc02,0xc400e000,0x49460408,0x4f07444e,0x26014701,0x0f007a01, // 0900 __._._____..._.___FIND_O_G_&_z._
0x1a014701,0x64013801,0x1a014f02,0x00014701,0x47096f01,0xbc011a01,0x6dff3f00,0x5f00bc01, // 0920 _G___8_d_O___G_._o_G____.?_m__._
0x32016d5f,0x5f00bc01,0x7a016d5f,0x5b010001,0xbc026409,0x0affff00,0x64096b01,0x1a007a02, // 0940 _m_2__.__m_z_._[_d__.____k_d_z._
0x4701ea01,0x6b010001,0x0a08ca09,0x2c097b01,0x4f014001,0x4f027301,0x0000e001,0x73098701, // 0960 ___G_._k_____{_,_@_O_s_O__..___s
0x0a027302,0x2c092b01,0x4f014001,0x73014001,0xb5014702,0xe0014f08,0x05091100,0x454d414e, // 0980 _s___+_,_@_O_@_s_G___O__.___NAME
0x002a073f,0x00e00916,0x5403099b,0x47075041,0x57060f01,0xe4012001,0xaa00e001,0x546b0409, // 09a0 ?_*.___.___TAP_G___W_ ____.___kT
0x47075041,0x0d00bc01,0x57017a00,0x0a00bc01,0x6d017a00,0xef010001,0x0800bc09,0x00017a00, // 09c0 AP_G__._.z_W__._.z_m_.____._.z_.
0x8209eb01,0x0a09ae00,0x1909ed01,0x4000e006,0x40014f01,0xe0014701,0x0609bd00,0x45434341, // 09e0 ____._________.@_O_@_G__.___ACCE
0x57075450,0x57019d01,0x7a029001,0x2f010001,0x4705ff0a,0xa2008201,0x5f00bc01,0x00020100, // 0a00 PT_W___W___z_._/___G__.___._.__.
0xae0a2901,0x2b010a09,0x0a09c20a,0x400a0901,0xa2015701,0xfb00e001,0x58450609,0x54434550, // 0a20 _)_____+_______@_W____.___EXPECT
0x5e0a0207,0x40010f00,0x3900e001,0x5551050a,0x07595245,0x00bc05e6,0x0a020080,0x010f0071, // 0a40 ___^.__@__.9__QUERY____._.__q.__
0x00bc0140,0x00670000,0x00e0010f,0x41050a4d,0x54524f42,0x1a004b07,0x0001f201,0xeb0a8101, // 0a60 @__...g.___.M__ABORT_K.____.____
0x6e00e000,0x6261060a,0x2274726f,0x99010007,0x5b06ed0a,0x7406ac03,0x4006ed0a,0x8500e001, // 0a80 ._.n__abort"_._____[___t___@__._
0x5245050a,0x07524f52,0x035b0646,0x00bc06ac,0x060f003f,0x0a7406de,0x240a0aa1,0x45544e49, // 0aa0 __ERROR_F_[____.?.____t____$INTE
0x45525052,0x09a10754,0x010001f2,0x01260aea,0x004000bc,0x0a8c016d,0x6f63200d,0x6c69706d, // 0ac0 RPRET_____.___&__.@.m____ compil
0x6e6f2065,0x00eb796c,0x054b00e0,0x0af60100,0x010a00e0,0x0aa70af8,0x5b810aba,0xc500bc07, // 0ae0 e only_._.K_.____._________[__._
0x0f003f0a,0xfa00e001,0x5645040a,0x88074c41,0x26014708,0x29010001,0x1a003f0b,0x0001f201, // 0b00 _?.___.___EVAL___G_&_._)_?.____.
0xeb0b2501,0x0f010a00,0xde01400b,0xc500bc06,0x1a003f0a,0x0001c701,0xfb0b5701,0x0400bc01, // 0b20 _%__.____@____.__?.____._W____._
0x38039400,0x66013201,0xf5076f01,0x110b4500,0x6f200507,0xe0203e6b,0x040b0900,0x54495551, // 0b40 .__8_2_f_o__.E____ ok> _.___QUIT
0x0000bc07,0x0f000b25,0x530afc01,0x0a0b0e0a,0x5b0b6b01,0x072c010b,0x01470244,0x001d0264, // 0b60 __..%_.____S_____k_[__,_D_G_d__.
0x010f010f,0x0b7500e0,0x072c4302,0x01470244,0x001d01e4,0x0120010f,0x0b8800e0,0x4e553f07, // 0b80 _____.u__C,_D_G____.__ __.___?UN
0x45555149,0xa1014707,0x0001f209,0x5b0bc401,0x1f00bc03,0x46016d00,0x1106ac06,0x72200607, // 0ba0 IQUE_G_____.___[__._.m_F______ r
0x66654465,0x00e00140,0x24030b9c,0x47076e2c,0x00011a01,0xa40bf301,0xb501470b,0x0f001d08, // 0bc0 eDef@__.___$,n_G___._____G____._
0x34014701,0x73010f00,0x1a002a02,0x0f014f01,0xa700e001,0x010bca0a,0x08880727,0x010009a1, // 0be0 _G_4.__s_*.__O____._____'_____._
0x00e00c04,0x0bf70aa7,0x54494c87,0x4c415245,0xbc00bc07,0x770b7700,0x0800e00b,0x4f43070c, // 0c00 ___._____LITERAL__._.w_w__.___CO
0x4c49504d,0x012c0745,0x011a0147,0x02640b77,0x00e00138,0x24080c1d,0x504d4f43,0x07454c49, // 0c20 MPILE_,_G___w_d_8__.___$COMPILE_
0x01f209a1,0x0c5e0100,0x00bc011a,0x016d0080,0x0c5a0100,0x010a00eb,0x0b770c5c,0x054b00e0, // 0c40 ____._^____._.m_._Z__.____w__.K_
0x0c680100,0x00e00c10,0x0c360aa7,0x4d4f430c,0x454c4950,0x4c4e4f2d,0x00bc0759,0x00340040, // 0c60 ._h____.__6__COMPILE-ONLY__.@.4.
0x0115011a,0x0c6c00e0,0x4d4d4909,0x41494445,0xbc074554,0x34008000,0x15011a00,0x8800e001, // 0c80 _____.l__IMMEDIATE__._.4._____._
0x4c41050c,0x07544f4c,0x0115001d,0x0ca100e0,0x4f435b89,0x4c49504d,0xf9075d45,0xe00b770b, // 0ca0 __ALLOT__.___.___[COMPILE]___w__
0x010cb000,0x00bc075d,0x003f0c3f,0x00e0010f,0x3a010cc3,0xce088807,0xbc0cc50b,0x8b000700, // 0cc0 .___]__.?_?.___.___:________._._
0xd200e00b,0x073b810c,0x00e000bc,0x0afc0b77,0x011a0034,0x010f002a,0x0ce500e0,0x524f4606, // 0ce0 __.___;__._.w___4.__*.___.___FOR
0x07544547,0x09a10888,0x010001f2,0x02730d26,0x001d0147,0x011a010f,0x002a0147,0x0034010f, // 0d00 GET_______._&_s_G__.____G_*.__4.
0x0140010f,0x0aa700e0,0x2e820cfc,0x00bc0728,0x08610029,0x00e006ac,0x5c810d2a,0x0a00bc07, // 0d20 __@__._____.(__.).a____.*_____._
0x4008a200,0x3a00e001,0x0728810d,0x002900bc,0x029f0861,0x0d4900e0,0x444f4304,0x08880745, // 0d40 .__@__.:__(__.).a____.I__CODE___
0x00340bce,0x002a011a,0x00e0010f,0x43060d58,0x54414552,0x0d5d0745,0x000600bc,0x00e00b8b, // 0d60 __4.__*.___.X__CREATE_]__._.___.
0x56080d6e,0x41495241,0x07454c42,0x00bc0d75,0x0b770000,0x0d8200e0,0x4e4f4308,0x4e415453, // 0d80 n__VARIABLE_u__...w__.___CONSTAN
0x0d5d0754,0x000400bc,0x0b770b8b,0x0d9800e0,0x45484185,0x25074441,0x44010a0c,0x0000bc02, // 0da0 T_]__._.__w__.___AHEAD_%___D__..
0xe00b7700,0x850db000,0x49414741,0x0c25074e,0x0b77010a,0x0dc700e0,0x47454285,0x44074e49, // 0dc0 .w__.___AGAIN_%___w__.___BEGIN_D
0xd800e002,0x4e55850d,0x074c4954,0x01000c25,0x00e00b77,0x49820de5,0x0c250746,0x02440100, // 0de0 __.___UNTIL_%_._w__.___IF_%_._D_
0x000000bc,0x00e00b77,0x54840df6,0x074e4548,0x014f0244,0x00e0010f,0x45840e0a,0x0745534c, // 0e00 _...w__.___THEN_D_O____.___ELSE_
0x014f0db6,0x00e00e0f,0x57850e1a,0x454c4948,0x4f0df907,0x2a00e001,0x4857840e,0xf9074e45, // 0e20 __O____.___WHILE___O__.*__WHEN__
0xe001570d,0x860e3900,0x45504552,0xcd075441,0xe00e0f0d,0x830e4700,0x07524f46,0x01380c25, // 0e40 _W__.9__REPEAT______.G__FOR_%_8_
0x00e00244,0x41830e57,0x40075446,0x440db601,0xe0014f02,0x840e6600,0x5458454e,0xf50c2507, // 0e60 D__.W__AFT_@___D_O__.f__NEXT_%__
0xe00b7700,0x030e7700,0x07222c24,0x002200bc,0x035b08a2,0x001d019d,0x00e0010f,0x41860e87, // 0e80 .w__.w__$,"__.".__[____.___.___A
0x54524f42,0x00bc0722,0x02440a8c,0x0e8b010f,0x0e9e00e0,0x07222482,0x070600bc,0x010f0244, // 0ea0 BORT"__.__D______.___$"__.__D___
0x00e00e8b,0x2e820eb4,0x00bc0722,0x02440711,0x0e8b010f,0x0ec600e0,0x414e3e05,0xea07454d, // 0ec0 ___.___."__.__D______.___>NAME__
0x26014701,0x7f00bc01,0xbc016d00,0xcc002000,0xdf010001,0xd800e00e,0x5544040e,0x1507504d, // 0ee0 _G_&__._.m__. .__.____.___DUMP__
0x38011a00,0xbc041a01,0x9d001f00,0x1000bc01,0x3801ac00,0x51010a01,0xbc06de0f,0x90000800, // 0f00 .__8____._.___._.__8___Q____._._
0xbc015702,0x44000500,0xbc013807,0x0f003a00,0x41010a06,0x1a01470f,0x0500bc01,0x64074400, // 0f20 _W__._.D_8__.:.____A_G____._.D_d
0x3500f502,0x4601380f,0x82064606,0x2c06ac02,0x1900f501,0x2c01400f,0x0f001501,0xf900e001, // 0f40 __.5_8_F_F_____,__.__@_,__.___._
0x4f57050e,0x07534452,0x002a06de,0x000000bc,0x010f007a,0x01f2011a,0x0fb60100,0x035b0147, // 0f60 __WORDS___*._...z.______.___G_[_
0x001f00bc,0x0147016d,0x000200bc,0x007a019d,0x06ac0115,0x06460646,0x007a0273,0x00bc011a, // 0f80 _._.m_G__._.__z.____F_F_s_z.___.
0x01c20040,0x0fb20100,0x00bc06de,0x007a0000,0x010a010f,0x00e00f74,0x43050f61,0x4b434f4c, // 0fa0 @.__.______...z.____t__.a__CLOCK
0x050fba38,0x414c4544,0x02c30759,0x02290fc0,0x0314007a,0x032a007a,0x022f0fc0,0x014f01de, // 0fc0 8___DELAY_____)_z.__z.*___/___O_
0x01000140,0x00e00fd4,0x50070fc3,0x4f4d4e49,0xea394544,0x414d030f,0x0ff53a50,0x3b4e4902, // 0fe0 @_.____.___PINMODE9___MAP:___IN;
0x4f030ffc,0x023c5455,0x49410310,0x10093d4e,0x4d575003,0x0310103e,0x3f524d54,0x50031017, // 1000 ___OUT<___AIN=___PWM>___TMR?___P
0x1e404943,0x4d540410,0x25414552,0x43500410,0x2d424549,0x52540510,0x44454341,0x53041035, // 1020 CI@___TMREA%__PCIEB-__TRACED5__S
0x45455641,0x4c04103e,0x4644414f,0x43041046,0x07444c4f,0x104e00bc,0x010f002a,0x104e00bc, // 1040 AVEE>__LOADFF__COLD__.N_*.___.N_
0x010f0034,0x0ac500bc,0x010f003f,0x0b6000bc,0x010f004b,0x0b6006de,0x00000000,0x00000000, // 1060 4.___.__?.___.`_K.____`_........

};

