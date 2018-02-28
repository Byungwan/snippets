# -*- coding: utf-8 -*-
'''
blah blah
'''
from multiprocessing import Process, Queue
import argparse


def f(q):
    q.put([42, None, 'hello'])


def main(args):
    q = Queue()
    ps = [Process(target=f, args=(q,)) for x in range(args.worker_processes)]

    for p in ps:
        p.start()

    print(q.get())

    for p in ps:
        p.join()


if __name__ == '__main__':
    parser = argparse.ArgumentParser(prog='mariadb_benchmark')
    parser.add_argument('--worker-processes', type=int, default=1)

    args = parser.parse_args()
    main(args)
