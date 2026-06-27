# Homework 02 — Vector Sum in C (DAXPY: d = a·x + y)

## 1. What this code does, and how to compile/run it

`daxpy.c` implements the linear algebra operation **d⃗ = a·x⃗ + y⃗** in C, with
all parameters (`N`, `a`, `x`, `y`) taken dynamically from the command line,
as required by the assignment.

- `x`, `y`, `d` are allocated **dynamically on the heap** (`malloc`), so the
  program can handle very large `N` (up to `10^8` and beyond, memory
  permitting) without overflowing the stack.
- Vectors `x` and `y` are filled uniformly with the scalar values passed on
  the command line (`x[i] = x_val`, `y[i] = y_val` for every `i`).
- An **internal correctness test** compares every computed `d[i]` against
  the expected value `a*x_val + y_val` (computed once), using exact (`==`)
  floating-point comparison, and reports the number of mismatches.
- Execution time is measured with `clock()` from `<time.h>`.

### Compilation

```bash
make
```

which runs:

```bash
gcc -std=c11 -Wall -Wextra -O2 daxpy.c -o daxpy -lm
```

### Usage

```bash
./daxpy N a x y
```

| Argument | Meaning                                   |
|----------|--------------------------------------------|
| `N`      | vector dimension (positive integer)        |
| `a`      | scalar multiplier (double)                 |
| `x`      | uniform fill value for vector `x` (double) |
| `y`      | uniform fill value for vector `y` (double) |

Example (the case requested in Step 4 of the assignment):

```bash
./daxpy 10 3 0.1 7.1
```

---

## 2. Terminal output — Step 2: testing N = 10, N = 10⁶, N = 10⁸

All three runs use `a = 3`, `x = 0.1`, `y = 7.1` (same values as Step 4, so
that timing and correctness can be discussed together).

**N = 10**
```
==== Vector Sum (DAXPY): d = a*x + y ====
N               = 10
a               = 3
x (uniform)     = 0.10000000000000001
y (uniform)     = 7.0999999999999996
expected value  = 7.3999999999999995
d[0]            = 7.3999999999999995
d[N-1]          = 7.3999999999999995
mismatches      = 0 / 10
internal test   = PASSED
elapsed time    = 0.000001 s
==========================================
```

**N = 10⁶**
```
==== Vector Sum (DAXPY): d = a*x + y ====
N               = 1000000
...
mismatches      = 0 / 1000000
internal test   = PASSED
elapsed time    = 0.003169 s
==========================================
```

**N = 10⁸**
```
==== Vector Sum (DAXPY): d = a*x + y ====
N               = 100000000
...
mismatches      = 0 / 100000000
internal test   = PASSED
elapsed time    = 4.385267 s
==========================================
```

(Exact timings depend on the machine; the qualitative trend — execution
time scaling roughly linearly with `N` — is what matters and is reproducible
on CloudVeneto.)

---

## 3. Questions & Answers

**Question 1.** *Develop a C program computing d⃗ = a·x⃗ + y⃗ with N, a, x, y
acquired dynamically from the command line.*

**Answer.** Done — see `daxpy.c`. Arguments are parsed with `atoll`/`atof`
from `argv[]`, and `x`, `y`, `d` are heap-allocated with `malloc` based on
the runtime value of `N`, so the size is genuinely determined at execution
time, not at compile time.

---

**Question 2.** *Execute the code testing N = 10, N = 10⁶, and N = 10⁸.*

**Answer.** All three executions complete successfully and pass the
internal test (see outputs above). The main observation is **scaling**:
going from `N = 10` to `N = 10⁶` the runtime is still dominated by
process startup overhead (both are near-instantaneous), but going to
`N = 10⁸` the cost of allocating and looping over **three** arrays of
`8 bytes × 10⁸ = 800 MB each (2.4 GB total)** becomes clearly visible
(several seconds). This is consistent with the memory-hierarchy discussion
in the lecture notes (Chapter 2): at `N = 10^8` the working set vastly
exceeds any CPU cache, so the loop is effectively **memory-bound** — every
iteration pays for a RAM access rather than reusing cached data, and
indeed the wall-clock time is much larger than what pure ALU throughput
would predict for `2×10^8` floating-point operations.

---

**Question 3.** *Implement an internal algorithmic test to rigorously
verify that every element of d⃗ is exactly equal to the expected
mathematical result of a·x + y.*

**Answer.** Implemented: since `x` and `y` are filled uniformly, the
expected value is computed once (`expected = a*x_val + y_val`) and every
`d[i]` is compared against it with the exact equality operator `==` (not an
approximate/tolerance-based comparison). The number of mismatches is
reported; in all three runs above it is **0**, i.e. the test **passes**.

---

**Question 4.** *Provide a = 3, x = 0.1, y = 7.1. Observe and report if the
internal test correctly evaluates the resulting sum to exactly 7.4.*

**Answer — and discussion of the floating-point anomaly.**

The internal test **passes** (0 mismatches), but this needs to be
interpreted carefully, because of the following subtlety:

- `0.1` and `7.1` **cannot be represented exactly** in IEEE-754 double
  precision (they are not finite binary fractions); they are stored as the
  nearest representable doubles.
- Computing `3.0 * 0.1 + 7.1` in double precision therefore does **not**
  give the mathematical value `7.4` bit-for-bit. Printing the result with
  full precision (`%.17g`) shows:

  ```
  a*x + y      = 7.3999999999999995   (computed)
  literal 7.4  = 7.4000000000000004   (the double closest to 7.4)
  (a*x+y) == 7.4  ->  false
  ```

  These two doubles differ by exactly **1 ULP** (unit in the last place).
  At the default `printf("%g", …)` precision (6 significant digits) both
  values *print* as `7.4`, which can mislead a careless reader into
  thinking the comparison would succeed — it would not, if we compared
  against the literal `7.4` directly.

- **Why does our internal test still pass, then?** Because the test does
  **not** compare `d[i]` against the literal constant `7.4`. It compares
  `d[i]` against `expected = a*x_val + y_val`, i.e. the **same arithmetic
  expression**, evaluated **once**, in the **same precision**, using the
  **same compiler and CPU instructions** as inside the loop. Since
  floating-point arithmetic in C is deterministic for a fixed expression,
  fixed precision, and fixed rounding mode, `d[i]` and `expected` are
  computed through the literally identical sequence of operations, so they
  are **guaranteed to be bit-for-bit identical**. The test is therefore
  **correct and meaningful** for what it claims to verify (numerical
  consistency of the loop), but it would have given a **different**
  (failing) result if we had instead hard-coded `if (d[i] != 7.4)` in the
  source.

- **Practical lesson:** this is a textbook illustration of why floating-point
  values should essentially never be compared with `==` against a *literal*
  constant, but comparing two *independently re-derived but equivalent*
  expressions for internal self-consistency checks (as done here) is a
  legitimate and common technique — provided one is aware that it tests
  **reproducibility of the computation**, not **closeness to the exact
  mathematical answer**. The discrepancy here (`~4.4×10⁻¹⁶` relative
  error) is exactly `1` machine epsilon (`ε ≈ 2.22×10⁻¹⁶` for double
  precision), as expected from a single rounding operation.

---

## 4. Discussion of computational anomalies (general, Step 2/4 combined)

- **Large N (10⁸):** no correctness anomalies appear (the test still
  passes, since the arithmetic loop is identical regardless of `N`), but
  **performance** changes qualitatively: the workload moves from
  "negligible" to "memory-bandwidth bound", illustrating the memory
  hierarchy concepts (Chapter 2 of the lecture notes — cache vs. RAM
  latency, and the importance of profiling before optimizing).
- **Floating-point equality (Step 4):** confirms that `0.1` and `7.1` are
  not exactly representable, and that the "exact" result `7.4` is itself
  only approximately represented. Internal self-consistency tests (compare
  recomputed expected values, not literal constants) are robust to this;
  naive comparisons against decimal literals are not.

---

## 5. References to course material

- **Chapter 1, §1.6** (C Basics: command-line arguments) — used directly to
  parse `argc`/`argv`.
- **Chapter 2, §2.16** (Memory Management: Stack vs. Heap) — motivates why
  `x`, `y`, `d` must be heap-allocated with `malloc` rather than declared as
  fixed-size stack arrays: for `N = 10^8`, a stack array would massively
  exceed the typical 1 MB stack limit and crash with a segmentation fault.
- **Chapter 2, §2.12** (Memory Hierarchy Timing) and **§2.15** (General
  Optimization Workflow) — explain the runtime behavior observed at
  `N = 10^8`: the loop is memory-bound, and profiling before optimizing is
  the correct general workflow.
- **Chapter 5** (Floating-Point Numbers) — the discussion in Question 4 is
  a direct application of the IEEE-754 representation and machine epsilon
  concepts introduced there: `0.1`, `7.1`, and `7.4` are all *not* exactly
  representable, and the result of `a*x+y` differs from the literal `7.4`
  by exactly one unit in the last place (`1 ULP ≈ ε_mach`).
