//?? -- Codegeneration explained --
//?? This file is used as a source for generating the final header.
//?? While this file is self-sufficient, it doesn't have any boilerplate macros, preventing you from being able to nest loops.
//?? Any macros having `xx` in their names are duplicated for each nesting level, with `xx` being replaced with ``,`0`,`1`, etc.
//?? `yy` is replaced with the next nesting level.
//?? Lastly, any lines beginning with `//??` are removed.
//?? ----
#ifndef MACRO_SEQUENCE_FOR_H_ // Intentionally not a `#pragma once`, to be able to tolerate multiple copies of the file.
#define MACRO_SEQUENCE_FOR_H_

// "macro_sequence_for.h", v0.3
// Implements macro loops with unlimited number of iterations, over sequences of the form `(a)(b)(c)` (though the nesting level is limited).
// See `SF_FOR_EACH()` below for the usage explanation.

// BSD Zero Clause License
//
// Copyright (c) 2022 Egor Mikhailov
//
// Permission to use, copy, modify, and/or distribute this software for any
// purpose with or without fee is hereby granted.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
// REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
// AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
// INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
// LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
// OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
// PERFORMANCE OF THIS SOFTWARE.


// A loop over a sequence of the form `(a)(b)(c)`. The elements can contain commas.
// `m` is the loop body, invoked as `m(n, d, ...)`, where `...` is one of the elements,
// `d` initially matches the argument `d` of `SF_FOR_EACH`, but is replaced with
//   the result of `s(n, d, ...)` after every iteration.
// After the loop finishes, `f(n, d)` is called with the final value of `d`.

// Additionally, if `s` returns something with a `,`, everything after the first comma
//   gets pasted to the output after `m(...)` on the same iteration. This is sometimes
//   useful in complex macros, where you'd otherwise repeat the same computation
//   in both `m` and `s`.
// But note that while `m` can return absolutely anything, `s` can't return mismatched parentheses.
//
// The `n` argument receives the next available loop nesting level. `SF_FOR_EACH`
//   sets it to `0`, while `SF_FOR_EACHi` sets it to `i+1`.
// Use `SF_FOR_EACHi(...)` for nested loops, where `i` can either be hardcoded,
//   or come from the `n` argument of the outer loop, in which case you
//   can use `SF_CAT(SF_FOR_EACH, n)(...)`.
#define SF_FOR_EACHxx(m, s, f, d, seq) IMPL_SEQFOR_FORxx(m, s, f, d, IMPL_SEQFOR_TO_GUIDE_A IMPL_SEQFOR_ANNOTATE_SEQ_END(IMPL_SEQFOR_ANNOTATE_SEQ_A seq)) )

// Various useful macros, to be passed as arguments to `SF_FOR_EACH`.
#define SF_NULL(...) // A no-op.
#define SF_STATE(...) IMPL_SEQFOR_STATE(__VA_ARGS__,) // Returns the second argument. As the step and/or final function, returns the state unchanged.

// Concatenate two identifiers.
#define SF_CAT(x, y) SF_CAT_(x, y)
#define SF_CAT_(x, y) x##y

// The max loop nesting level. This is baked in at the code generation time, adjust it in `generate.sh`.
// The actual number of supporting nesting levels is `N+1`, with name suffixes ``, `0`, `1`, ..., `N-1`.
// The most nested loop will report `N` as the next available level.
#define SF_MAX_DEPTH 0


// Implementation:

#if defined(_MSC_VER) && (!defined(_MSVC_TRADITIONAL) || _MSVC_TRADITIONAL == 1)
#error The standard-conformant MSVC preprocessor is required, enable it with `/Zc:preprocessor`.
#endif

// Same as `SF_FOR_EACH`, but the list is spelled differently, as described in `IMPL_SEQFOR_FOR_GUIDExx_A`.
#define IMPL_SEQFOR_FORxx(m, s, f, d, guide) IMPL_SEQFOR_IDENTITYxx( IMPL_SEQFOR_FOR_GUIDExx_A IMPL_SEQFOR_LPAREN m, s, f, d, guide )

// Various helpers.
#define IMPL_SEQFOR_NULL(...)
#define IMPL_SEQFOR_IDENTITYxx(...) __VA_ARGS__
#define IMPL_SEQFOR_LPAREN (
#define IMPL_SEQFOR_CAT(x, y) IMPL_SEQFOR_CAT_(x,y)
#define IMPL_SEQFOR_CAT_(x, y) x##y
// Implementation of `SF_STATE`.
#define IMPL_SEQFOR_STATE(n, d, ...) d

// `IMPL_SEQFOR_ANNOTATE_SEQ_END(IMPL_SEQFOR_ANNOTATE_SEQ_A seq)` converts `seq` from e.g. `(a)(b)(c)` to `()(a)()(b)()(c)(0)`.
#define IMPL_SEQFOR_ANNOTATE_SEQ_A(...) ()(__VA_ARGS__)IMPL_SEQFOR_ANNOTATE_SEQ_B
#define IMPL_SEQFOR_ANNOTATE_SEQ_B(...) ()(__VA_ARGS__)IMPL_SEQFOR_ANNOTATE_SEQ_A
#define IMPL_SEQFOR_ANNOTATE_SEQ_A_END
#define IMPL_SEQFOR_ANNOTATE_SEQ_B_END
#define IMPL_SEQFOR_ANNOTATE_SEQ_END(x) IMPL_SEQFOR_ANNOTATE_SEQ_END_(x)(0)
#define IMPL_SEQFOR_ANNOTATE_SEQ_END_(x) x##_END

// `IMPL_SEQFOR_TO_GUIDE_A seq` converts `seq` from e.g. `(a)(b)(c)` to `a)b)c)IMPL_SEQFOR_TO_GUIDE_A` (or ...`_B`).
// We call the resulting sequence `a)b)c)` a "guide".
#define IMPL_SEQFOR_TO_GUIDE_A(...) __VA_ARGS__)IMPL_SEQFOR_TO_GUIDE_B
#define IMPL_SEQFOR_TO_GUIDE_B(...) __VA_ARGS__)IMPL_SEQFOR_TO_GUIDE_A

// Usage: `IMPL_SEQFOR_FOR_GUIDExx_A(m,s,d,`, followed by a sequence of 0+ blocks of the form `)x)`, followed by `0)`, finally followed by arbitrary junk.
// For each `)x)` in the sequence, expands to `m(r,d,x)`, then modifies `d` to contain `s(r,d,x)`, where `r` is the next available loop nesting level.
#define IMPL_SEQFOR_FOR_GUIDExx_A(m, s, f, d, e) IMPL_SEQFOR_CAT(IMPL_SEQFOR_FOR_GUIDExx_A_, e)(m, s, f, d)
#define IMPL_SEQFOR_FOR_GUIDExx_A_(m, s, f, d) IMPL_SEQFOR_FOR_GUIDExx_B(m, s, f, d,
#define IMPL_SEQFOR_FOR_GUIDExx_A_0(m, s, f, d) f(yy, d) IMPL_SEQFOR_NULL(
#define IMPL_SEQFOR_FOR_GUIDExx_B(m, s, f, d, ...) m(yy, d, __VA_ARGS__) IMPL_SEQFOR_FOR_GUIDExx_B_0(m, s, f, s(yy, d, __VA_ARGS__))
#define IMPL_SEQFOR_FOR_GUIDExx_B_0(...) IMPL_SEQFOR_FOR_GUIDExx_B_1(__VA_ARGS__)
#define IMPL_SEQFOR_FOR_GUIDExx_B_1(m, s, f, d, ...) __VA_ARGS__ IMPL_SEQFOR_FOR_GUIDExx_A(m, s, f, d,


// Generated boilerplate for nested loops:
#endif
