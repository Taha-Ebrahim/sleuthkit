#!/bin/bash
set -e

IMAGE="test/data/image_ext2.dd"
OFFSET=0
OUTDIR="test/tools/input/known_good_dir"
FS_TYPE="ext2"

echo "⚙️ Building known_good_dir from $IMAGE"

rm -rf "$OUTDIR"
mkdir -p "$OUTDIR"

# Use full paths
fls -r -p -f $FS_TYPE -o $OFFSET "$IMAGE" > fls_ext2.txt

COUNT=0

while IFS= read -r line; do
  FILETYPE=$(echo "$line" | awk '{print $1}')
  INUM=$(echo "$line" | awk -F: '{print $1}' | awk '{print $NF}')
  NAME=$(echo "$line" | awk -F: '{gsub(/^[ \t]+|[ \t]+$/, "", $2); print $2}')


  if [[ -z "$INUM" || -z "$NAME" ]]; then
    echo "⚠️ Skipping malformed line: $line"
    continue
  fi

  OUTPATH="$OUTDIR/$NAME"
  mkdir -p "$(dirname "$OUTPATH")"

  case "$FILETYPE" in
    r/r)
      icat -f $FS_TYPE -o $OFFSET "$IMAGE" "$INUM" > "$OUTPATH" && echo "✅ extracted $NAME"
      ((COUNT++))
      ;;
    l/l)
      echo "<symlink placeholder>" > "$OUTPATH"
      echo "🔗 created placeholder for symlink $NAME"
      ((COUNT++))
      ;;
    *)
      echo "⏭️ skipping $FILETYPE $NAME"
      ;;
  esac
done < fls_ext2.txt

echo "✅ Done. Total files written: $COUNT"
