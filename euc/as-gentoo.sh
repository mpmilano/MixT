#echo `whoami` "is now here: " `pwd` "with id" $1
cd
cd consistency-tester/transactions/
#git checkout pg_env.sh
#git pull
source pg_env.sh
killall -9 vm
#make clean
#causalGroup="$1" make -j4 vm
#./vm
#line 50, hpp
