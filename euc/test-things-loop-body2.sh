i=$1
foo="$2"
configuration="$3"
strong_percent="$4"
write_percent="$5"

path=/home/research/results/MyriaStore-"strong_percent"-"write_percent"/"$configuration"/"$foo"/

mkdir -p "$path"
scp  -o "UserKnownHostsFile /dev/null" -o strictHostKeyChecking=no -i MyriaInstances.pem ubuntu@"$foo":/mnt/gentoo/tmp/MyriaStore* "$path"

