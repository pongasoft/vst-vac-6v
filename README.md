VST2/3 VAC-6V - Volume Analyzer & Controller
============================================

This project started as a VST implementation of the VAC-6 rack extension. It is now the official latest version of this plugin under the new name VAC-6V (V stands for VST). You can check the [VAC-6V](https://pongasoft.com/vst/VAC-6V.html) documentation directly.

Status for `master` (unreleased)
--------------------------------
* Extracted and use of [jamba](https://github.com/pongasoft/jamba) framework

2018-07-15 - Status for tag `v1.0.0`
------------------------------------
* first public release / free / open source

Configuration and requirements
------------------------------
Check the Jamba [README](https://github.com/pongasoft/jamba/blob/master/README.md) file for instructions on how to install and configure the VST3 SDK.

Building this project for macOS
-------------------------------

- For simplicity I am creating the build at the root of the source tree, but can obviously be *outside* the source tree entirely by running the script from anywhere

        ./configure.sh Debug
        cd build/Debug

- In order to build the plugin run:

        ./build.sh

- In order to test the plugin (unit tests) run:

        ./test.sh

- In order to validate the plugin (uses validator) run:

        ./validate.sh

- In order to edit the plugin UI (uses editor) run:

        ./edit.sh

- In order to install the plugin locally run (~/Library/Audio/Plug-Ins/VST for VST2 and ~/Library/Audio/Plug-Ins/VST3 for VST3):

        ./install.sh

Because this project uses `cmake` you can also generate an Xcode project by using the proper generator (`-G Xcode`). You can also load the project directly in CLion.

Building this project for Windows
---------------------------------

- For simplicity I am creating the build at the root of the source tree, but can obviously be *outside* the source tree entirely by running the script from anywhere

        ./configure.bat
        cd build

- In order to build the plugin run:

        For Debug => ./build.bat
        For Release => ./build.bat Release

- In order to test the plugin (unit tests) run:

        For Debug => ./test.bat
        For Release => ./test.bat Release

- In order to validate the plugin (uses validator) run:

        For Debug => ./validate.bat
        For Release => ./validate.bat Release

- In order to edit the plugin UI (uses editor) run:

        ./edit.sh


- In order to install the plugin:

  For VST2, copy VST3/VAC-6V.vst3 and RENAME into VAC-6V.dll under
  - C:\ProgramFiles\VstPlugins
  - or any DAW specific path (64bits)
  - MAKE SURE TO RENAME the file otherwise it will not work

  For VST3, copy VAC-6V.vst3 under
  - C:\Program Files\Common Files\VST3 (may require admin access)
  - or any DAW specific path (64bits)

Building this project for macOS and Windows 10
----------------------------------------------

A convenient script (`build-prod.sh` for macOS and `build-prod.bat` for Windows) will invoke the proper commands to build and zip the entire project for production release. This can be run in any directory and will create a `build` folder.

Misc
----
- This project uses [loguru](https://github.com/emilk/loguru) for logging (included under `src/cpp/logging`)
- The background image is coming from user [jojo-ojoj @ deviant art](http://fav.me/d7dn7bl)

Licensing
---------
GPL version 3
