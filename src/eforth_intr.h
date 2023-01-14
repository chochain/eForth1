/**
 * @file
 * @brief eForth1 interrupt handler class
 */
#ifndef __EFORTH1_INTR_H
#define __EFORTH1_INTR_H
#include "efore_core.h"

namespace EF1Intr {
	extern U8   t_idx;             ///< timer ISR index
	extern U16  p_xt[3];           ///< pin change ISR
	extern U16  t_xt[8];           ///< timer ISR

    void reset();                  ///< reset interrupts
    U16  hits();                   ///< fetch interrupt service routines

    void add_timer(U16 prd, U16 xt);
    void add_pci(U16 pin, U16 xt);
    void enable_timer(U16 f);
    void enable_pci(U16 f);
};    // namespace EF1Intr

#endif //__EFORTH1_INTR_H
