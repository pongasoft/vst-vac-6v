VST2/3 VAC-6V - Volume Analyzer & Controller
============================================

This project started as a VST implementation of the VAC-6 rack extension. It is now the official latest version of this plugin under the new name VAC-6V (V stands for VST). You can check the [VAC-6V](https://pongasoft.com/vst/VAC-6V.html) documentation directly.

Release Notes
-------------

### 2019-01-02 - `v1.2.0`
* Slight tweaks to the scrollbar
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

The following steps describes (for each platform) how to build the plugin.

### macOS

- For simplicity I am creating the build at the root of the source tree, but can obviously be *outside* the source tree entirely by running the script from anywhere

        ./configure.sh
        cd build

- In order to build, test, validate, etc... simply use the `jamba.sh` script (use `-h` for details):

         ./jamba.sh -h

### Windows

- For simplicity I am creating the build at the root of the source tree, but can obviously be *outside* the source tree entirely by running the script from anywhere. Note that PowerShell is highly recommended.

        .\configure.bat
        cd build

- In order to build, test, validate, etc... simply use the `jamba.bat` script (use `-h` for details):

         .\jamba.bat -h

Misc
----
- This project uses [loguru](https://github.com/emilk/loguru) for logging (included under `src/cpp/logging`)
- The background image is coming from user [jojo-ojoj @ deviant art](http://fav.me/d7dn7bl)

Licensing
---------
GPL version 3
