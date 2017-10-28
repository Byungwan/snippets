#!/usr/bin/env python

from setuptools import setup
from logchsep import __version__

setup(
    name='logchsep',
    version=__version__,
    url='http://inisoft.co.kr',
    description='log field delimiter',
    author='Jun, Byungwan',
    author_email='byungwan.jun@inisoft.co.kr',
    license='proprietary',
    packages=['logchsep'],
    entry_points={
        'console_scripts': [
            'logchsep = logchsep:main'
        ]
    }
)
