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
				if [[ $sort = 2 ]]; then
						echo $type > /tmp/csv_of_aligned_workdir/r_"$i"
						echo $line | tr "," "\n" | tr -d ' ' >> /tmp/csv_of_aligned_workdir/r_"$i"
				fi
				sort=$[sort+1]
		done
		i=$[i+1]
done < /dev/stdin
paste /tmp/csv_of_aligned_workdir/r_* -d',' > /tmp/csvright.csv
paste /tmp/csv_of_aligned_workdir/l_* -d',' > /tmp/csvleft.csv
rm /tmp/csv_of_aligned_workdir/r_*
rm /tmp/csv_of_aligned_workdir/l_*
rmdir /tmp/csv_of_aligned_workdir
