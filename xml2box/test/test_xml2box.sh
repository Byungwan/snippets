#!/bin/sh

TEMP_FILE=$(mktemp /tmp/xml2box_output.XXXXXX)

trap "rm -f $TEMP_FILE; exit" SIGHUP SIGINT SIGTERM >/dev/null 2>&1

if ../xml2box ElephantsDream_AAC48K_064.sidx.xml "$TEMP_FILE"; then
    echo "xml2box run ... OK"
else
    echo "xml2box run ... ERROR"
    rm -f "$TEMP_FILE"
    exit 1
fi

if cmp ElephantsDream_AAC48K_064.sidx.box "$TEMP_FILE"; then
    echo "xml2box output compare ... OK"
else
    echo "xml2box output compare ... ERROR"
    rm -f "$TEMP_FILE"
    exit 1
fi

rm -f "$TEMP_FILE"
