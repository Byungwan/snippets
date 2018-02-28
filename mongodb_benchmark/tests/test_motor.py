# -*- coding: utf-8 -*-
'''
blah blah
'''

import asyncio
from motor import motor_asyncio


def test_mongodb_transaction():
    client = motor_asyncio.AsyncIOMotorClient('localhost', 27017)
    db = client.mariadb_benchmark

    async def go():
        await db.test_timeline.drop()

        timeline = db.test_timeline

        seg = {'media_timestamp': 123456789,
               'file_path': '/stg/ch001/video_eng_3000000/123.mp4'}
        res = await timeline.insert_one(seg)
        assert res.inserted_id

        found = await timeline.find_one()
        assert found == seg

    loop = asyncio.get_event_loop()
    loop.run_until_complete(go())
    loop.close()
