#!/bin/bash -e

if [ ! "${srcdir+x}" ]; then
    srcdir=.
fi

export DATA_DIR=${srcdir}/test/data

# test image
if [ ! -r $DATA_DIR/image_ext2.dd ]; then
    echo cannot read $DATA_DIR/image_ext2.dd
    exit 77
fi

TD=${srcdir}/test/tools/tool_differ.sh

# Test usage message
TMP_EXPECTED=$(mktemp)
TMP_ACTUAL=$(mktemp)
sed -E 's|^(usage: )[^ ]+|\1<ILS_PATH>|' ${srcdir}/test/tools/fstools/ils_output/1 > $TMP_EXPECTED
tools/fstools/ils$EXEEXT 2>&1 | sed -E 's|^(usage: )[^ ]+|\1<ILS_PATH>|' > $TMP_ACTUAL
$TD "cat $TMP_ACTUAL" $TMP_EXPECTED
rm -f $TMP_EXPECTED $TMP_ACTUAL

# Test inode listing
TMPFILE=$(mktemp)
tools/fstools/ils -e -i raw -f ext2 $DATA_DIR/image_ext2.dd | sed '2s/|.*||[0-9]*$/|unknown||0/' | sed 's/[[:space:]]*$//' | sed -e '$a\' > $TMPFILE
$TD "cat $TMPFILE" ${srcdir}/test/tools/fstools/ils_output/2
rm -f $TMPFILE
