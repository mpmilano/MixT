#echo `whoami` "is now here: " `pwd` "with id" $1
cd
cd consistency-tester/transactions/
git checkout pg_env.sh
git pull
if [[ -d mutils]]
then cd mutils; git pull; cd ..
else git clone https://github.com/mpmilano/mutils.git
fi
if [[ -d mutils-tasks]]
then cd mutils-tasks; git pull; cd ..
else git clone https://github.com/mpmilano/mutils.git
fi
if [[ -d mutils-serialization]]
then cd mutils-serialization; git pull; cd ..
else git clone https://github.com/mpmilano/mutils.git
fi
if [[ -d mutils-containers]]
then cd mutils-containers; git pull; cd ..
else git clone https://github.com/mpmilano/mutils.git
fi
source pg_env.sh
killall -9 vm
rm /tmp/Myria*
if [[ $6 ]];
then make clean
else rm vm_main.o; rm vm
fi
concurrencySetting="$2" defineThis="$3" causalGroup="$1" MY_IP="$4" STRONG_REMOTE_IP="$5" make -j4 vm
./vm
#for ((i=1; i <= $2; i++))
#do
#	./vm &
#done
wait
echo "done waiting"
