from setuptools import setup# find_packages

setup(
    name='rodosmwinterface',
    version='0.1',
    description='rodos middleware in python',
    packages=["rodosmwinterface/"],
    author=["Sebastian Kind"],
    install_requires=["pyserial"],
)
