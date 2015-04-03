`test_osg` for fun
===================

Version 0.0.1 [![Build Status](https://travis-ci.org/msgpack/msgpack-c.svg?branch=master)](https://travis-ci.org/msgpack/msgpack-c)

It's like JSON but small and fast.

Overview
--------

[Tesg_osg] is an sample project for testing OSG, Bullet and other
interesting things  

Example
-------


See [`QUICKSTART.md`](./QUICKSTART.md) for more details.


Usage
-----

### Running from command string

To change zone from command string:

    test_osg "zone_name"

Empty, sheremetyevo, adler zones are  currently supported.

### Building and Installing

### Dependences

You will need:

 - `OpenSceneGraph 3.2.1`
 - `osgWorks 3.0.0`
 - `osgBullet 3.0.0`
 - `COLLADA`
 - `pugixml`
 - `bullet`
 - `Spark 1.5.5`
 - `boost >= 1.5.0 && < 1.5.5   `
 
Some libraries from private project 
 - `alloc`
 - `fms`
 - `bada`
 - `meteo`

#### Install from git repository

##### Using cmake

###### Using the Terminal (CLI)

You will need:

 - `gcc >= 4.1.0`
 - `cmake >= 2.8.0`

C and C++03:

    $ git clone https://github.com/yaroslav-tarasov/test_osg.git
    $ cd test_osg
    $ cmake .
    $ make
    $ sudo make install

If you want to setup C++11 version of msgpack instead,
execute the following commands:

    $ git clone https://github.com/yaroslav-tarasov/test_osg.git
    $ cd test_osg
    $ cmake -DMSGPACK_CXX11=ON .
    $ sudo make install

##### GUI on Windows

Clone test_osg git repository.

    $ git clone https://github.com/yaroslav-tarasov/test_osg.git

or using GUI git client.

e.g.) tortoise git https://code.google.com/p/tortoisegit/

1. Launch [cmake GUI client](http://www.cmake.org/cmake/resources/software.html).

2. Set 'Where is the source code:' text box and 'Where to build
the binaries:' text box.

3. Click 'Configure' button.

4. Choose your Visual Studio version.

5. Click 'Generate' button.

6. Open the created msgpack.sln on Visual Studio.

7. Build all.

### Documentation

You can get addtional information on the
[wiki](https://github.com/yaroslav-tarasov/test_osg/wiki).

Contributing
------------

`test_otsg` is developed on GitHub at [test_osg](https://github.com/yaroslav-tarasov/test_osg).
To report an issue or send a pull request, use the
[issue tracker](https://github.com/yaroslav-tarasov/test_osg/issues).


