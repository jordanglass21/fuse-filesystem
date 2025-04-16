if [[ -z "$1" ]]; then
    make
fi
echo "Run GDB? (Enter for yes, q to quit, n to straight mount)"
read opt
if [[ "$opt" == "q" ]]; then
    exit
elif [[ -z "$opt" ]]; then
    gdb --args ./hw3fuse -s -d -image test2.img dir
elif [[ "$opt" == "n" ]]; then 
    ./hw3fuse -image test2.img dir
else 
    echo "Unknown option"
    exit 1
fi
echo "Is the test terminal at project root? (Enter to continue with unmounting)"
read
fusermount -u dir
