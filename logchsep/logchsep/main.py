import argparse
import fileinput
import re


def parse_tokens(line):
    tokens = list()
    index = 0
    while index < len(line):
        letter = line[index]
        if letter == '\"':
            pat = r'^("[^"]*")\s'
        elif letter == '[':
            pat = r'^(\[[^\]]*\])\s'
        elif letter == '(':
            pat = r'^(\([^\)]*\))\s'
        else:
            pat = r'^(\S+)\s'
        m = re.match(pat, line[index:])
        if m:
            tokens.append(m.group(1))
            index = index + m.span()[1]
        else:
            index = index + 1

    return tokens


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

    for line in input:
        if line != "\n":
            print(args.delim.join(parse_tokens(line)))

if __name__ == "__main__":
    main()
