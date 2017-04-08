i=$1
foo="$2"
causal_percent="$3"
read_percent="$4"

path=/home/research/results/MyriaStore-"$causal_percent"-"$read_percent"/"$foo"/

mkdir -p "$path"
scp  -o "UserKnownHostsFile /dev/null" -o strictHostKeyChecking=no -i MyriaInstances.pem ubuntu@"$foo":/mnt/gentoo/tmp/MyriaStore* "$path"

