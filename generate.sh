#!/bin/bash

# Generates the header file, adding the appropriate boilerplate.
# See `macro_sequence_for.input.h` for the details on what is being generated.

# The max level of loop nesting. (The number of iterations is always unlimited.)
N=8

INPUT=macro_sequence_for.input.h
OUTPUT=macro_sequence_for.h

# The base part, aka the -1th nesting level.
# `-e '$d'` deletes the last line, which is the `#endif` of the include guard.
sed -e '$d' -e 's/xx//g' -e 's/yy/0/g' -e 's/SF_MAX_NESTING 0/SF_MAX_NESTING '"$N"'/' "$INPUT" | grep -vE '^//\?\?' >"$OUTPUT"

# Boilerplate for nesting levels `0..N-1`.
for ((i=0; $i<$N; i=$(($i+1))))
do
    echo "// $i" >>"$OUTPUT"
    grep '#define \w*xx\w*' "$INPUT" | sed -e 's/xx/'"$i"'/g' -e 's/yy/'"$((i+1))"'/g' >>"$OUTPUT"
done

# Lastly, close the include guard.
echo $'\n#endif' >>"$OUTPUT"
