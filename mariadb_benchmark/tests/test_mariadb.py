# -*- coding: utf-8 -*-
'''
blah blah
'''

import pymysql
import warnings


def test_mariadb_transaction():
    db = pymysql.connect('localhost',
                         'mariadb_benchmark',  # user
                         'mariadb_benchmark',  # password
                         'mariadb_benchmark')  # database
    cur = db.cursor()

    with warnings.catch_warnings():
        warnings.simplefilter('ignore')
        cur.execute('DROP TABLE IF EXISTS test_timeline')

    stmt_create = '''CREATE TABLE test_timeline (
    media_start BIGINT UNSIGNED PRIMARY KEY NOT NULL,
    media_duration BIGINT UNSIGNED DEFAULT 0,
    media_sequence INT UNSIGNED DEFAULT 0,

    file_path VARCHAR(512) NOT NULL,
    file_offset BIGINT UNSIGNED DEFAULT 0,
    file_data_size INT UNSIGNED DEFAULT 0,

    key_id BINARY(16))'''
    cur.execute(stmt_create)

    seg_info = (123456789, '/stg/ch001/video_eng_3000000/123.mp4')
    stmt_insert = '''INSERT INTO test_timeline (media_start, file_path)
    VALUES (%s, %s)'''
    try:
        cur.execute(stmt_insert, seg_info)
        db.commit()
    except:
        db.rollback()

    stmt_select = 'SELECT media_start, file_path FROM test_timeline'
    try:
        cur.execute(stmt_select)
        res = cur.fetchall()
    except:
        pass

    assert len(res) == 1
    assert res[0] == seg_info

    db.close()
