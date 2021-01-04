VST2/3 VAC-6V - Volume Analyzer & Controller
============================================

This project started as a VST implementation of the VAC-6 rack extension. It is now the official latest version of this plugin under the new name VAC-6V (V stands for VST). You can check the [VAC-6V](https://pongasoft.com/vst/VAC-6V.html) documentation directly.

Release Notes
-------------

### 2021-01-04 - `v1.2.1`
* Upgraded to [Jamba](https://github.com/pongasoft/jamba) 5.1.2 / VST3 SDK 3.7.0
* Added support for Apple Chipset / universal build on macOS

### 2019-01-03 - `v1.2.0`
* Tweaks to the scrollbar (now includes zoom handles)
* Use of [Jamba](https://github.com/pongasoft/jamba) framework 3.0.0

### 2018-10-06 - `v1.1.0`
* added Audio Unit support (macOS)
* use of [Jamba](https://github.com/pongasoft/jamba) framework

### 2018-07-15 - `v1.0.0`
* first public release / free / open source

Configuration and requirements
------------------------------
Check the Jamba [README](https://github.com/pongasoft/jamba/blob/master/README.md) file for instructions on how to install and configure the VST3 SDK.

Build this project
------------------

The following steps describes how to build the plugin: 

1. Invoke the `configure.py` python script to configure the project
2. Run the `jamba.sh` (resp. `jamba.bat`) command line to build, test validate...

### macOS

For simplicity I am creating the build at the root of the source tree, but can obviously be *outside* the source tree entirely by running the script from anywhere

```
> ./configure.py -h # to see the help
> ./configure.py
> cd build
> ./jamba.sh -h
```

### Windows

For simplicity I am creating the build at the root of the source tree, but can obviously be *outside* the source tree entirely by running the script from anywhere. Note that PowerShell is highly recommended.

```
> python configure.py -h # to see the help
> python configure.py
> cd build
> .\jamba.bat -h
```

Misc
----
- This project uses [loguru](https://github.com/emilk/loguru) for logging.
- The background image is coming from user [jojo-ojoj @ deviant art](http://fav.me/d7dn7bl)

Licensing
---------
GPL version 3
