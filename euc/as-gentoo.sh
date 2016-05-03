#echo `whoami` "is now here: " `pwd` "with id" $1
cd
cd consistency-tester/transactions/
git checkout pg_env.sh
git pull
if [[ -d mutils ]]
then cd mutils; git pull; cd ..
else git clone https://github.com/mpmilano/mutils.git
fi
if [[ -d mutils-tasks ]]
then cd mutils-tasks; git pull; cd ..
else git clone https://github.com/mpmilano/mutils-tasks.git
fi
if [[ -d mutils-serialization ]]
then cd mutils-serialization; git pull; cd ..
else git clone https://github.com/mpmilano/mutils-serialization.git
fi
if [[ -d mutils-containers ]]
then cd mutils-containers; git pull; cd ..
else git clone https://github.com/mpmilano/mutils-containers.git
fi
if [[ -d mutils-networking ]]
then cd mutils-networking; git pull; cd ..
else git clone https://github.com/mpmilano/mutils-networking.git
fi
source pg_env.sh
killall -9 vm
rm /tmp/Myria*
if [[ $7 ]];
then make clean
else rm vm_main.o; rm vm
fi
extra_macro_defs="-D$2 -DWRITE_PERCENT=$3 -DSTRONG_PERCENT=$4" causalGroup="$1" MY_IP="$5" STRONG_REMOTE_IP="$6" make -j4 vm
./vm
#for ((i=1; i <= $2; i++))
#do
#	./vm &
#done
wait
echo "done waiting"
