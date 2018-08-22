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
This project is known to work on macOS High Siera 10.13.3 with Xcode 9.2 installed. It also has been tested on Windows 10 64 bits and Visual Studio Build tools (2017). It requires `cmake` version 3.9 at the minimum. Because it uses `cmake` it should work on other platforms but it has not been tested.

Downloading the SDK
-------------------
You need to download the VST3 SDK version 3.6.9 from [steinberg](https://download.steinberg.net/sdk_downloads/vstsdk369_01_03_2018_build_132.zip) (shasum 256 => `7c6c2a5f0bcbf8a7a0d6a42b782f0d3c00ec8eafa4226bbf2f5554e8cd764964`). Note that 3.6.10 was released in June 2018 but at this time, this project uses 3.6.9.

Installing the SDK
-------------------
Unpack the SDK to the following location (note how I renamed it with the version number)

* `/Users/Shared/Steinberg/VST_SDK.369` for macOS
* `C:\Users\Public\Documents\Steinberg\VST_SDK.369` for windows.

You can also store in a different location and use the `VST3_SDK_ROOT` variable when using cmake to define its location.

Configuring the SDK
-------------------
In order to build both VST2 and VST3 at the same time, you need to run the following commands

    # for macOS
    cd /Users/Shared/Steinberg/VST_SDK.369
    ./copy_vst2_to_vst3_sdk.sh

    # for Windows
    cd C:\Users\Public\Documents\Steinberg\VST_SDK.369
    copy_vst2_to_vst3_sdk.bat

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
