#!/usr/bin/env python

from setuptools import setup

setup(
    name='logchsep',
    version='0.0.1',
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
