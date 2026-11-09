CC = gcc
CXX = g++
CFLAGS = -O2 -Wall
CXXFLAGS = -O2 -Wall

TARGETS = SERDA.exe THP.exe FVAM.exe VAM.exe SAM.exe IHW.exe RBSM.exe CSM.exe benchmark.exe

all: $(TARGETS)

SERDA.exe: src/SERDA.c
	$(CC) $(CFLAGS) src/SERDA.c -o SERDA.exe

THP.exe: src/THP.c
	$(CC) $(CFLAGS) src/THP.c -o THP.exe

FVAM.exe: src/FVAM.c
	$(CC) $(CFLAGS) src/FVAM.c -o FVAM.exe

VAM.exe: src/VAM.c
	$(CC) $(CFLAGS) src/VAM.c -o VAM.exe

SAM.exe: src/SAM.c
	$(CC) $(CFLAGS) src/SAM.c -o SAM.exe

IHW.exe: src/IHW.c
	$(CC) $(CFLAGS) src/IHW.c -o IHW.exe

RBSM.exe: src/RBSM.cpp
	$(CXX) $(CXXFLAGS) src/RBSM.cpp -o RBSM.exe

CSM.exe: src/CSM.cpp
	$(CXX) $(CXXFLAGS) src/CSM.cpp -o CSM.exe

benchmark.exe: src/benchmark.cpp
	$(CXX) $(CXXFLAGS) src/benchmark.cpp -o benchmark.exe

benchmark: all
	benchmark.exe

clean:
ifeq ($(OS),Windows_NT)
	-del /Q /F $(TARGETS) *.o 2>nul || exit 0
else
	rm -f $(TARGETS) *.o
endif
