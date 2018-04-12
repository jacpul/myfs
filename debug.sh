#!/bin/bash

usage()
{
cat <<TEXT
debug.sh -- debug your myfs
Usage: ./debug.sh [test_num]
Ex: ./debug.sh 0
TEXT

exit 0
}

f=$(ls tests/$1_*.test 2> /dev/null)
if [[ $? -ne 0 ]]; then
    echo "Invalid test number"
    echo
    usage
fi
n=$(echo "$f" | wc -l)
if [[ n -ne 1 ]]; then
    usage
fi

echo "Cleaning..."
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

test=$1

echo "Will run test: $1"
name=$(echo $(basename $f) | cut -d"." -f1)

echo "Name: '$name'"


./fly_swamp fs.iso < "$f" > "flies/$name"
python read_fs.py dump -p > "swamps/$name"
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
        fi
    fi
else
    echo "Test '$name' Failed :("
fi

