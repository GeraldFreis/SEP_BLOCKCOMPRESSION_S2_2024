test: input_test.out
	./input_test.out f < test_data_file_32768_4x4x4.csv > out.csv
	diff --strip-trailing-cr out.csv test_data_file_32768_4x4x4.csv
	./input_test.out b < test_data_file_32768_4x4x4.csv > out.csv
	diff --strip-trailing-cr out.csv test_data_file_32768_4x4x4.csv

input_test.out: input_test.cpp ../../Input/BasicInput.cpp
	g++ input_test.cpp ../../Input/*.cpp -O2 -o input_test.out

clean:
	rm input_test.out out.csv
