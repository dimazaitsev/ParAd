#!/usr/bin/bash

# Converts ParAd SPM file into Matrix Market MTX file
# Just adds to the head: 
# (1) a line with Matrix market label in comments
# (2) a line M N L
# Counts the number of SPM lines as L and the maximum
# in the first and second dimension as M and L correspondingly

# Usage:
#   ./SPMtoMTX.sh input.spm > output.mtx
#
# Input format (SPM):
#   i j value
#   ...
#
# Output format (MTX):
#   M N L
#   i j value
#   ...

echo "%%MatrixMarket matrix coordinate real symmetric"

awk '
BEGIN {
    max_i = 0
    max_j = 0
    count = 0
}

# Skip empty lines or comments
NF >= 3 && $1 !~ /^;/ {
    i = $1
    j = $2
    v = $3

    data[count] = i " " j " " v
    count++

    if (i > max_i) max_i = i
    if (j > max_j) max_j = j
}

END {
    # Print M N L
    print max_i, max_j, count

    # Print entries
    for (k = 0; k < count; k++) {
        print data[k]
    }
}
' "$1"

# ParAd additional scripts, 2026, DimaZaitsev.github.io
