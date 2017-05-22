# timeline

A tool for displaying timeline redis format which can be recognized by
human

## Build

    gcc -Wall -g -o timeline timeline.c -lhiredis

## Usage

    Usage: timeline [OPTION]
    
     From REDIS
      -h <hostname>      Server hostname (default: 127.0.0.1)
      -p <port>          Server port (default: 6379)
      -k <key>           Key
     From FILE
      -f <file>          Input RDB dump file, `-' means STDIN

# schedule

A tool for displaying schedule redis format which can be recognized by
human

## Build

    gcc -Wall -g -o schedule schedule.c -lhiredis

## Usage

    Usage: schedule [OPTION]
    
     From REDIS
      -h <hostname>      Server hostname (default: 127.0.0.1)
      -p <port>          Server port (default: 6379)
      -k <key>           Key
     From FILE
      -f <file>          Input RDB dump file, `-' means STDIN

# file2redis.sh

A tool for change to redis protocol from timeline.zset

## Usage

    Usage: file2redis.sh [-i <file>]

# Examples

    file2redis.sh -i /data/dvr/20170214/02/timeline.set \
        | timeline -f - \
        | cut -d ' ' -f 1,3,5
