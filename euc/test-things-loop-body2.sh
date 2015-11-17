i=$1
foo="$2"
num_per="$3"
configuration="$4"

path=/home/research/results/MyriaStore/"$num_per"per/"$configuration"/"$foo"/

mkdir -p "$path"
scp  -o "UserKnownHostsFile /dev/null" -o strictHostKeyChecking=no -i MyriaInstances.pem ubuntu@"$foo":/mnt/gentoo/tmp/MyriaStore* "$path"

