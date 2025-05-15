make clean
rm -rf /etc/users.db
sudo rmmod passwd_module

make
sudo insmod passwd_module.ko
sudo bash ./auth.sh
