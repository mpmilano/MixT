i=$1
foo="$2"
configuration="$3"

path=/home/research/results/MyriaStore/"$configuration"/"$foo"/

mkdir -p "$path"
scp  -o "UserKnownHostsFile /dev/null" -o strictHostKeyChecking=no -i MyriaInstances.pem ubuntu@"$foo":/mnt/gentoo/tmp/MyriaStore* "$path"

