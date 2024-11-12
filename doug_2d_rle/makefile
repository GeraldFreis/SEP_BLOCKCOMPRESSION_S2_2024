ARGS = --static -O3 -march=tigerlake -fcf-protection=none -std=c++20 -g

all: fast_2d.out fast_2d_single.out fast_2d_msingle.out fast_2d_mutex.out

input.csv:
	cd ../helper; cargo run --release -- gen 256 256 32000 32 32 16 --tag-len 12 --tags-no 11 -o ../doug_2d_rle/input.csv

# linux
fast_2d_msingle.out: fast_2d_single.cpp ../Input/*.cpp MultiSerialiser.hpp
	g++ $(ARGS) -D MULTISERIALISER fast_2d_single.cpp ../Input/*.cpp -o $@

%.out: %.cpp ../Input/*.cpp Serialiser.hpp
	g++ $(ARGS) $< ../Input/*.cpp -o $@

# windows
fast_2d_msingle.exe: fast_2d_single.cpp ../Input/*.cpp MultiSerialiser.hpp
	x86_64-w64-mingw32-g++ $(ARGS) -D MULTISERIALISER fast_2d_single.cpp ../Input/*.cpp -o a.exe

%.exe: %.cpp ../Input/*.cpp Serialiser.hpp
	x86_64-w64-mingw32-g++ $(ARGS) $< ../Input/*.cpp -o a.exe

clean:
	rm *.out *.exe
