%.exe: %.cpp ../Input/*.cpp
	x86_64-w64-mingw32-g++ --static $< ../Input/*.cpp -o $@

clean:
	rm *.exe
