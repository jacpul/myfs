#!/bin/bash

# the flies directory stores the stdout from each test.
# the swamps directory stores the dump after each test.

# To create hex data for tests run the following command
# python <<< 's="Hello World"; print "Size:", len(s), "\nHex Data:", "".join(map(lambda x:hex(ord(x))[2:].zfill(2), s)).upper()'
# Remember that every test should end with 'e'

echo "Pre-Cleaning..."
make clean-all &> /dev/null
if [[ $? -ne 0 ]]; then
    echo "Error Cleaning! :("
    echo "Run 'make clean' to see error."
    exit 1
fi
rm -rf flies swamps
mkdir flies swamps

echo "Compiling..."
make &> /dev/null #mount &> /dev/null
if [[ $? -ne 0 ]]
then
    echo "Error compiling (run 'make' to see error)."
    exit 1
fi

tests=0
good=0

for t in tests/*.test
do
#    echo "Cleaning and compiling..."
    make clean &> /dev/null; make &> /dev/null
    if [[ $? -ne 0 ]]; then
        echo "Error Cleaning or Compiling! :("
        echo "Run 'make clean; make' to see error."
        exit 1
    fi
    name=$(echo $(basename $t) | cut -d"." -f1)
#    echo "Running test '$name'..."
    ./fly_swamp fs.iso < "$t" > "flies/$name"
    python read_fs.py --dump -p > "swamps/$name"
    if [[ $? -eq 0 ]]; then
        diff "flies/$name" "tests/good/flies/$name" &> /dev/null
        if [[ $? -ne 0 ]]; then
            echo "Test '$name' Failed :("
        else
            diff "swamps/$name" "tests/good/swamps/$name" &> /dev/null
            if [[ $? -ne 0 ]]; then
                echo "Test '$name' Failed :("
            else
                echo "Test '$name' Passed :)"
                ((good++))
            fi
        fi
    else
        echo "Test '$name' Failed :("
    fi
    ((tests++))

done

#echo "Post-Cleaning..."
make clean &> /dev/null

echo "Passed $good tests of $tests."
