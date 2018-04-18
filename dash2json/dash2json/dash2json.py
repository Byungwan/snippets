# -*- coding: utf-8 -*-

import json
from collections import OrderedDict
import xml.etree.cElementTree as ET


ARRAY_ELEMS = ('Period', 'AdaptationSet', 'Representation',
               'ContentProtection', 'S')
TEXT_ELEMS = ('pro', 'pssh')

NUM_TYPE = ('t', 'd', 'width', 'height', 'bandwidth', 'startWithSAP')
BOOL_TYPE = ('segmentAlignment')

TRUE_VALUES = ('true', 'True', 'TRUE', 't', 'T',
               'yes', 'Yes', 'YES', 'y', 'Y'
               '1')


def strip_ns(name):
    if '}' in name:
        return name.split('}', 1)[1]
    return name


def conv_manifest_py(elem):
    d = OrderedDict()
    elem_tag = strip_ns(elem.tag)

    if elem.attrib:
        attrib = OrderedDict()
        for key, value in list(elem.attrib.items()):
            key = strip_ns(key)
            if key in NUM_TYPE:
                value = int(value)
            elif key in BOOL_TYPE:
                value = value in TRUE_VALUES
            attrib[key] = value
        d['@'] = attrib

    for subelem in elem:
        v = conv_manifest_py(subelem)
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
                d["#"] = text
            else:
                d = text

    return {elem_tag: d}


def dash2json(xmlstring):
    mpd = conv_manifest_py(ET.fromstring(xmlstring))
    return json.dumps(mpd['MPD'], separators=(',', ':'))
