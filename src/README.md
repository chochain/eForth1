### eForth1 Source Code

    ~/eforth1.ino - example eForth1 wrapper
    ~/src         - Arduino eForth1 library package
        + eForth1.h         - interface for Arduino .ino
        + eforth_config.h   - universal types and sizing
        + eforth_asm.h      - C-based macro assembler
        + eforth_asm.c      - Forth meta-compiler to build eforth_rom.c
        + eforth_rom.c      - eForth1 built-in words from eforth_asm.c
        + eforth_core.h     - definition for interrupt handlers and system interface
        + eforth_core.cpp   - interrupt handler, system function implementation
        + eforth_vm.h       - definition for eForth1 Virtual Machine
        + eforth_vm.cpp     - eForth1 VM implementation
        + eforth1.cpp       - eForth1 library main entry point
        
