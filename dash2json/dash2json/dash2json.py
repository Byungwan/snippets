# -*- coding: utf-8 -*-

import json
from collections import OrderedDict
import xml.etree.cElementTree as ET


ARRAY_ELEMS = ('Period', 'AdaptationSet', 'Representation',
               'ContentProtection', 'S')
TEXT_ELEMS = ('pro', 'pssh')


def strip_ns(name):
    if '}' in name:
        return name.split('}', 1)[1]
    return name


def parse_dash(elem):
    global ARRAY_ELEMS
    global TEXT_ELEMS

    d = OrderedDict()
    elem_tag = strip_ns(elem.tag)

    if elem.attrib:
        attrib = OrderedDict()
        for key, value in list(elem.attrib.items()):
            attrib[strip_ns(key)] = value
        d['@'] = attrib

    for subelem in elem:
        v = parse_dash(subelem)

        tag = strip_ns(subelem.tag)

        value = v[tag]

        try:
            d[tag].append(value)
        except KeyError:
            if tag in ARRAY_ELEMS:
                d[tag] = []
                d[tag].append(value)
            else:
                d[tag] = value

    if elem.text:
        text = elem.text.strip()
        if text:
            if elem_tag not in TEXT_ELEMS or elem.attrib:
                d["#text"] = text
            else:
                d = text

    return {elem_tag: d}


def dash2json(xmlstring):
    mpd = parse_dash(ET.fromstring(xmlstring))
    return json.dumps(mpd['MPD'])
