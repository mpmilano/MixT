prime=`cat ../big_prime`

query_begin="update global_clock set vc1= vc2= vc3= vc4= "
echo 'set search_path to causalstore,public; select max(vc1) as vc1,max(vc2) as vc2,max(vc3) as vc3,max(vc4) as vc4 from (select max(vc1) as vc1,max(vc2) as vc2,max(vc3) as vc3,max(vc4) as vc4 from "BlobStore" union select max(vc1) as vc1,max(vc2) as vc2,max(vc3) as vc3,max(vc4) as vc4 from "IntStore") as foo' | psql -qt 'DataStore' |  tr -d  [[:space:]] | tr '|' '\n' | while read int
do
	echo 
done
