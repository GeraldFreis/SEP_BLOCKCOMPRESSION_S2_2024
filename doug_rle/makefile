ARGS = --static -O3 -march=tigerlake -fcf-protection=none

all: simd_shift_v0.out simd_shift_v1.out simd_shift_v2.out simd_shift_v3.out

input.csv:
	cd ../helper \
	cargo run --release -- gen 700 500 3000 14 10 12 --tag-len 14 --tags-no 8 -o ../doug_rle/input.csv

# linux
%.out: %.cpp ../Input/*.cpp Serialiser.h
	g++ $(ARGS) $< ../Input/*.cpp -o $@

# windows
%.exe: %.cpp ../Input/*.cpp Serialiser.h
	x86_64-w64-mingw32-g++ $(ARGS) $< ../Input/*.cpp -o a.exe

clean:
	rm *.out *.exe
