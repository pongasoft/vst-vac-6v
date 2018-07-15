VST2/3 VAC-6V - Volume Analyzer & Controller
============================================

This project started as a VST implementation of the [VAC-6](https://pongasoft.com/rack-extensions/VAC6.html) rack extension. It is now the official latest version of this plugin under the new name VAC-6V (V stands for VST). You can check the [VAC-6V](https://pongasoft.com/vst/VAC-6V.html) documentation directly.

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

- Create a folder for the build and `cd` to it (for simplicity I am creating it at the root of the source tree, but can obviously be *outside* the source tree entirely):

        mkdir -p build/Debug
        cd build/Debug

- Generate the Makefile(s): provide the path to the *source* of this project (which contains `CMakeLists.txt`):

        cmake -DCMAKE_BUILD_TYPE=Debug ../..

   Note that you may see a few warnings from `googletest` that I was unable to remove... but feel free to ignore (or if you find a way to remove them, please let me know!)

- Now build the plugin (all its dependencies will be built as well):

        cmake --build .

- Testing that it is a valid VST3 plugin (already run part of the build, but can be run separately):

        ./bin/validator VST3/pongasoft_VAC6V.vst3

- Deploying the plugin and testing in a real DAW

    -  For VST2 (like Maschine and Reason) you copy and *rename* it:

            mkdir -p ~/Library/Audio/Plug-Ins/VST
            cp -r VST3/pongasoft_VAC6V.vst3 ~/Library/Audio/Plug-Ins/VST/pongasoft_VAC6V.vst

    -  For VST3:

            mkdir -p ~/Library/Audio/Plug-Ins/VST3
            cp -r VST3/pongasoft_VAC6V.vst3 ~/Library/Audio/Plug-Ins/VST3

- You can also run the unit tests part of this project:

        cmake --build . --target VST_VAC6_test
        ctest

Because this project uses `cmake` you can also generate an Xcode project by using the proper generator (`-G Xcode`). You can also load the project directly in CLion.

Building this project for Windows
---------------------------------
- Create a folder for the build and `cd` to it (for simplicity I am creating it at the root of the source tree, but can obviously be *outside* the source tree entirely):

      mkdir build
      cd build

- Generate the Makefile(s): provide the path to the *source* of this project (which contains `CMakeLists.txt`):

      cmake -G"Visual Studio 15 2017 Win64" -DCMAKE_CXX_FLAGS=/D_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING ../

- Now build the plugin (all its dependencies will be built as well) (note that unlike macOS the type of build is specified during the build not during the generation of the project) (use `Debug` for development version, `Release` for production version):

      cmake --build . --config Debug

- You can also run the unit tests part of this project:

        cmake --build . --config Debug --target VST_VAC6_test
        ctest -C Debug

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
