#
# To build ROM only
# 1) make rom
#
CXX  = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Isrc

EXE  = tests/eforth1
ASM  = src/eforth_asm.o
OBJS = \
	src/eforth_core.o \
	src/eforth_vm.o   \
	src/eforth_rom.o  \
	src/eforth1.o

%.o: %.cpp %.c
	$(CXX) $(CXXFLAGS) -Isrc -c -o $@ $<

all: rom $(EXE)

rom:
	$(CXX) $(CXXFLAGS) -DASM_ONLY=1 -c -o $(ASM) src/eforth_asm.cpp
	$(CXX) -o tests/ef_$@ $(ASM)
	echo Geneating eForth ROM...
	tests/ef_$@ > src/eforth_rom.c
	rm $(ASM)

$(EXE): $(OBJS)
	$(CXX) $(CXXFLAGS) -c -o $(ASM) src/eforth_asm.cpp
	$(CXX) -o $@ $(ASM) $^

clean:
	rm $(EXE) $(OBJS) $(ASM)


