ARGS = --static -O3 -march=tigerlake -fcf-protection=none

all: naive_copy.out simd_shift.out

windows: naive_copy.exe simd_shift.exe

# linux
naive_copy.out: naive_copy.cpp
	g++ $(ARGS) naive_copy.cpp -o naive_copy.out

simd_shift.out: simd_shift.cpp
	g++ $(ARGS) simd_shift.cpp -o simd_shift.out

# windows
naive_copy.exe: naive_copy.cpp
	x86_64-w64-mingw32-g++ $(ARGS) naive_copy.cpp -o naive_copy.exe

simd_shift.exe: simd_shift.cpp
	x86_64-w64-mingw32-g++ $(ARGS) simd_shift.cpp -o simd_shift.exe

clean:
	rm *.out *.exe
