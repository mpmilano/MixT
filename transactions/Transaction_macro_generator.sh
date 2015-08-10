for ((j = 1; j < 50; j = j + 1));
do echo;
   echo -n '#define TRANS_SEQ'"$j"'(';
   for ((i = 1; i < $j; i = i + 1)); do echo -n 'a'"$i"','; done;
   echo -n 'a'$[j]') ';
   for ((i = 1; i < $j; i = i + 1));
   do echo -n 'TRANS_CONS(a'"$i"') ' ;
   done;
   echo -n 'TRANS_CONS(a'$[j]')';
   echo -n ' END_TRANSACTION';
   for ((i = 1; i < $j; i = i + 1));
   do echo -n '}}}';
   done;
   echo -n '}}}';
done;
								     
