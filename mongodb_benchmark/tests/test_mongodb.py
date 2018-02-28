# -*- coding: utf-8 -*-
'''
blah blah
'''

from pymongo import MongoClient


def test_mongodb_transaction():
    client = MongoClient('localhost', 27017)
    db = client.mariadb_benchmark

    db.test_timeline.drop()

    timeline = db.test_timeline

    seg = {'media_timestamp': 123456789,
           'file_path': '/stg/ch001/video_eng_3000000/123.mp4'}
    seg_id = timeline.insert_one(seg).inserted_id
    assert seg_id

    found = timeline.find_one()
    assert found == seg
