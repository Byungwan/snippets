# -*- coding: utf-8 -*-
from logchsep.main import parse_log


def test_parse_log():
    log_line = ('[14/Mar/2018:03:52:03 +0900] '
                '200 '
                '192.168.10.135 '
                '- '
                '- '
                '"GET /live/ocn/s6/QualityLevels(3000000,as=video_eng)'
                '/Fragments(video_eng=15209670914208222) HTTP/1.1" '
                '1595458 '
                '"-" '
                '"User Agent" '
                '"192.168.10.213" '
                '"HIT [14/Mar/2018:03:52:03 +0900]" '
                '"0.000" '
                '"-" '
                '"-"')
    expected = ['[14/Mar/2018:03:52:03 +0900]',
                '200',
                '192.168.10.135',
                '-',
                '-',
                '"GET /live/ocn/s6/QualityLevels(3000000,as=video_eng)'
                '/Fragments(video_eng=15209670914208222) HTTP/1.1"',
                '1595458',
                '"-"',
                '"User Agent"',
                '"192.168.10.213"',
                '"HIT [14/Mar/2018:03:52:03 +0900]"',
                '"0.000"',
                '"-"',
                '"-"']
    actual = parse_log(log_line)
    assert expected == actual
