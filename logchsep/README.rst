========
logchsep
========

logchsep parse each line of an access log and print its fields
separated by a given delimiter.  It is useful with cut command.


Build
-----

    cd logchsep
    python setup.py build

Installation
------------

    cd logchsep
    sudo python setup.py install

Usage
-----

    usage: logchsep [-h] [-d, ---delimiter DELIM] [FILE]

    Change log field delimiter

    positional arguments:
      FILE                    with no FILE, or when FILE is -, read standard input

    optional arguments:
      \-h, --help             show this help message and exit

      \-d, ---delimiter DELIM use DELIM instead of TAB for field delimiter

Example
-------
Get 404 client IPs and URLs:

    grep ' 404 ' access_proxy.log | logchsep | cut -f 3,6
