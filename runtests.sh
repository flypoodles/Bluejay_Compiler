#!/bin/bash

COMPILER=bluejaycc
DUMMY=test
OUTPUT=output
TEST_DIR=tests
TIMEOUT=1s
DIFF_FILE=d

PASSING=0

RED='\033[41;37m'
GREEN='\033[42m'
YELLOW='\033[43m'
RESET='\033[0m'

#export ASAN_OPTIONS="detect_leaks=false"
export LSAN_OPTIONS="exitcode=23"
export MallocNanoZone="0"

make clean && make

START=$(date +%s%N | cut -b1-13 | sed s/N/000/g)
for T in $(ls $TEST_DIR | grep -E "$1" | sort)
do
    PASS=1
    LEAK=1
    for F in $(ls $TEST_DIR/$T | grep ".bluejay$" | grep pass | sort)
    do
        echo -n -e "Running ${GREEN}positive test${RESET} $T/$F"
        echo -n $'\t'
        TESTFILE="$TEST_DIR/$T/$F"
        EXPECTED=$(sed 's/.bluejay/.expected/g' <<<"$TESTFILE")
        cp $TESTFILE $DUMMY
        T1=$(date +%s%N | cut -b1-13 | sed s/N/000/g)
        (ASAN_OPTIONS="detect_leaks=false" ./$COMPILER $TESTFILE out.mips || false) &> /dev/null
        RET=$?
        T2=$(date +%s%N | cut -b1-13 | sed s/N/000/g)
        TT=$((T2-T1))

        if [ $RET -eq 0 ]
        then
            ./$COMPILER $TESTFILE out.mips &> /dev/null
            RETLEAK=$?
            if [ $RETLEAK -eq 23 ]
            then
                echo -e -n "${YELLOW}COMPILE WITH LEAKS in ${TT}ms${RESET}\t"
                LEAK=0
            else
               echo -e -n "${GREEN}COMPILE OK in ${TT}ms ${RESET}\t"
            fi
        else
           echo -e "${RED}COMPILE FAIL in ${TT}ms ${RESET}"
           cat $DIFF_FILE
           PASS=0
           continue
        fi

        T1=$(date +%s%N | cut -b1-13 | sed s/N/000/g)
        spim -stext 1000000 -quiet -file out.mips &> $OUTPUT
        RET=$?
        echo "" >> $OUTPUT
        perl -0777 -pi -e 's/SPIM Version.*\nCopyright.*\nAll Rights.*\nSee.*\nLoaded.*\n//' $OUTPUT
        T2=$(date +%s%N | cut -b1-13 | sed s/N/000/g)
        TT=$((T2-T1))

        if [ $RET -eq 0 ]
        then
            echo -e -n " ${GREEN}RUN OK in ${TT}ms${RESET}\t"
        else
            echo -e " ${RED}RUN FAIL in ${TT}ms${RESET}"
            cat $OUTPUT
            PASS=0
            continue
        fi


        # Remove possible Windows \r from output and expected files
        sed 's/\r$//' -i $OUTPUT   &> /dev/null
        sed 's/\r$//' -i $EXPECTED &> /dev/null
        diff $EXPECTED $OUTPUT &> $DIFF_FILE
        DIFF=$?
        rm $DUMMY $OUTPUT &> /dev/null

        if [ $DIFF -eq 0 ]
        then
            echo -e " ${GREEN}DIFF OK${RESET}\t"
        else
            echo -e " ${RED}DIFF FAIL${RESET}\t"
            cat $DIFF_FILE
        fi

        if [ $RET -ne 0 ] || [ $DIFF -ne 0 ]
        then
            PASS=0
            #continue 2
        fi
    done

    for F in $(ls $TEST_DIR/$T |  grep ".bluejay$" |grep fail | sort)
    do
        echo -n -e "Running ${RED}negative test${RESET} $T/$F"
        echo -n $'\t'
        TESTFILE="$TEST_DIR/$T/$F"
        cp $TESTFILE $DUMMY
        (ASAN_OPTIONS="detect_leaks=false" ./$COMPILER $DUMMY &> /dev/null)
        RET=$?
        $(./$COMPILER $DUMMY &> /dev/null)
        RETLEAK=$?

        if [ $RET -ne 0 ]
        then
            echo -e -n "${GREEN}RET OK${RESET}"
        else
            echo -e -n "${RED}RET FAIL${RESET}"
            #continue 2
            PASS=0
        fi

        if [ $RETLEAK -eq 23 ]
        then
            echo -e " ${YELLOW}LEAKS FAIL${RESET}"
            LEAK=0
        else
            echo -e " ${GREEN}LEAKS OK${RESET}"
        fi


        rm $DUMMY
    done


    if [ $PASS -eq 0 ]
    then
            echo -e "${RED}                   TEST ${T} FAILING                  ${RESET}"
            echo -e "${RED}                   TEST ${T} FAILING                  ${RESET}"
            echo -e "${RED}                   TEST ${T} FAILING                  ${RESET}"
    elif [ $LEAK -eq 0 ]
    then
            echo -e "${YELLOW}                   TEST ${T} PASSING WITH MEM LEAKS               ${RESET}"
            echo -e "${YELLOW}                   TEST ${T} PASSING WITH MEM LEAKS               ${RESET}"
            echo -e "${YELLOW}                   TEST ${T} PASSING WITH MEM LEAKS               ${RESET}"
    else
            echo -e "${GREEN}                   TEST ${T} PASSING               ${RESET}"
            echo -e "${GREEN}                   TEST ${T} PASSING               ${RESET}"
            echo -e "${GREEN}                   TEST ${T} PASSING               ${RESET}"
            LEAKING=$(($LEAKING+1))
    fi

    PASSING=$(($PASSING+$PASS))

done
END=$(date +%s%N | cut -b1-13 | sed s/N/000/g)
echo "Finished in $((END-START))ms"

echo $LEAKING > leak.txt

echo $PASSING
