# -*- coding: utf-8 -*-
import io
from setuptools import find_packages, setup
from logchsep import __version__


def long_description():
    with io.open('README.rst', 'r', encoding='utf-8') as f:
        readme = f.read()
    return readme

setup(
    name='logchsep',
    version=__version__,
    description='Change log field delimiter',
    long_description=long_description(),
    url='https://github.com/Byungwan/snippets/logchsep',
    author='Byungwan Jun',
    author_email='byungwan.jun@inisoft.co.kr',
    license='MIT',
    packages=find_packages(exclude=('tests', 'tests.*')),
    classifiers=[
        'Development Status :: 4 - Beta',
        'Intended Audience :: System Administrators',
        'Topic :: Utilities',
        'License :: OSI Approved :: MIT License',
        'Programming Language :: Python :: 3.5',
        'Programming Language :: Python :: 3.6',
    ],
    zip_safe=False,
    install_requires=[],
    # pytest-runner 4.0 has a fatal bug:
    #   https://github.com/pytest-dev/pytest-runner/issues/39
    setup_requires=['pytest-runner<4'],
    tests_require=['pytest'],
    entry_points={
        'console_scripts': [
            'logchsep = logchsep:main'
        ]
    }
)
