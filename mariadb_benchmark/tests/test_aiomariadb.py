# -*- coding: utf-8 -*-
'''
blah blah
'''

import asyncio
import warnings
from aiomysql import create_pool


def test_aiomariadb_transaction():
    async def go():
        pool = await create_pool(host='localhost', port=3306,
                                 user='mariadb_benchmark',
                                 password='mariadb_benchmark',
                                 db='mariadb_benchmark',
                                 loop=loop)

        async with pool.get() as conn:
            async with conn.cursor() as cur:
                with warnings.catch_warnings():
                    warnings.simplefilter('ignore')
                    await cur.execute("DROP TABLE IF EXISTS test_timeline")

        async with pool.get() as conn:
            async with conn.cursor() as cur:
                stmt_create = '''CREATE TABLE test_timeline (
                media_timestamp BIGINT UNSIGNED PRIMARY KEY NOT NULL,
                media_duration BIGINT UNSIGNED DEFAULT 0,
                media_sequence INT UNSIGNED DEFAULT 0,

                file_path VARCHAR(512) NOT NULL,
                file_offset BIGINT UNSIGNED DEFAULT 0,
                file_data_size INT UNSIGNED DEFAULT 0,

                key_id BINARY(16))'''
                await cur.execute(stmt_create)
                await conn.commit()

        seg = (123456789, '/stg/ch001/video_eng_3000000/123.mp4')
        async with pool.get() as conn:
            async with conn.cursor() as cur:
                stmt_insert = '''INSERT INTO
                test_timeline (media_timestamp, file_path)
                VALUES (%s, %s)'''

                await cur.execute(stmt_insert, seg)
                await conn.commit()

        async with pool.get() as conn:
            async with conn.cursor() as cur:
                stmt_select = '''SELECT media_timestamp, file_path
                FROM test_timeline'''

                await cur.execute(stmt_select)
                res = await cur.fetchall()

        assert len(res) == 1
        assert res[0] == seg

        pool.close()
        await pool.wait_closed()

    loop = asyncio.get_event_loop()
    loop.run_until_complete(go())
    loop.close()
