rm /tmp/csvleft.csv 2>/dev/null
rm /tmp/csvright.csv 2>/dev/null
mkdir -p /tmp/csv_of_aligned_workdir/
i=0
while read outer; do
		sort=0
		type=0
		echo $outer | tr '|' '\n' | while read line; do
				if [[ $sort = 0 ]]; then
						type=`echo $line | cut -d',' -f1`;
				fi
				if [[ $sort = 1 ]]; then
						echo $type > /tmp/csv_of_aligned_workdir/l_"$i"
						echo $line | tr "," "\n" | tr -d ' ' >> /tmp/csv_of_aligned_workdir/l_"$i"
				fi
				sort=$[sort+1]
		done
		echo /tmp/csv_of_aligned_workdir/l_"$i" >> /tmp/csv_of_aligned_workdir/lefts
		i=$[i+1]
done < /dev/stdin
cat /tmp/csv_of_aligned_workdir/lefts | xargs paste -d',' > /tmp/csvleft.csv
rm /tmp/csv_of_aligned_workdir/lefts
rm /tmp/csv_of_aligned_workdir/l_*
rmdir /tmp/csv_of_aligned_workdir
