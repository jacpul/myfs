#!/bin/bash

# the flies directory stores the stdout from each test.
# the swamps directory stores the dump after each test.

# To create hex data for tests run the following command
# python <<< 's="Hello World"; print "Size:", len(s), "\nHex Data:", "".join(map(lambda x:hex(ord(x))[2:].zfill(2), s)).upper()'
# Remember that every test should end with 'e'

SOURCEFILE="myfs.c"
DO_COMPILE=1
DO_CLEANUP=1
QUIT_ON_ERROR=0

usage()
{
cat <<TEXT
test.sh -- test your myfs
Options:
    -q  Quit on any error.
    -d  Skip clean up on exit.
    -h  Print this screen.
TEXT

exit 0
}

clean_all()
{
    make clean-all &> /dev/null
    if [[ $? -ne 0 ]]; then
        echo "Error Cleaning! :("
        echo "Run 'make clean-all' to see error."
        exit 1
    fi
    rm -rf flies swamps
}

clean()
{
   make clean-all &> /dev/null
	if [[ $? -ne 0 ]]; then
    	echo "Error Cleaning! :("
    	echo "Run 'make clean-all' to see error."
    	exit 1
	fi
}

while getopts "qdh" OPTION
do
    case $OPTION in
        q)
            QUIT_ON_ERROR=1
            echo "Wlll quit on any error."
            ;;
        d)
            DO_CLEANUP=0
            echo "Will skip final cleanup."
            ;;
        h)
            usage
            ;;
    esac
done

echo "Pre-Cleaning..."
clean_all
mkdir flies swamps

if [[ $DO_COMPILE == 1 ]]
then
    echo "Compiling..."
    make &> /dev/null #mount &> /dev/null
    if [[ $? -ne 0 ]]
    then
        echo "Error compiling (run 'make' to see error)."
        exit 1
    fi
fi

BANNED="strcpy strcat strtok sprintf vsprintf gets strlen"

echo "Checking for banned functions: $BANNED"
for bfunc in $BANNED
do
    if grep -nE "\s${bfunc}\W" $SOURCEFILE &> /dev/null
    then
        echo "BANNED FUNCTION '$bfunc' DETECTED on line:"
        echo
        grep -nE "\s${bfunc}\W" $SOURCEFILE
        echo
        echo "Remove all banned functions from your code and try again."
        exit 1
    else
        echo "    $bfunc"
    fi
done
echo "All is good here."
echo

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
            if [[ $QUIT_ON_ERROR == 1 ]]; then
                break
            fi
        else
            diff "swamps/$name" "tests/good/swamps/$name" &> /dev/null
            if [[ $? -ne 0 ]]; then
                echo "Test '$name' Failed :("
                if [[ $QUIT_ON_ERROR == 1 ]]; then
                    break
                fi
            else
                echo "Test '$name' Passed :)"
                ((good++))
            fi
        fi
    else
        echo "Test '$name' Failed :("
        if [[ $QUIT_ON_ERROR == 1 ]]; then
            break
        fi
    fi
    ((tests++))

done

if [[ $DO_CLEANUP == 1 ]]
then
    echo "Post-Cleaning..."
    clean
fi

echo
echo "Passed $good tests of the $tests tested."
echo
echo "Thanks for using the Fly Swamp tester!"
echo "Please come again soon."

