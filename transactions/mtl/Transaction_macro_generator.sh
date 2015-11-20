for ((j = 1; j < 50; j = j + 1));
do echo;
   echo -n '#define TRANS_SEQ'$[j + 1]'(';
   for ((i = 1; i < $j; i = i + 1)); do echo -n 'a'"$i"','; done;
   echo -n 'a'$[j]', end_stmt) ';
   for ((i = 1; i < $j; i = i + 1));
   do echo -n 'TRANS_CONS(a'"$i"') ' ;
   done;
   echo -n 'TRANS_CONS(a'$[j]')';
   echo -n ' end_stmt';
   for ((i = 1; i < $j; i = i + 1));
   do echo -n '}}}';
   done;
   echo -n '}}}';
done;
								     
