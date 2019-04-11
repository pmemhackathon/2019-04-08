
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
# Compile examples.
#
make

#
# Warmup - a simple persistent counter.
#
# pmempool create obj may fail since the SDS feature see:
# https://github.com/pmem/issues/issues/1039
# when you create the pool by the pmempool create, you may need use the command:
# PMEMOBJ_CONF="sds.at_create=0" pmempool create obj --layout=queue -s 100M /mnt/pmem-fsdax/queue
#
pmempool create obj --layout=warmup -s 100M /mnt/pmem-fsdax0/pmdkuserX/warmup
./warmup /mnt/pmem-fsdax0/pmdkuserX/warmup

#
# find_bugs.cpp
#
# Program which contains few bugs. Can you find them?
#
pmempool create obj --layout=find_bugs -s 100M /mnt/pmem-fsdax0/pmdkuserX/find_bugs
./find_bugs /mnt/pmem-fsdax0/pmdkuserX/find_bugs

# run find-bugs under pmemcheck
valgrind --tool=pmemcheck ./find_bugs /mnt/pmem-fsdax0/pmdkuserX/find_bugs

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
pmempool create obj --layout=queue -s 100M /mnt/pmem-fsdax0/pmdkuserX/queue
pmempool info /mnt/pmem-fsdax0/pmdkuserX/queue
./queue_pmemobj /mnt/pmem-fsdax0/pmdkuserX/queue
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
pmempool create obj --layout=simplekv -s 100M /mnt/pmem-fsdax0/pmdkuserX/simplekv-simple
pmempool info /mnt/pmem-fsdax0/pmdkuserX/simplekv-simple
./simplekv_simple /mnt/pmem-fsdax0/pmdkuserX/simplekv-simple

#
# simplekv_word_count.cpp
#
# A C++ program which reads words to a simplekv hashtable and uses MapReduce
# to count words in specified text files.
#
pmempool create obj --layout=simplekv -s 100M /mnt/pmem-fsdax0/pmdkuserX/simplekv-words
pmempool info /mnt/pmem-fsdax0/pmdkuserX/simplekv-words
./simplekv_word_count /mnt/pmem-fsdax0/pmdkuserX/simplekv-words words1.txt words2.txt
