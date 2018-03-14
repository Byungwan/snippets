# -*- coding: utf-8 -*-
import argparse
import fileinput
import re
import signal
import sys


class ParsingError(Exception):
    def __init__(self, pos, str):
        super(ParsingError, self).__init__(
            "syntax error {} \"{}\"".format(pos, str))
        self.pos = pos
        self.str = str


def signal_handler(signal, frame):
    sys.exit(0)


def parse_log(line):
    fields = list()
    index = 0
    while index < len(line):
        letter = line[index]
        if letter == ' ' or letter == '\t':
            index = index + 1
        else:
            if letter == '\"':
                pat = r'^("[^"]*")'
            elif letter == '[':
                pat = r'^(\[[^\]]*\])'
            elif letter == '(':
                pat = r'^(\([^\)]*\))'
            else:
                pat = r'^(\S+)'
            m = re.match(pat, line[index:])
            if m:
                fields.append(m.group(1))
                index = index + m.span()[1]
            else:
                raise ParsingError(index, line)
    return fields


def main():
    parser = argparse.ArgumentParser(prog='logchsep',
                                     description='Change log field delimiter')
    parser.add_argument('file', metavar='FILE', nargs='?', default='-',
                        help='''with no FILE, or when FILE is -,
                             read standard input''')
    parser.add_argument('-d, ---delimiter', dest='delim',
                        metavar='DELIM', default='\t',
                        help='use DELIM instead of TAB for field delimiter')
    args = parser.parse_args()

    if args.file == '-':
        input = fileinput.input()
    else:
        input = fileinput.input(args.file)

    signal.signal(signal.SIGINT, signal_handler)

    for line in input:
        if line != "\n":
            try:
                print(args.delim.join(parse_log(line)))
            except ParsingError as exc:
                print("error: {}".format(exc), file=sys.stderr)


if __name__ == "__main__":
    main()
