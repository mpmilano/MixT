#!/bin/bash
mkdir -p /tmp/transpose_workdir
i=0
while read outer; do
		i=$[i+1]
		echo $outer | tr ',' '\n' > /tmp/transpose_workdir/$i
		echo -n /tmp/transpose_workdir/"$i "
done < /dev/stdin | xargs paste -d','
rm /tmp/transpose_workdir/[0-9]*
rmdir /tmp/transpose_workdir/
