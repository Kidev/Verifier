#!/usr/bin/env bash

total=0
passed=0
leak=0
error=0

testfor() {

    echo "Testing for file \"$1\""
    echo "================"

    valgrind --log-file=valgrind.log ~/.verify/bin/verify $1
    echo "================"
    grep -qF "no leaks are possible" valgrind.log
    
    if [ $? -ne 0 ]; then
        echo -n "(LEAKS) "
        leak=1
    fi;

    grep -qF "0 errors" valgrind.log
    
    if [ $? -ne 0 ]; then
        echo -n "(ERRORS) "
        error=1
    fi;

    rm valgrind.log

    ~/.verify/bin/verify $1 | grep -qF "$2"
    if [ $? -eq 0 ]; then
        echo "OK"
        passed=`expr $passed + 1`
    else
        echo "FAILED, expected \"$2\""
    fi;
    echo ""

    total=`expr $total + 1`
}

testfor error_no_data.vfy "[INVALID FILE]"
testfor error_no_hdr.vfy "[INVALID FILE]"
testfor error_no_sgn.vfy "[INVALID FILE]"
testfor error_wrongname.vfy "[INVALID FILE]"
testfor invalid_archive_badfile.vfy "[INVALID FILE]"
testfor invalid.vfy "[NOT SAFE]"
testfor unknown.vfy "[NOT SAFE]"
testfor valid.vfy "[SAFE]"
testfor nothere.vfy "[INVALID FILE]"

passed=`expr $passed \* 100`

echo -n `expr $passed \/ $total`
echo "% pass"

if [ $leak -eq 1 ]; then
    echo "There are some leaks to fix!"
fi;
if [ $error -eq 1 ]; then
    echo "There are some errors to fix!"
fi;

