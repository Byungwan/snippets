import io
from setuptools import find_packages, setup


# Read in the README for the long description of PyPI
def long_description():
    with io.open('README.rst', 'r', encoding='utf-8') as f:
        readme = f.read()
    return readme

setup(
    name='dash2json',
    version='0.1',
    description='Convert DASH to manifest.json',
    long_description=long_description(),
    url='https://github.com/Byungwan/snippets/dash2json',
    author='Byungwan Jun',
    author_email='byungwan.jun@inisoft.co.kr',
    license='MIT',
    packages=find_packages(exclude=['tests']),
    classifiers=[
        'Development Status :: 3 - Alpha',
        'Intended Audience :: Developers',
        'Topic :: Multimedia :: Video',
        'License :: OSI Approved :: MIT License',
        'Programming Language :: Python :: 3.5',
        'Programming Language :: Python :: 3.6',
    ],
    zip_safe=False,
    install_requires=[],
    # pytest-runner 4.0 has a fatal bug:
    #   https://github.com/pytest-dev/pytest-runner/issues/39
    setup_requires=['pytest-runner<4'],
    tests_require=['pytest'])
