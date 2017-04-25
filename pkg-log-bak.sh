#!/bin/bash

echoerr() { echo "$@" 1>&2; }

gzip_log_dir() {
    if [ $# -eq 0 ]; then
        echoerr "gzip_log_dir() takes exactly 1 argument (0 given)"
        return 1
    fi
    log_dir=$1
    if tar zcvf "${log_dir}.tar.gz" -C "${log_dir}" . ; then
        \rm -rf "${log_dir}"
    fi
}

usage() { echo "Usage: $0 -t TOP_DIR LOG_DIR ..." 1>&2; exit 1; }

while getopts "t:h" opt; do
    case "${opt}" in
        t)
            top_dir=${OPTARG}
            ;;
        h)
            usage
            ;;
        *)
            usage
            ;;
    esac
done
shift $((OPTIND-1))

if [ "$top_dir" == "" ] || [ $# -eq 0 ]; then
    usage
fi

while [ $# -gt 0 ]; do
    for d in $(find "${top_dir}/${1}" -mindepth 1 -maxdepth 1 -type d -mtime +1)
    do
        gzip_log_dir d
    done
    shift
done
