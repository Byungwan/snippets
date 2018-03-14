# -*- coding: utf-8 -*-
'''
blah blah
'''
from mariadb_benchmark import main
from collections import namedtuple


Args = namedtuple('Args', 'worker_processes')


def test_main(capfd):
    args = Args(1)
    main(args)

    out, err = capfd.readouterr()
    assert out == "[42, None, 'hello']\n"
    assert err == ""

    args = Args(1)
    main(args)

    out, err = capfd.readouterr()
    assert out == "[42, None, 'hello']\n[42, None, 'hello']\n"
    assert err == ""
