========
logchsep
========

logchsep parse each line of an access log and print its fields
separated by a given delimiter.  It is useful with cut command.

Monospaced text is marked with two backquotes "``" instead of asterisks;
no bold or italic is possible within it (asterisks just represent
themselves), although in some contexts, code syntax highlighting may be
applied.  Note that in monospaced text, multiple spaces are *not*
collapsed, but are preserved; however, flow and wrapping *do* occur, and
any number of spaces may be replaced by a line break.  Markdown allows
monospaced text within bold or italic sections, but not vice versa -
reStructuredText allows neither.  In summary, the common inline markup
is the following::

    Mark *italic text* with one asterisk, **bold text** with two.
    For ``monospaced text``, use two "backquotes" instead.

Build
-----

Commands::
    cd logchsep
    python setup.py build

Installation
------------

Commands::
    cd logchsep
    sudo python setup.py install

Usage
-----

How to use::
    usage: logchsep [-h] [-d, ---delimiter DELIM] [FILE]

    Change log field delimiter

    positional arguments:
      FILE            with no FILE, or when FILE is -, read standard input

    optional arguments:
      \-h, --help     show this help message and exit
      \-d, ---delimiter DELIM
                       use DELIM instead of TAB for field delimiter

Example
-------

Get 404 client IPs and URLs::
    grep ' 404 ' access_proxy.log | logchsep | cut -f 3,6
