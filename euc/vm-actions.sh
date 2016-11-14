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
echo "
*         hard    nofile      500000
*         soft    nofile      500000
root      hard    nofile      500000
root      soft    nofile      500000
" > /etc/security/limits.conf
cp /etc/security/limits.conf etc/security/limits.conf
cp -L /etc/resolv.conf etc/ 2>/dev/null
cd /mnt/
if [[ -f /mnt/gentoo/etc/portage/package.env ]]
then
	echo "this one is spared"
else
	echo "dev-libs/libpqxx clang" > /mnt/gentoo/etc/portage/package.env
	chroot gentoo /usr/bin/emerge --oneshot dev-libs/libpqxx
fi

#echo "done so far" `pwd`
echo $*
chroot gentoo /bin/su -c "/bin/bash /as-gentoo.sh $*" research
#chroot gentoo /bin/su -c "/usr/bin/emerge libunwind" root
echo exiting vm
pkill --signal HUP sshd
exit
