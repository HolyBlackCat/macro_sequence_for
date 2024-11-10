## macro_sequence_for.h

A 'for each' loop macro, with unlimited number of iterations (not limited by the amount of generated boilerplate macros), and with the ability to transfer state between iterations.

* [The motivating example](#the-motivating-example)
* [Common function arguments](#common-function-arguments)
* [Recursion](#recursion)
* [How?](#how)
  * [Unlimited number of iterations?](#unlimited-number-of-iterations)
  * [Ability to transfer state between iterations?](#ability-to-transfer-state-between-iterations)

### The motivating example
```cpp
#include "macro_sequence_for.h"

#define MAKE_FLAGS(name, seq) enum name {SF_FOR_EACH(BODY, STEP, FINAL, 0, seq)};
#define BODY(n, d, x) x = 1 << (d),
#define STEP(n, d, x) d+1
#define FINAL(n, d) _mask = (1 << (d)) - 1

MAKE_FLAGS(E, (a)(b)(c))
```
This expands to:
```cpp
enum E
{
    a = 1 << (0),                // 0b0001
    b = 1 << (0+1),              // 0b0010
    c = 1 << (0+1+1),            // 0b0100
    _mask = (1 << (0+1+1+1)) - 1 // 0b0111
};
```
Here, `BODY()` is executed for every element of `(a)(b)(c)` to generate the respective enum constant. Each `BODY` invocation receives one element of the input sequence (`x`) and the "state" (`d`, which acts as a counter in this case).

The initial value of the state is taken from the fourth argument of `SF_FOR_EACH()`, and then the state is updated by calling `STEP()` on every iteration (in this case, it increments the counter).

Lastly, `FINAL()` is called after the loop ends, receiving the final state (here, the value of the counter after the final increment).

In all three macros, `n` receives the current loop nesting level (`0` in the case, since the loop is flat).

### Common function arguments

We provide a few simple functions as predefined arguments for `SF_FOR_EACH`:

* `SF_NULL`, which expands to nothing, and
* `SF_STATE`, which expands to its second argument.

Example: Computing a sum.
```cpp
#define SUM(seq) SF_FOR_EACH(SF_NULL, SUM_STEP, SF_STATE, 0, seq)
#define SUM_STEP(n, d, x) d+x
int x = SUM((1)(2)(3)); // 0+1+2+3
```

Example: Declaring multiple variables with the same value.
```cpp
#define DECL(value, seq) SF_FOR_EACH(DECL_BODY, SF_STATE, SF_NULL, value, seq)
#define DECL_BODY(n, d, x) int x = d;
DECL(42, (x)(y)(z)) // int x = 42; int y = 42; int z = 42;
```

### Recursion

We do support recursive loops, but the depth is limited by the amount of generated boilerplate macros. The max depth is reported by `SF_MAX_DEPTH`. Edit `generate.sh` and re-run it if you need to change this parameter.

Since the same macro can't expand recursively, each recursion level has to use a different variation of `SF_FOR_EACH`: suffixed either with nothing or with `0`...`SF_SF_MAX_DEPTH-1`.

Example:
```cpp
#define ARRAYS(seq) SF_FOR_EACH(ARRAYS_BODY, SF_NULL, SF_NULL,, seq)
#define ARRAYS_BODY(n, d, name, values) int name[] = {SF_FOR_EACH2(ARRAYS_BODY2, SF_NULL, SF_NULL,, values)};
#define ARRAYS_BODY2(n, d, x) x,

ARRAYS((f,(1)(2)(3))(g,(4)(5))) // int f[] = {1,2,3,}; int g[] = {4,5,};
```

You can automatically get the recursion level from the `n` argument, and call `SF_FOR_EACH(...)` as `SF_CAT(SF_FOR_EACH,n)(...)` to glue the number to the macro name.

### Some extras

`STEP()` can emit output in addition to `BODY()`, by returning `d, ...` (`...` gets emitted as text after the result of `BODY()`, while `d` gets assigned to the state variable). This can let you avoid duplicate work in some cases, and often looks better in general. The downside is that it can't emit unbalanced parentheses.

### How?

The basic idea is taken from the [PPMP Iceberg](https://jadlevesque.github.io/PPMP-Iceberg/explanations#codefxyyyyycode) article: to convert the sequence of the form `(a)(b)(c)` to `a) b) c)`. They call those "guides".

For some reason, I'm unable to find any existing implementation using this idea.

The article uses a wonky termination condition (each element has to start with a letter or digit, a specific element was reserved for ending the loop).

I've fixed this limitation by producing  `)a) )b) )c) 0)` instead of `a) b) c)`, where `)...` indicates the next sequence element, and `0)` indicates the end of the loop.


#### Unlimited number of iterations?

Most implementations of the macro loops (e.g. [`BOOST_PP_SEQ_FOR_EACH()`](https://www.boost.org/doc/libs/1_80_0/libs/preprocessor/doc/ref/seq_for_each.html)) have a limit on the number of iterations, determined by the amount of boilerplate macros they've generated.

Here's a minimal example of such a loop, supporting up to 4 iterations.
```cpp
#define LOOP(m, ...) __VA_OPT__(LOOP_0(m, __VA_ARGS__))
#define LOOP_0(m, x, ...) m(x) __VA_OPT__(LOOP_1(m, __VA_ARGS__))
#define LOOP_1(m, x, ...) m(x) __VA_OPT__(LOOP_2(m, __VA_ARGS__))
#define LOOP_2(m, x, ...) m(x) __VA_OPT__(LOOP_3(m, __VA_ARGS__))
#define LOOP_3(m, x, ...) m(x) __VA_OPT__(LOOP_4(m, __VA_ARGS__))

#define BODY(x) [x]
LOOP(BODY, 1, 2, 3)       // Ok:     [1][2][3]
LOOP(BODY, 1, 2, 3, 4)    // Ok:     [1][2][3][4]
LOOP(BODY, 1, 2, 3, 4, 5) // Breaks: [1][2][3][4] LOOP_4(BODY, 5)
```

In general, boilerplate-based loops are more flexible, since they're not limited to iterating over sequences, and can perform arbitrary computations (see [`BOOST_PP_FOR()`](https://www.boost.org/doc/libs/1_80_0/libs/preprocessor/doc/ref/for.html)). But they seem to be inferior for the 'for each' loops.

Our macro doesn't have a limit on the number of iterations, it's not constrained by the amount of boilerplate.

But we still have *some* boilerplate to support nested loops, and the nesting level *is* limited by the amount of boilerplate.

#### Ability to transfer state between iterations?

This refers to the ability to forward the parameter of `SF_FOR_EACH()` to the loop body (`BODY()` in the example above), and to modify it between iterations.

While this seems like an obvious feature to have, certain kinds of simple loops don't have it.

Consider the following boilerplate-less loop, which doesn't have a limit on the number of iterations:
```cpp
#define LOOP(seq) END(A seq)
#define BODY(x) int x;
#define A(x) BODY(x) B
#define B(x) BODY(x) A
#define A_END
#define B_END
#define END(...) END_(__VA_ARGS__)
#define END_(...) __VA_ARGS__##_END

LOOP((a)(b)(c)) // int a; int b; int c;
```

What's not to like here?

Firstly, we can't forward any state from outside to the loop body. It's impossible to implement `LOOP(42, (a)(b)(c))`->`int a=42; int b=42; int c=42;` with this approach.

Similarly, this doesn't let us preserve state between iterations. E.g. we can't produce `int a=1; int b=1+1; int c=1+1+1;`.

And lastly, we can't abstract away this loop into a single macro, since passing the `BODY()` into such macro would too constitute "forwarding state into the loop body".
