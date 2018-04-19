# -*- coding: utf-8 -*-

from hls2json.hls2json import hls2json
import json


def test_master():
    m3u8string = '''#EXTM3U
#EXT-X-VERSION:5
#EXT-X-MEDIA:TYPE=AUDIO,GROUP-ID="audio",NAME="audio_eng",DEFAULT=YES,AUTOSELECT=YES,LANGUAGE="eng",URI="audio_eng_128000/playlist.m3u8"
#EXT-X-STREAM-INF:BANDWIDTH=2000000,AUDIO="audio"
video_eng_2000000/playlist.m3u8
'''
    expected = '''
{
  "@hls":{"version":5},
  "Period":[{
    "AdaptationSet":[{
      "@":{"id":1, "lang":"eng"},
      "Representation":[{
        "@":{"mimeType":"audio/mp4"},
        "@hls":{
          "TYPE":"AUDIO",
          "GROUP-ID":"audio",
          "NAME":"audio_eng",
          "DEFAULT":"YES",
          "AUTOSELECT":"YES",
          "LANGUAGE":"eng",
          "URI":"audio_eng_128000/playlist.m3u8"
        }
      }]
    },{
      "@":{"id":2},
      "@hls":{"EXT-X-STREAM-INF":true},
      "Representation":[{
        "@":{"media":"video_eng_2000000/playlist.m3u8"},
        "@hls":{
          "BANDWIDTH":2000000,
          "AUDIO":"audio"}
      }]
    }]}]}'''
    assert json.loads(expected) == json.loads(hls2json(m3u8string))


def test_stream():
    m3u8string = '''#EXTM3U
#EXT-X-VERSION:5
#EXT-X-TARGETDURATION:4
#EXT-X-MEDIA-SEQUENCE:615
#EXT-X-KEY:METHOD=SAMPLE-AES,URI="skd://5B26CD83395D4C80EEC85EFBB38B41DB",KEYFORMAT="com.apple.streamingkeydelivery",KEYFORMATVERSIONS="1"
#EXTINF:3.76164,title
15235243115298222.ts
#EXTINF:3.69199,title
15235243152914666.ts
#EXTINF:3.76164,title
15235243189834554.ts
#EXTINF:3.73841,title
15235243227451000.ts
#EXTINF:3.76163,title
15235243264835112.ts
#EXTINF:3.73842,title
15235243302451446.ts
#EXTINF:3.76163,title
15235243339835666.ts
#EXTINF:3.73836,title
15235243377452000.ts
#EXTINF:3.76163,title
15235243414835554.ts
#EXTINF:3.73844,title
15235243452451888.ts
'''
    expected = '''
{
  "@hls":{"version":5,"type":"dynamic"},
  "@":{"startNumber":615,"duration":4},
  "SegmentList":{
    "SegmentURL":[
      {
        "@hls":{
          "EXT-X-KEY":{
            "METHOD":"SAMPLE-AES",
            "URI":"skd://5B26CD83395D4C80EEC85EFBB38B41DB",
            "KEYFORMAT":"com.apple.streamingkeydelivery",
            "KEYFORMATVERSIONS":"1"
           },
           "DURATION":3.76164,"TITLE":"title"},
        "@":{"media":"15235243115298222.ts"}},
      {"@hls":{"DURATION":3.69199,"TITLE":"title"},"@":{"media":"15235243152914666.ts"}},
      {"@hls":{"DURATION":3.76164,"TITLE":"title"},"@":{"media":"15235243189834554.ts"}},
      {"@hls":{"DURATION":3.73841,"TITLE":"title"},"@":{"media":"15235243227451000.ts"}},
      {"@hls":{"DURATION":3.76163,"TITLE":"title"},"@":{"media":"15235243264835112.ts"}},
      {"@hls":{"DURATION":3.73842,"TITLE":"title"},"@":{"media":"15235243302451446.ts"}},
      {"@hls":{"DURATION":3.76163,"TITLE":"title"},"@":{"media":"15235243339835666.ts"}},
      {"@hls":{"DURATION":3.73836,"TITLE":"title"},"@":{"media":"15235243377452000.ts"}},
      {"@hls":{"DURATION":3.76163,"TITLE":"title"},"@":{"media":"15235243414835554.ts"}},
      {"@hls":{"DURATION":3.73844,"TITLE":"title"},"@":{"media":"15235243452451888.ts"}}
    ]}}'''
    assert json.loads(expected) == json.loads(hls2json(m3u8string))
