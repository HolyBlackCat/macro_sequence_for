// Compile-time tests.
// If this file compiles (no linking), the tests passed.

#ifndef MACRO_SEQUENCE_FOR_H_
#include "macro_sequence_for.h"
#endif

#define STR(...) STR_(__VA_ARGS__)
#define STR_(...) #__VA_ARGS__

// Compares two strings for equality, ignoring whitespace.
constexpr bool same(const char *a, const char *b)
{
    while (true)
    {
        while (*a == ' ' || *a == '\n') a++;
        while (*b == ' ' || *b == '\n') b++;
        if (*a != *b)
            return false;
        if (!*a)
            return true;
        a++;
        b++;
    }
}

#define BODY(n_, d_, ...) (body:n=n_;x=__VA_ARGS__;d=d_)
#define STEP(n_, d_, ...) (step:n=n_;x=__VA_ARGS__;d=d_)
#define FINAL(n_, d_) (final:n=n_;d=d_)

// Number of iterations:
// 0
static_assert(same(STR(SF_FOR_EACH(BODY, STEP, FINAL, s0,)), "(final:n=0;d=s0)"), "Test: 0 iterations.");
// 1
static_assert(same(STR(SF_FOR_EACH(BODY, STEP, FINAL, s0, (1))), "(body:n=0;x=1;d=s0) (final:n=0;d=(step:n=0;x=1;d=s0))"), "Test: 1 iteration.");
// 3
static_assert(same(STR(SF_FOR_EACH(BODY, STEP, FINAL, s0, (1)(2)(3))), R"(
    (body:n=0;x=1;d=s0)
    (body:n=0;x=2;d=(step:n=0;x=1;d=s0))
    (body:n=0;x=3;d=(step:n=0;x=2;d=(step:n=0;x=1;d=s0)))
    (final:n=0;   d=(step:n=0;x=3;d=(step:n=0;x=2;d=(step:n=0;x=1;d=s0))))
)"), "Test: 3 iteration.");

// Unusual elements: empty and with commas.
static_assert(same(STR(SF_FOR_EACH(BODY, STEP, FINAL, s0, ()(0)(1,2))), R"(
    (body:n=0;x=   ;d=s0)
    (body:n=0;x=0  ;d=(step:n=0;x=   ;d=s0))
    (body:n=0;x=1,2;d=(step:n=0;x=0  ;d=(step:n=0;x= ;d=s0)))
    (final:n=0;     d=(step:n=0;x=1,2;d=(step:n=0;x=0;d=(step:n=0;x= ;d=s0))))
)"), "Test: 3 iteration.");

// Operating with a non-zero nesting level:
static_assert(same(STR(SF_FOR_EACH2(BODY, STEP, FINAL, s0, (1)(2)(3))), R"(
    (body:n=3;x=1;d=s0)
    (body:n=3;x=2;d=(step:n=3;x=1;d=s0))
    (body:n=3;x=3;d=(step:n=3;x=2;d=(step:n=3;x=1;d=s0)))
    (final:n=3;   d=(step:n=3;x=3;d=(step:n=3;x=2;d=(step:n=3;x=1;d=s0))))
)"), "Test: Non-zero nesting level argument.");

// True nested loops.
#define NESTED_BODY(n_, d_, x_, ...)  (n=n_,d=d_,x=x_,SF_CAT(SF_FOR_EACH,n_)(SF_CAT(NESTED_BODY,n_), SF_NULL, SF_NULL, d_, __VA_ARGS__))
#define NESTED_BODY0(n_, d_, x_, ...) (n=n_,d=d_,x=x_,SF_CAT(SF_FOR_EACH,n_)(SF_CAT(NESTED_BODY,n_), SF_NULL, SF_NULL, d_, __VA_ARGS__))
#define NESTED_BODY1(n_, d_, x_, ...) (n=n_,d=d_,x=x_,SF_CAT(SF_FOR_EACH,n_)(SF_CAT(NESTED_BODY,n_), SF_NULL, SF_NULL, d_, __VA_ARGS__))

static_assert(same(STR(SF_FOR_EACH(NESTED_BODY, SF_NULL, SF_NULL,, (0,)(1,(10,))(2,(20,)(21,(210,)(211,))))), R"(
    (n=0,d=,x=0,)
    (n=0,d=,x=1,
        (n=1,d=,x=10,)
    )
    (n=0,d=,x=2,
        (n=1,d=,x=20,)
        (n=1,d=,x=21,
            (n=2,d=,x=210,)
            (n=2,d=,x=211,)
        )
    )
)"), "Test: True nested loops.");

// Predefined macro arguments:
// SF_PRESERVE_STATE
static_assert(same(STR(SF_FOR_EACH(BODY, SF_PRESERVE_STATE, SF_NULL, s0, (1)(2)(3))), "(body:n=0;x=1;d=s0) (body:n=0;x=2;d=s0) (body:n=0;x=3;d=s0)"), "Test: SF_PRESERVE_STATE");
// SF_DUMP_STATE
static_assert(same(STR(SF_FOR_EACH(SF_NULL, SF_PRESERVE_STATE, SF_DUMP_STATE, s0, (1)(2)(3))), "s0"), "Test: SF_DUMP_STATE");
