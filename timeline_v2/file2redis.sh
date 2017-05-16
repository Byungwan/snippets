#!/bin/sh

usage() { echo "Usage: $0 [-i <file>]" 1>&2; exit 1; }

echoerr() { echo "$@" 1>&2; }

while getopts "i:" opt; do
    case "${opt}" in
        i)
            input_file=${OPTARG}
            ;;
        *)
            usage
            ;;
    esac
done
shift $((OPTIND-1))

if [ -z "${input_file}" ]; then
    usage
fi

if [ ! -f "${input_file}" ]; then
    echoerr "No such input file: ${input_file}"
    exit 1
fi

count_file="${input_file}.cnt"
if [ ! -f "${count_file}" ]; then
    echoerr "No such count file: ${count_file}"
    exit 1
fi

line_count=`cat ${count_file}`

sed "s/^\*\\\$TOTAL_COUNT\\\$/\*${line_count}/" "${input_file}"
