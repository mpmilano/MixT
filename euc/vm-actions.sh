#echo hello `whoami`
#echo "I have id $1"
mkdir /mnt/gentoo 2>/dev/null
mount /dev/vdb /mnt/gentoo 2>/dev/null
cp /home/ubuntu/as-gentoo.sh /mnt/gentoo 
cd /mnt/gentoo/
mount --rbind /dev dev 2>/dev/null
rm /dev/shm 2>/dev/null
mkdir /dev/shm 2>/dev/null
mount -t tmpfs shm /dev/shm 2>/dev/null
mount --make-rslave dev 2>/dev/null
mount --rbind /sys/ sys 2>/dev/null
mount -t proc none proc 2>/dev/null
mount --make-rslave sys 2>/dev/null
cp -L /etc/resolv.conf etc/ 2>/dev/null
cd /mnt/
#echo "done so far" `pwd`
chroot gentoo /bin/su -c "/bin/bash /as-gentoo.sh $1 $2 $3 $4 $5 $6" research
#chroot gentoo /bin/su -c "/usr/bin/emerge libunwind" root

