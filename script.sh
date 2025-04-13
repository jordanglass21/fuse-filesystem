if [[ -z "$1" ]]; then
    make
fi
echo "Run GDB? (Enter for yes, q to quit)"
read opt
if [[ "$opt" == "q" ]]; then
    exit
fi
gdb --args ./hw3fuse -s -d -image test2.img dir
echo "Is the test terminal at project root? (Enter to continue with unmounting)"
read
fusermount -u dir

