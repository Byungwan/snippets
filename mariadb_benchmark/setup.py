import io
from setuptools import find_packages, setup


# Read in the README for the long description of PyPI
def long_description():
    with io.open('README.rst', 'r', encoding='utf-8') as f:
        readme = f.read()
    return readme

setup(
    name='mariadb_benchmark',
    version='0.1',
    description='MariaDB Benchmark',
    long_description=long_description(),
    url='https://github.com/Byungwan/snippets/mariadb_benchmark',
    author='Byungwan Jun',
    author_email='byungwan.jun@inisoft.co.kr',
    license='MIT',
    packages=find_packages(),
    classifiers=[
        'Programming Language :: Python :: 2.7',
        'Programming Language :: Python :: 3',
        'Programming Language :: Python :: 3.5',
    ],
    zip_safe=False,
    install_requires=['PyMySQL'],
    setup_requires=['pytest-runner'],
    tests_require=['pytest'])
