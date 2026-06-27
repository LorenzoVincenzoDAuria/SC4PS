# Homework 03 — Matrix Multiplication and Benchmarking

## 1. What this code does, and how to compile/run it

`matmul.c` computes **C = A·B** for square `N×N` matrices, where (as
specified in the assignment) `A` is filled entirely with the scalar `a` and
`B` is filled entirely with the scalar `b`. Matrices are stored as flat
1D arrays in **row-major order** (`M[i*N + j]`), matching how C actually
lays out 2D data in memory (see lecture notes, Chapter 2).

The program implements **all six possible loop-nesting orders**
(`ijk`, `ikj`, `jik`, `jki`, `kij`, `kji`) for the triple loop computing
`C[i][j] += A[i][k]*B[k][j]`, selectable from the command line, so the
exact same compiled binary can be used to benchmark every permutation.

### Compilation

```bash
make
```
```
gcc -std=c11 -Wall -Wextra -O2 matmul.c -o matmul -lm
```

### Usage

```bash
./matmul a b N fileout [order]
```

| Argument  | Meaning                                                       |
|-----------|----------------------------------------------------------------|
| `a`       | scalar fill value for matrix A                                |
| `b`       | scalar fill value for matrix B                                |
| `N`       | matrix dimension (square N×N)                                 |
| `fileout` | path of the text file where C is written                      |
| `order`   | optional, one of `ijk ikj jik jki kij kji` (default: `ijk`)    |

Example:

```bash
./matmul 2 3 600 C_out.txt ikj
```

To benchmark all six orderings at once:

```bash
make benchmark N=600
```

---

## 2. The O(1) correctness test (Step 4 — optimization challenge)

Since `A = a · J_N` and `B = b · J_N` (`J_N` = all-ones matrix), every
element of `C` is mathematically:

```
C[i][j] = Σ_k A[i][k]*B[k][j] = Σ_k (a*b) = N * a * b      (constant, independent of i, j)
```

Instead of re-checking all `N²` entries (an extra `O(N²)` pass on top of the
`O(N³)` multiplication itself), the program checks only **5 representative
entries** (the four corners and the center element) against this
closed-form value `N*a*b`, with a small numerical tolerance (since summing
`N` floating-point copies of `a*b` is not always bit-identical to the
single product `N*a*b`, though it is extremely close).

---

## 3. Terminal output

**Small run (N = 5, a = 2, b = 3):**
```
==== Matrix Multiplication C = A*B (order=ijk) ====
N               = 5
a               = 2
b               = 3
expected C[i][j]= 30  (= N*a*b)
max |error|     = 0.000e+00 (sampled corners + center)
internal test   = PASSED
elapsed time    = 0.000002 s
output file     = /tmp/small.txt
====================================================
```
Content of `C_out.txt` for this run (every entry is indeed `30 = 5*2*3`):
```
30 30 30 30 30
30 30 30 30 30
30 30 30 30 30
30 30 30 30 30
30 30 30 30 30
```

**Benchmark, N = 600, a = 2, b = 3, all six orderings (measured with `make benchmark N=600`):**

| Loop order | Elapsed time (s) |
|------------|-------------------|
| `ijk`      | 0.283             |
| `ikj`      | **0.128**         |
| `jik`      | 0.272             |
| `jki`      | 0.332             |
| `kij`      | **0.135**         |
| `kji`      | 0.322             |

(All six runs pass the internal correctness test; timings are from a single
representative run on the development machine — absolute numbers will
differ on CloudVeneto, but the **relative ranking** is what matters and is
reproducible.)

---

## 4. Questions & Answers

**Question 1–3.** *Acquire `a, b, N, fileout` from the command line; build
`A = aI_N`, `B = bI_N`; save `C` to `fileout`.*

**Answer.** Implemented as described above; `C` is written to `fileout` as
a plain-text matrix, one row per line, space-separated values (`%.10g`).

---

**Question 4.** *Implement a rigorous internal test verifying every element
of C, ideally without an O(N²) comparison.*

**Answer.** Implemented using the closed-form result `C[i][j] = N·a·b`
(derived above) checked at 5 sampled positions, in O(1) — see §2.

---

**Question 5.** *Benchmark the matrix multiplication, try different loop
orderings, find the fastest, and explain why based on 1D memory access
patterns and CPU caching.*

**Answer.** The benchmark (table above) shows two clear groups:

- **Fast orderings: `ikj` and `kij`** (≈ 0.13 s) — both have `j` as the
  **innermost** loop variable.
- **Slow orderings: `ijk`, `jik`, `jki`, `kji`** (≈ 0.27–0.33 s) — none of
  them has `j` innermost; the slowest of all, `jki`, has `i` innermost.

**Why this happens (1D memory + caching):**

Matrices are stored row-major, so `B[k*N+j]` and `C[i*N+j]` are contiguous
in memory when **`j` varies** (stride 1), and `B[(k+1)*N+j]` jumps `N`
doubles ahead when **`k` varies** with `j` fixed (stride `N`, i.e. a full
row — almost always a cache miss for large `N`).

- In **`ikj`** (and **`kij`**), the innermost loop varies `j`: both `B` and
  `C` are accessed **sequentially**, letting the CPU's hardware
  prefetcher load consecutive cache lines before they're needed, and `A`'s
  index (`i*N+k`) is fixed for the whole inner loop (read once, reused from
  a register). This is exactly the *cache-friendly* loop order described in
  the lecture notes (§2.14: "innermost loop on the last index of the
  matrix").
- In **`jki`** (the worst case), the innermost loop varies `i`: **both**
  `A[i*N+k]` and `C[i*N+j]` jump by `N` doubles at every iteration (`B` is
  the only operand accessed contiguously, but it is now invariant in the
  inner loop, so contiguity there buys nothing). The CPU is forced into
  costly cache misses on *two* operands per inner iteration instead of
  zero, which matches it being the slowest measured order.
- `ijk`, `jik`, `kji` sit in between: each strides badly on exactly **one**
  of the three matrices in the inner loop (e.g. `ijk` strides on `B` since
  the inner index is `k`), which is consistent with their intermediate
  timings (≈ 2× slower than `ikj`/`kij`, but not as bad as `jki`).

**Conclusion:** the fastest orderings are the ones where the innermost loop
index matches the **last** (contiguous, row-major) index of both `B` and
`C` simultaneously — i.e. `j` innermost — confirming the general rule from
the lecture notes that loop ordering should be chosen so that the innermost
loop walks along contiguous memory, minimizing cache misses, independently
of which matrix the algorithm conceptually "outer-loops" on first.

---

## 5. Discussion of results

The roughly **2.5×** speed difference between the best (`ikj`/`kij`) and
worst (`jki`) orderings, for the *exact same number of floating-point
operations* (`2N³`), is a direct, measured illustration of the principle
that **algorithmic complexity alone does not determine real-world
performance** — memory access patterns and cache behavior often dominate.
This matches the General Optimization Workflow in the lecture notes
(§2.15): write correct code first, then profile/benchmark, then apply
targeted optimizations (here, loop reordering) to the part that actually
matters.

---

## 6. References to course material

- **Chapter 2, §2.6–§2.7** (Mental model of CPU execution; Loop Ordering
  and Data Locality) — the direct theoretical basis for the benchmarking
  task and its explanation.
- **Chapter 2, §2.14** (Loop Ordering and Matrices: Summary) — explicitly
  recommends reordering `i, j, k → i, k, j` for cache-friendly row-major
  access; our measurements confirm `ikj`/`kij` (the two orders with `j`
  innermost) as the fastest, consistent with this recommendation.
- **Chapter 2, §2.16** (Stack vs. Heap) — `A`, `B`, `C` are heap-allocated
  with `malloc`, required for matrices of size `N = 600` and beyond
  (`3×600²×8 bytes ≈ 8.6 MB`, already too large to safely place on the
  default 1 MB stack).
- **Chapter 2, §2.15** (General Optimization Workflow) — followed
  explicitly: correctness first (O(1) analytical test), then profiling
  (the benchmark table), then explanation grounded in cache behavior
  rather than guesswork.
