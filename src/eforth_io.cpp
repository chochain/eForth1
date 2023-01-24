///
/// @file
/// @brief - eForth1 EEPROM save/load module
///
///
///> eForth1 SAVE/LOAD functions
///
#include "eforth_core.h"

#define GET(d)    (*(DU*)&data[d])
#define SET(d, v) (*(DU*)&data[d] = (v))

#if ARDUINO
#include <EEPROM.h>
#else // !ARDUINO
class MockPROM                                ///< mock EEPROM access class
{
    U8 _prom[FORTH_UVAR_SZ + FORTH_DIC_SZ];   ///< mock EEPROM storage
public:
    U8   read(U16 idx)         { return _prom[idx]; }
    void update(U16 idx, U8 v) { _prom[idx] = v; }
};
MockPROM EEPROM;                              ///> fake Arduino EEPROM unit
#endif // ARDUINO

int ef_save(U8 *data)
{
	U16 here = GET(FORTH_UVAR_ADDR + sizeof(DU)*2);
	int sz   = here - FORTH_RAM_ADDR;
    for (int i=0; i < sz; i++) {
    	EEPROM.update(i, data[i]);   /// * store dictionary byte-by-byte
    }
    return sz;
}
int ef_load(U8 *data)
{
	IU  hidx = FORTH_UVAR_ADDR + sizeof(DU) * 9;  /// * >IN (buffer pointer)
	DU  vIN  = GET(hidx);                         /// * keep vIN, vNTIB
	DU  vNTIB= GET(hidx + sizeof(DU));

	U16 pidx = sizeof(DU) * 2;        ///< CP addr in EEPROM (aka HERE)
    U16 vCP  = ((U16)EEPROM.read(pidx+1)<<8) + EEPROM.read(pidx);
    int  sz  = vCP - FORTH_RAM_ADDR;
    if (!vCP || sz > FORTH_DIC_SZ) return 0;

    for (int i=0; i < sz; i++) {
        data[i] = EEPROM.read(i);     /// * retrieve dictionary byte-by-byte
    }
    SET(hidx, vIN);					  /// * restore vIN, vNTIB
    SET(hidx + sizeof(DU), vNTIB);

    return sz;
}
