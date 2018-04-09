#!/bin/bash

hostname="127.0.0.1"
port=6379
ch=""
minus=""
start=""
end=""
human=0
utc=0
verbose=0

function usage
{
    echo "Usage: timeline-all.sh [OPTION] ... " 1>&2;
    echo "OPTION:" 1>&2;
    echo "  -h <hostname>      Server hostname (default: 127.0.0.1)" 1>&2;
    echo "  -p <port>          Server port (default: 6379)" 1>&2;
    echo "  -c <channel>       channel name" 1>&2;
    echo "  -m <time>          minus (timescale: minutes)" 1>&2;
    echo "  -s <time>          start time (inclusive) ex: 2018-04-09_17:33:21" 1>&2;
    echo "  -e <time>          end time (exclusive) ex: 2018-04-10_08:15:00" 1>&2;
    echo "" 1>&2;
    exit 1
}

while getopts "c:h:p:m:s:e:HUV" opt; do
    case "${opt}" in
        c)
            ch=${OPTARG}
            ;;
        h)
            hostname=${OPTARG}
            ;;
        p)
            port=${OPTARG}
            ;;
        m)
            minus=${OPTARG}
            ;;
        s)
            start=${OPTARG}
            ;;
        e)
            end=${OPTARG}
            ;;
        H)
            human=1
            ;;
        U)
            utc=1
            ;;
        V)
            verbose=1
            ;;
        *)
            usage
            ;;
    esac
done
if [ "$ch" == "" ]; then
    usage
fi

timeline_args=""
if [ "$minus" != "" ]; then
    s=$(date -d "${minus} min ago" +%s)
    start="${s}0000000"
fi
if [ "$start" != "" ]; then
    timeline_args="-s $start"
fi
if [ "$end" != "" ]; then
    timeline_args="${timeline_args} -e $end"
fi
if [ $human -ne 0 ]; then
    timeline_args="${timeline_args} -H"
fi
if [ $utc -ne 0 ]; then
    timeline_args="${timeline_args} -U"
fi
if [ $verbose -ne 0 ]; then
    timeline_args="${timeline_args} -V"
fi


for trk in $(mediainfo -h "$hostname" -p "$port" -c "$ch")
do
    echo "-- ${ch}/${trk} ---------------"
    for aid in $(schedule -h "$hostname" -p "$port" -c "$ch" -V | awk '{print $4}' | sort | uniq)
    do
        timeline -h "$hostname" -p "$port" -k "${aid}/${trk}/timeline.zset" $timeline_args
    done
done
