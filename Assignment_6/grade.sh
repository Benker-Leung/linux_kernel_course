#!/bin/sh
rm -f linux-4.19.20.tar.xz
make clean; make
rmmod rate_limit.ko

insmod rate_limit.ko port=443 rate=50000
wget https://course.cse.ust.hk/comp4511/Password_Only/linux-4.19.20.tar.xz
rmmod rate_limit.ko 
rm -f linux-4.19.20.tar.xz

insmod rate_limit.ko port=443 rate=5000
wget https://course.cse.ust.hk/comp4511/Password_Only/linux-4.19.20.tar.xz
rmmod rate_limit.ko 
rm -f linux-4.19.20.tar.xz

insmod rate_limit.ko port=443 rate=2000
wget https://course.cse.ust.hk/comp4511/Password_Only/linux-4.19.20.tar.xz
rmmod rate_limit.ko 
rm -f linux-4.19.20.tar.xz
