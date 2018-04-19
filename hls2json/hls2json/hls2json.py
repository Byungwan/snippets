# -*- coding: utf-8 -*-

import re
import json
from collections import OrderedDict

TRUE_VALUES = ('true', 'True', 'TRUE', 't', 'T',
               'yes', 'Yes', 'YES', 'y', 'Y'
               '1')


class M3u8ParseError(Exception):
    def __init__(self, line, msg='error'):
        super(M3u8ParseError, self).__init__(
            '{}: \"{}\"'.format(msg, line))


def conv_type(val, type):
    if type == 'int':
        val = int(val)
    elif type == 'float':
        val = float(val)
    return val


def read_nothing(line, rule):
    pass


def read_x_value(line, rule):
    m = re.match(r'^#[^:]+:(\S+)', line)
    if m:
        val = m.group(1)
        if 'type' in rule:
            val = conv_type(val, rule['type'])
        return val
    elif 'defval' in rule:
        return rule['defval']
    else:
        raise M3u8ParseError(line)


def read_x_pairs(line, rule):
    grps = re.findall(r'([\w-]+)=\"?([\w.:/-]+)\"?', line)
    pairs = OrderedDict()
    for item in grps:
        key = item[0]
        val = item[1]
        if 'type' in rule:
            if key in rule['type']:
                val = conv_type(val, rule['type'][key])
        pairs[key] = val
    return pairs


def read_extinf(line, rule):
    val = read_x_value(line, rule)
    vals = val.split(',')
    ret = {'DURATION': float(vals[0])}
    if len(vals) > 1:
        ret['TITLE'] = vals[1]
    return ret


TAG_RULES = {
    'EXTINF': {
        'fn': read_extinf,
        'require-tags': ('EXT-X-BYTERANGE', 'URL'),
        'many': True
    },
    'EXT-X-BYTERANGE': {
        'fn': read_x_pairs,
    },
    'EXT-X-TARGETDURATION': {
        'fn': read_x_value,
        'type': 'int'
    },
    'EXT-X-MEDIA-SEQUENCE': {
        'fn': read_x_value,
        'type': 'int'
    },
    'EXT-X-KEY': {
        'fn': read_x_pairs,
        'following-tag': 'EXTINF',
        'many': True
    },
    'EXT-X-STREAM-INF': {
        'fn': read_x_pairs,
        'require-tags': ('URL'),
        'type': {
            'BANDWIDTH': 'int'
        },
        'many': True
    },
    'EXT-X-I-FRAME-STREAM-INF': {
        'fn': read_x_pairs,
        'require-tags': ('URL'),
        'type': {
            'BANDWIDTH': 'int'
        },
        'many': True
    },
    'EXT-X-MEDIA': {
        'fn': read_x_pairs,
        'many': True
    },
    'EXT-X-ENDLIST': {
        'fn': read_nothing,
        'many': True
    },
    'EXT-X-DISCONTINUITY': {
        'fn': read_nothing,
        'following-tag': ('EXTINF'),
        'many': True
    },
    'EXT-X-VERSION': {
        'fn': read_x_value,
        'type': 'int',
        'defval': 3,
        'many': False
    },
    'EXT-X-PLAYREADYHEADER': {
        'fn': read_x_value,
        'many': True
    }
}


def get_tag(line):
    m = re.match(r'^#([^:]+)', line)
    if m:
        return m.group(1)
    return None


def get_next_line(line_iter):
    while True:
        line = next(line_iter)
        line = line.strip()
        if line:
            return line


def merge_require_lines(line_iter, tag, value, rule):
    req_tags = rule['require-tags']

    req_line = get_next_line(line_iter)
    if req_line[0] == '#':
        req_tag = get_tag(req_line)
        if req_tag in req_tags:
            req_rule = TAG_RULES[req_tag]
            req_value = req_rule['fn'](req_line, req_rule)
            value[req_tag] = req_value
        else:
            raise M3u8ParseError('expected ' + req_tags[0],
                                 req_line)
    if 'URL' in req_tags:
        if req_line[0] == '#':
            req_line = get_next_line(line_iter)
            if req_line[0] == '#':
                raise M3u8ParseError('expected URL', req_line)
        value['URL'] = req_line


def read_m3u8_lines(lines):
    m3u8 = OrderedDict()

    line_iter = iter(lines)
    try:
        while True:
            line = get_next_line(line_iter)

            tag = get_tag(line)
            if not tag:
                continue

            rule = TAG_RULES.get(tag, None)
            if not rule:
                continue

            fol_tag = None
            fol_value = None
            exp_tag = rule.get('following-tag', None)
            if exp_tag:
                fol_tag = tag
                fol_value = rule['fn'](line, rule)

                line = get_next_line(line_iter)
                tag = get_tag(line)
                if exp_tag != tag:
                    raise M3u8ParseError('expected ' + tag, line)
                rule = TAG_RULES[tag]

            value = rule['fn'](line, rule)
            try:
                m3u8[tag].append(value)
            except KeyError:
                if 'many' in rule and rule['many']:
                    m3u8[tag] = []
                    m3u8[tag].append(value)
                else:
                    m3u8[tag] = value

            if fol_value:
                value[fol_tag] = fol_value

            if 'require-tags' in rule:
                merge_require_lines(line_iter, tag, value, rule)
    except StopIteration:
        pass

    return m3u8


def get_mime_type(type):
    if type == 'AUDIO':
        return 'audio/mp4'
    elif type == 'VIDEO':
        return 'video/mp4'
    elif type == 'SUBTITLES':
        return 'application/mp4'
    else:
        return 'unknown/mp4'


def m3u8_to_master(m3u8):
    hls = OrderedDict()
    hls['@hls'] = {'version': m3u8.get('EXT-X-VERSION', 4)}  # XXX

    hls['Period'] = []
    period = OrderedDict()
    hls['Period'].append(period)

    if 'EXT-X-PLAYREADYHEADER' in m3u8:
        period['ContentProtection'] = []
        for pr in m3u8['EXT-X-PLAYREADYHEADER']:
            period['ContentProtection'].append(pr)

    period['AdaptationSet'] = []

    id = 1
    if 'EXT-X-MEDIA' in m3u8:
        media_group = OrderedDict()
        for media in m3u8['EXT-X-MEDIA']:
            lang = media.get('LANGUAGE', '')
            try:
                media_group[lang].append(media)
            except KeyError:
                media_group[lang] = []
                media_group[lang].append(media)

        for lang, medias in media_group.items():
            adapt_set = OrderedDict()
            period['AdaptationSet'].append(adapt_set)

            adapt_set['@'] = {'id': id}
            id += 1
            if lang:
                adapt_set['@']['lang'] = lang

            adapt_set['Representation'] = []
            for media in medias:
                represent = OrderedDict()
                adapt_set['Representation'].append(represent)
                represent['@'] = {'mimeType': get_mime_type(media['TYPE'])}
                represent['@hls'] = media

    if 'EXT-X-STREAM-INF' in m3u8:
        adapt_set = OrderedDict()
        period['AdaptationSet'].append(adapt_set)

        adapt_set['@'] = {'id': id}
        adapt_set['@hls'] = {'EXT-X-STREAM-INF': True}

        adapt_set['Representation'] = []
        for stream in m3u8['EXT-X-STREAM-INF']:
            represent = OrderedDict()
            adapt_set['Representation'].append(represent)
            represent['@'] = {'media': stream['URL']}
            del stream['URL']
            represent['@hls'] = stream

    return hls


def m3u8_to_stream(m3u8):
    hls = OrderedDict()
    hls['@hls'] = {'version': m3u8.get('EXT-X-VERSION', 4)}  # XXX
    if 'EXT-X-ENDLIST' in m3u8:
        hls['@hls']['type'] = 'static'
    else:
        hls['@hls']['type'] = 'dynamic'

    hls['@'] = {}
    if 'EXT-X-TARGETDURATION' in m3u8:
        hls['@']['duration'] = m3u8['EXT-X-TARGETDURATION']
    if 'EXT-X-MEDIA-SEQUENCE' in m3u8:
        hls['@']['startNumber'] = m3u8['EXT-X-MEDIA-SEQUENCE']

    hls['SegmentList'] = {'SegmentURL': []}
    for extinf in m3u8['EXTINF']:
        seg = {'@': {'media': extinf['URL']}}
        del extinf['URL']
        seg['@hls'] = extinf

        hls['SegmentList']['SegmentURL'].append(seg)

    return hls


def hls2json(m3u8string):
    m3u8 = read_m3u8_lines(m3u8string.splitlines())
    if 'EXTINF' in m3u8:
        hls = m3u8_to_stream(m3u8)
    else:
        hls = m3u8_to_master(m3u8)
    return json.dumps(hls, separators=(',', ':'))
