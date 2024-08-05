m5 readfile > workloads
echo "Done reading workloads"

# ESTM-hashtable ESTM-skiplist lockfree-fraser-skiplist
# lockfree-numask-skiplist MUTEX-hoh-list MUTEX-skiplist
# sequential-rbtree SPIN-hashtable SPIN-RCU-tree ESTM-linkedlist
# ESTM-specfriendly-tree lockfree-hashtable lockfree-rotating-skiplist
# MUTEX-lazy-list sequential-hashtable sequential-skiplist
# SPIN-skiplist ESTM-rbtree lockfree-bst lockfree-linkedlist
# MUTEX-hashtable MUTEX-RCU-tree sequential-linkedlist 
# sequential-specfriendly-tree  SPIN-lazy-list SPIN-hoh-list
#
# array-lock                   feather-test3-small         feather-test8                huron-boost-test         
# huron-lockless-toy-manual    phoenix-kmeans
# both-false-and-true-sharing  feather-test3-small-manual  feather-test8-small          huron-boost-test-manual  
# huron-lockless-writer        phoenix-matrixmultiply
# both-true-and-false-sharing  feather-test4               feather-test8-small-manual   huron-false              
# huron-lu-ncb                 phoenix-pca
# false-sharing                feather-test4-small         feather-test9                huron-false-unrolling    
# huron-lu-ncb-manual          phoenix-reverseindex
# false-sharing-with-eviction  feather-test4-small-manual  feather-test9-small          huron-hist               
# huron-mrmw                   phoenix-wc-manual
# feather-test1                feather-test5               feather-test9-small-manual   huron-hist-manual        
# huron-mrsw                   phoenix-wordcount
# feather-test10               feather-test5-small         fs-ts-diffline               huron-linear-reg         
# huron-ref-count              proportional-fs
# feather-test1-small          feather-test6               fs-ts-sameline               huron-linear-reg-manual  
# huron-ref-count-manual       proportional-ts
# feather-test1-small-manual   feather-test6-small         fs-two-thread-small          huron-locked-toy         
# huron-string-match           repetitive-fs-ts-diffline
# feather-test2                feather-test6-small-manual  huron-atomic-lock            huron-locked-toy-manual  
# huron-string-match-manual    repetitive-fs-ts-sameline
# feather-test2-small          feather-test7               huron-boost-spinlock         huron-locked-writer      
# huron-string-match-original  tmi-boost-refcount
# feather-test3                feather-test7-small         huron-boost-spinlock-manual  huron-lockless-toy       
# no-false-sharing             true-sharing
#########################
#   SYNCHROBENCH LIST   #
#########################
if [ -s workloads];
then
    echo "Running customized benchmarks"
    if [[ "$workload" == "MUTEX-hashtable" ]];
    then
        echo "Running MUTEX hashtable"
        cd /home/gem5/false-sharing-benchmarks/synchrobench/c-cpp/bin
	    ./MUTEX-hashtable -t 4 -d 4 > MUTEX-hashtable.txt
	    m5 writefile /home/gem5/false-sharing-benchmarks/synchrobench/c-cpp/bin/MUTEX-hashtable.txt $m5filespath/MUTEX-hashtable.txt
    elif [[ "$workload" == "MUTEX-skiplist" ]];
    then
        echo "Running MUTEX skiplist"
        cd /home/gem5/false-sharing-benchmarks/synchrobench/c-cpp/bin
	    ./MUTEX-skiplist -t 4 -d 4 > MUTEX-skiplist.txt
	    m5 writefile /home/gem5/false-sharing-benchmarks/synchrobench/c-cpp/bin/MUTEX-skiplist.txt $m5filespath/MUTEX-skiplist.txt
else
    echo "Couldn't find any workload"
    m5 exit
    m5 exit
    m5 exit
fi