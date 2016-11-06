#echo `whoami` "is now here: " `pwd` "with id" $1
cd
cd consistency-tester/transactions/
git checkout pg_env.sh
git checkout vm_main.cpp
git checkout master
git pull
#git checkout 8aa31399d5e62f5ad1f94459ecffb94bc0f18fc8
if [[ -d mutils ]]
then cd mutils; git checkout master; git pull; cd ..
else git clone https://github.com/mpmilano/mutils.git
fi
if [[ -d mutils-tasks ]]
then cd mutils-tasks; git checkout master;  git pull; cd ..
else git clone https://github.com/mpmilano/mutils-tasks.git
fi
cd mutils-tasks; git checkout master; cd ..
if [[ -d mutils-serialization ]]
then cd mutils-serialization;  git checkout master; git pull;  cd ..
else git clone https://github.com/mpmilano/mutils-serialization.git
fi
if [[ -d mutils-containers ]]
then cd mutils-containers;  git checkout master; git pull;  cd ..
else git clone https://github.com/mpmilano/mutils-containers.git
fi
if [[ -d mutils-networking ]]
then cd mutils-networking;  git checkout master; git pull;  cd ..
else git clone https://github.com/mpmilano/mutils-networking.git
fi
source pg_env.sh
export extra_macro_defs="-D$2 -DWRITE_PERCENT=$3 -DSTRONG_PERCENT=$4 $6"
export causalGroup="$1"
export MY_IP="$5"
shift 6
export CAUSAL_REMOTE_IP_1="$2"
export CAUSAL_REMOTE_IP_2="$3"
export STRONG_REMOTE_IP="$1"
shift 3
export num_clients=$1
export client_rate=$2
export increase_by=$3
export increase_delay=$4
export test_stop_time=$5
shift 5
export MAX_THREADS=$1
export first_iter=$2
killall -9 vm
rm /tmp/Myria*
if [[ $first_iter ]];
then make clean
else rm vm_main.o; rm vm
fi
make -j4 vm
./vm
wait
echo "done waiting"
exit
