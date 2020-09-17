#! /bin/bash

# set -x

item_count=5000000
tx_count=2000000
refs_count=10
run_count=5
mode=1
first=1
outdir=""

while getopts "i:x:r:c:m:d:" opt
do
    case $opt in
        i)
            item_count=$OPTARG ;;
        x)
            tx_count=$OPTARG ;;
        r)
            refs_count=$OPTARG ;;
        c)
            run_count=$OPTARG ;;
        m)
            mode=$OPTARG ;;
        d)
            outdir=$OPTARG ;;
        *)
            echo "usage: $0 [-i item_count] [-x tx_count] [-r refs_count] [-c run_count] [-m mode]"
            exit 1 ;;
    esac
done

echo "testing with"
echo "  lockable items: $item_count"
echo "  transactions  : $tx_count"
echo "  refs per tx   : $refs_count"
echo "  test mode     : $mode"

if [ $mode == "0" ];
then
    echo "  thread count  : single-threaded"
    first="0"
    run_count="0"
else
    echo "  thread count  : 1..$run_count threads"
fi

echo " "
echo " threads   elapsed msec   conflicts     races"
echo " ============================================="

for ((I = $first ; I <= $run_count ; I++))
do
    fname=""

    if [ $outdir == "" ];
    then
        fname="$(printf "t%02d_mode_%s.txt" $I $mode)"
    else
        fname="$(printf "%s/t%02d_mode_%s.txt" $outdir $I $mode)"
    fi

    ./tx_test -i $item_count -t $I -x $tx_count -r $refs_count -o $fname -m $mode

    total=`grep took $fname | cut -d" " -f4 | paste -s -d+ - | bc`
    count=$I
    denom=$I

    if [ $denom == "0" ];
    then
        denom="1"
    fi

    average=`echo $total/$denom | bc`

    conflicts=`grep CONFLICT $fname | wc -l`
    races=`grep RACE $fname | wc -l`

    printf "    %2d       %7d      %7d      %7d\n" $count $average $conflicts $races
done
echo ""
echo ""
