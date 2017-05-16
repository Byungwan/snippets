# logchsep

## Build

    cd logchsep
    python setup.py build

## Install

    cd logchsep
    sudo python setup.py install

## Usage

    usage: logchsep [-h] [-d, ---delimiter DELIM] [FILE]

    Change log field delimiter

    positional arguments:
      FILE                  with no FILE, or when FILE is -, read standard input

    optional arguments:
      -h, --help            show this help message and exit
      -d, ---delimiter DELIM
                            use DELIM instead of TAB for field delimiter

## Example


    logchsep access.log
    cat access.log | logchsep
    grep ' 404 ' access_proxy.log | cut -f 5
