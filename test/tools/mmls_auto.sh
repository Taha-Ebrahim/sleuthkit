#!/bin/bash

# Make sure these are defined
: "${SLEUTHKIT_TEST_DATA_DIR:?Need to set SLEUTHKIT_TEST_DATA_DIR}"
: "${EXEEXT:=}"  # Default to empty string if not set

IMAGE="$SLEUTHKIT_TEST_DATA_DIR/from_brian/10-ntfs-autodetect/10-ntfs-disk.dd"
TOOL="tools/vstools/mmls$EXEEXT"
OUTDIR="test/tools/output"

mkdir -p "$OUTDIR"

i=2

# Flag combinations to test
flags=(
    "-B"
    "-c"
    "-a"
    "-A"
    "-m"
    "-M"
    "-B -c"
    "-r"
    "-a -M"
    "-A -M"
    "-a -A"
    "-a -A -M"
    "-t dos"
)

for flag in "${flags[@]}"; do
    outfile="$OUTDIR/mmls_${i}.stdout"
    cmd="$TOOL $flag $IMAGE"
    eval "$cmd" > "$outfile" 2>&1

    # Print cli_tests.txt line
    echo "mmls_$i|$cmd|$outfile|0"

    ((i++))
done