# Test Results
Tests performed on a 11th Gen Intel(R) Core(TM) i7-11390H @ 3.40GHz.

```sh
# Testing Command
hyperfine --shell=none --warmup=10 -r 1000 --input=input.csv --output=./a ./file.out
```

## Naive Copy
```
Time (mean ± σ):       4.5 ms ±   0.2 ms    [User: 1.6 ms, System: 2.8 ms]
Range (min … max):     4.2 ms …   6.4 ms    1000 runs
```

## SIMD Shift
```
Time (mean ± σ):       1.6 ms ±   0.2 ms    [User: 0.6 ms, System: 0.9 ms]
Range (min … max):     1.4 ms …   2.4 ms    1000 runs
```

# Possible Improvements
- Is it better to serialise all the blocks after like before?
- What is the impact removing the size specialisations?
- Does a better compression algorithem reduce time overall?
- Currently Perf doesnt really work well becuase its too small. Is there a better way to do this while keeing the fixed size?
