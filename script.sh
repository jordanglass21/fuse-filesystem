if [[ -z "$1" ]]; then
    echo "Missing parameter: State Disk Image"
    echo "USAGE: ./script.sh {test.img | test2.img} <anything, to suppress make>"
    exit 0
fi
if [[ -z "$2" ]]; then
    make
fi
echo "Run GDB? (Enter for yes, q to quit, n to straight mount)"
read opt
if [[ "$opt" == "q" ]]; then
    exit
elif [[ -z "$opt" ]]; then
    gdb --args ./hw3fuse -s -d -image "$1" dir
elif [[ "$opt" == "n" ]]; then 
    ./hw3fuse -image "$1" dir
else 
    echo "Unknown option"
    exit 1
fi
echo "Is the test terminal at project root? (Enter to continue with unmounting)"
read
fusermount -u dir
