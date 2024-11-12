# Tests
Tests performed on a 11th Gen Intel(R) Core(TM) i7-11390H @ 3.40GHz. These results are still very flaky.

```sh
# Testing Command
hyperfine --shell=none --warmup=2 -r 10 --input=input.csv --output=./a ./simd_shift.out
```

# Times

## V0
Spend 12.98% of the time in main and 39.10% of the time in `Serialiser::single`. In main the `divq` instruction took 18% of the time, rest was spread even. In `Serialiser::single` ~30% of the time was taken copying the label. The `get_label` function is an indirect call but doesnt seem to have that great an impact. `Serialiser::single` also isnt inlined. maybe it would be better to use a cache for compressed blocks and then serialise in blocks for locality, maybe not.

```
Time (mean ± σ):      5.175 s ±  0.314 s    [User: 2.572 s, System: 2.501 s]
Range (min … max):    4.844 s …  5.652 s    10 runs

```

## V1
Divide is hard to remove for cases when you skip chunks. changing to compile in fixed chunk_x to allow gcc to optimise this out. Didnt help, im not quite sure why is significantly worse.

```
Time (mean ± σ):      5.437 s ±  0.341 s    [User: 2.757 s, System: 2.550 s]
Range (min … max):    5.039 s …  6.008 s    10 runs

```

## V2
Changing to using a lookup table instead of modulo.

```
Time (mean ± σ):      5.150 s ±  0.275 s    [User: 2.578 s, System: 2.508 s]
Range (min … max):    4.975 s …  5.734 s    10 runs

```

## V3
Changing to formatting blocks in chunks.

```
Time (mean ± σ):      5.085 s ±  0.048 s    [User: 2.881 s, System: 2.177 s]
Range (min … max):    5.015 s …  5.146 s    10 runs
```

# Possible Improvements
- Try multithreading, this may or may not be advantages, the input is still kinda small.
- Copying labels are slow
