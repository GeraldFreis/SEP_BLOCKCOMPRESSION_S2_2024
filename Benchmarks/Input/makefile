all: input label

input: testdata.csv 
	@echo "benchmark input"
	@cd ../../Test/Input; \
	make input_test.out
	time ../../Test/Input/input_test.out f < testdata.csv > out.csv
	time ../../Test/Input/input_test.out b < testdata.csv > out.csv

label: testdata.csv benchmark_getting_labels.out
	@echo "benchmark label"
	time ./benchmark_getting_labels.out f < testdata.csv
	time ./benchmark_getting_labels.out b < testdata.csv

benchmark_getting_labels.out: ../../Input/*.cpp benchmark_getting_labels.cpp
	g++ benchmark_getting_labels.cpp ../../Input/*.cpp -O2 -o benchmark_getting_labels.out

access_method: testdata.csv
	g++ benchmark_access_method.cpp -O2 -o benchmark_access_method.out
	./benchmark_access_method.out

testdata.csv:
	@echo "Generating test data"
	@cd ../../helper; \
	cargo run --release -- gen 1200 1200 1200 20 20 20 -t 100 -o ../Benchmarks/Input/testdata.csv

clean:
	rm *.out testdata.csv
