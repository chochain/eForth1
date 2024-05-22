CXX      = g++
ASM_ONLY = 0
CXXFLAGS = -std=c++17 -O2 -Wall -DASM_ONLY=$(ASM_ONLY)

EXE  = tests/eforth1
ASM  = tests/eforth1_rom
OBJS = \
	src/eforth_asm.o \
	src/eforth_core.o \
	src/eforth_vm.o \
	src/eforth_rom.o \
	src/eforth1.o

exe: $(EXE)

rom: $(ASM)

%.o: %.cpp %.c
	$(CXX) $(CXXFLAGS) -Isrc -c -o $@ $<

$(ASM): src/eforth_asm.o
	$(CXX) -o $@ $^
	$@ > src/eforth_rom.c

$(EXE): $(OBJS) 
	$(CXX) -o $@ $^

clean:
	rm $(EXE) $(OBJS)


