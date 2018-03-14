========
logchsep
========

Installation
------------
You should run the setup command from the distribution root directory.
Running setup.py install builds and installs all modules in one run::
    cd logchsep
    python setup.py install

Usage
-----
The logchsep utility parses each line of an access log from a given file
and writes its fields separated by a given delimiter to the standard
output.  If no file argument is specified, or a file argument is a
single dash ('-'), cut reads from the standard input::
    usage: logchsep [-h] [-d, ---delimiter DELIM] [FILE]

    Change log field delimiter

    positional arguments:
      FILE            with no FILE, or when FILE is -, read standard input

    optional arguments:
      -h, --help     show this help message and exit
      -d, ---delimiter DELIM
                     use DELIM instead of TAB for field delimiter

Example
-------
It is useful with cut utility.
For example, getting 404 client IPs and URLs::
    grep ' 404 ' access_proxy.log | logchsep | cut -f 3,6
