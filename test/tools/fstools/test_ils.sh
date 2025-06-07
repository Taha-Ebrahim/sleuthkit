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
$TD 'tools/fstools/ils$EXEEXT' ${srcdir}/test/tools/fstools/ils_output/1

# Test inode listing
TMPFILE=$(mktemp)
tools/fstools/ils -e -i raw -f ext2 $DATA_DIR/image_ext2.dd | sed '2s/|.*||[0-9]*$/|unknown||0/' | sed 's/[[:space:]]*$//' | sed -e '$a\' > $TMPFILE
$TD "cat $TMPFILE" ${srcdir}/test/tools/fstools/ils_output/2
rm -f $TMPFILE
