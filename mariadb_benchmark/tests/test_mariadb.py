# -*- coding: utf-8 -*-
"""
blah blah
"""

import warnings

import pymysql


def test_mariadb_transaction():
    db = pymysql.connect("localhost",
                         "mariadb_benchmark",  # user
                         "mariadb_benchmark",  # password
                         "mariadb_benchmark")  # database
    cursor = db.cursor()

    with warnings.catch_warnings():
        warnings.simplefilter('ignore')
        cursor.execute("DROP TABLE IF EXISTS test_timeline")

    sql = '''CREATE TABLE test_timeline (
    media_start BIGINT UNSIGNED PRIMARY KEY NOT NULL,
    media_duration BIGINT UNSIGNED DEFAULT 0,
    media_sequence INT UNSIGNED DEFAULT 0,

    file_path VARCHAR(512) NOT NULL,
    file_offset BIGINT UNSIGNED DEFAULT 0,
    file_data_size INT UNSIGNED DEFAULT 0,

    key_id BINARY(16)
)'''
    cursor.execute(sql)

    sql = """INSERT INTO test_timeline (media_start, file_path)
             VALUES (123456789, '/stg/ch001/video_eng_3000000/123.mp4')"""
    try:
        cursor.execute(sql)
        db.commit()
    except:
        db.rollback()

    sql = "SELECT media_start, file_path FROM test_timeline"
    try:
        cursor.execute(sql)
        results = cursor.fetchall()
        for row in results:
            media_start = row[0]
            file_path = row[1]
    except:
        pass

    assert media_start == 123456789
    assert file_path == "/stg/ch001/video_eng_3000000/123.mp4"

    db.close()
