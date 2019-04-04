
#
# libpmemobj-cpp workshop
#
# These are the materials used during the workshop:
#	slides.pdf contains the slides shown
#	Commands demonstrated during the workshop are listed below.
#	Source files for programming examples are in this repo.
#
# These instructions are designed to work on the workshop guest VMs.
# Start by cloning this repo, so you can easily cut and paste commands from
# this README into your shell.
#
# Many of the sys admin steps described here can be found in the
# Getting Started Guide on pmem.io:
# https://docs.pmem.io/getting-started-guide

#
# Start by making a clone of this repo...
#
git clone https://github.com/pmemhackathon/2019-04-08
cd 2019-04-08

#
# making sure your platform provides an NFIT table
ndctl list -BN     # check the "provider" field

#
# Checking to make sure your kernel supports pmem:
#
uname -r	# see kernel currently running
grep -i pmem /boot/config-`uname -r`
grep -i nvdimm /boot/config-`uname -r`

#
# Use ndctl to show that you have pmem installed:
#
ndctl list -u

#
# create namespace:
#
sudo ndctl create-namespace -f -e namespace0.0 --mode fsdax

#
# Create a DAX-capable file system and mount it:
#
sudo mkdir /mnt/pmem-fsdax
sudo mkfs.ext4 /dev/pmem0
sudo mount -o dax /dev/pmem0 /mnt/pmem-fsdax
sudo chmod 777 /mnt/pmem-fsdax
df -h

#
# Install valgrind for persistent memory.
#
cd
sudo apt-get install autoconf
git clone --recursive https://github.com/pmem/valgrind.git
cd valgrind
./autogen.sh
./configure --prefix=/usr
make -j2
sudo make install

#
# Install latest version of pmdk.
#
cd
sudo apt-get install autoconf pkg-config libndctl-dev libdaxctl-dev
git clone https://github.com/pmem/pmdk
cd pmdk
make -j2
sudo make install prefix=/usr

#
# Also install the C++ bindings for libpmemobj.
#
cd
sudo apt-get install cmake
git clone https://github.com/pmem/libpmemobj-cpp
cd libpmemobj-cpp
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=/usr -DBUILD_EXAMPLES=OFF -DBUILD_TESTS=OFF ..
make -j2
sudo make install

#
# Now everything in the hackathon repo should build...
#
cd
cd 2019-04-08
make

#
# queue.cpp
#
# Simple implementation of a volatile queue.
#
./queue
push 1
push 2
push 3
pop
show

#
# queue_pmemobj.cpp
#
# Simple implementation of a persistent queue.
#
# pmempool create obj may fail since the SDS feature see:
# https://github.com/pmem/issues/issues/1039
# when you create the pool by the pmempool create, you may need use the command:
# PMEMOBJ_CONF="sds.at_create=0" pmempool create obj --layout=queue -s 100M /mnt/pmem-fsdax/queue
#
pmempool create obj --layout=queue -s 100M /mnt/pmem-fsdax/queue
pmempool info /mnt/pmem-fsdax/queue
./queue_pmemobj /mnt/pmem-fsdax/queue
push 1
push 2
push 3
pop
show

#
# simplekv_simple.cpp
#
# Hashmap test program.
#
pmempool create obj --layout=simplekv -s 100M /mnt/pmem-fsdax/simplekv-simple
pmempool info /mnt/pmem-fsdax/simplekv-simple
./simplekv_simple /mnt/pmem-fsdax/simplekv-simple

#
# simplekv_word_count.cpp
#
# A C++ program which reads words to a simplekv hashtable and uses MapReduce
# to count words in specified text files.
#
pmempool create obj --layout=simplekv -s 100M /mnt/pmem-fsdax/simplekv-words
pmempool info /mnt/pmem-fsdax/simplekv-words
./simplekv_word_count /mnt/pmem-fsdax/simplekv-words words1.txt words2.txt

#
# find_bugs.cpp
#
# Program which contains few bugs. Can you find them?
#
pmempool create obj --layout=find_bugs -s 100M /mnt/pmem-fsdax/find_bugs
./find_bugs /mnt/pmem-fsdax/find_bugs

# run find-bugs under pmemcheck
valgrind --tool=pmemcheck ./find_bugs /mnt/pmem-fsdax/find_bugs
