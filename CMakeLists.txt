cmake_minimum_required (VERSION 3.17)

# build Audio Unit?
option(JAMBA_ENABLE_AUDIO_UNIT "Enable Audio Unit" ON)

# Download VST SDK if not installed?
option(JAMBA_DOWNLOAD_VSTSDK "Download VST SDK if not installed" ON)

# Sets the deployment target for macOS
set(JAMBA_MACOS_DEPLOYMENT_TARGET "10.14" CACHE STRING "macOS deployment target")

# To use local jamba install, uncomment the following line (no download)
set(JAMBA_ROOT_DIR "")
#set(JAMBA_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}/../../pongasoft/jamba")

# download jamba framework
include(fetch_jamba.cmake)

# Determine proper architecture for the project
include("${JAMBA_ROOT_DIR}/cmake/JambaSetArchitecture.cmake")

set(PLUGIN_MAJOR_VERSION 1)
set(PLUGIN_MINOR_VERSION 2)
set(PLUGIN_PATCH_VERSION 1)
set(PLUGIN_VERSION "${PLUGIN_MAJOR_VERSION}.${PLUGIN_MINOR_VERSION}.${PLUGIN_PATCH_VERSION}")

project("vst-vac-6v" VERSION "${PLUGIN_VERSION}")

# To use local googletest install, uncomment the following line (no download) and modify the path accordingly
set(GOOGLETEST_ROOT_DIR "")
#set(GOOGLETEST_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}/../../google/googletest")

# Include Jamba
include("${JAMBA_ROOT_DIR}/jamba.cmake")

set(CPP_SOURCES src/cpp)

# Generating the version.h header file which contains the plugin version (to make sure it is in sync with the version
# defined here)
set(VERSION_DIR "${CMAKE_BINARY_DIR}/generated")
configure_file("${CPP_SOURCES}/version.h.in" "${VERSION_DIR}/version.h")

# Source files for VST2
if (JAMBA_ENABLE_VST2)
  set(vst2_sources
      "${CPP_SOURCES}/VAC6VST2.cpp"
      )
endif ()

set(vst_sources
		${CPP_SOURCES}/controller/GainView.h
		${CPP_SOURCES}/controller/GainView.cpp
		${CPP_SOURCES}/controller/HistoryView.h
		${CPP_SOURCES}/controller/HistoryView.cpp
		${CPP_SOURCES}/controller/LCDDisplayView.h
		${CPP_SOURCES}/controller/LCDDisplayView.cpp
		${CPP_SOURCES}/controller/LCDScrollbarView.h
		${CPP_SOURCES}/controller/LCDScrollbarView.cpp
		${CPP_SOURCES}/controller/MaxLevelView.h
		${CPP_SOURCES}/controller/MaxLevelView.cpp
		${CPP_SOURCES}/controller/VAC6Controller.h
		${CPP_SOURCES}/controller/VAC6Controller.cpp
		${CPP_SOURCES}/controller/VAC6Controller.h
		${CPP_SOURCES}/VAC6CIDs.h
		${CPP_SOURCES}/VAC6Constants.h
		${CPP_SOURCES}/VAC6Model.h
		${CPP_SOURCES}/VAC6Model.cpp
		${CPP_SOURCES}/VAC6AudioChannelProcessor.h
		${CPP_SOURCES}/VAC6AudioChannelProcessor.cpp
		${CPP_SOURCES}/VAC6Plugin.h
		${CPP_SOURCES}/VAC6Plugin.cpp
		${CPP_SOURCES}/VAC6Processor.h
		${CPP_SOURCES}/VAC6Processor.cpp
		${CPP_SOURCES}/VAC6VST3.cpp
		${CPP_SOURCES}/ZoomWindow.h
		${CPP_SOURCES}/ZoomWindow.cpp
		)

# Location of resources
set(RES_DIR "${CMAKE_CURRENT_LIST_DIR}/resource")

# List of resources (images)
set(vst_resources
    "${RES_DIR}/Background.png"
    "${RES_DIR}/Background_2x.png"
    "${RES_DIR}/Button_Bypass_2frames.png"
    "${RES_DIR}/Button_Bypass_2frames_2x.png"
    "${RES_DIR}/Button_InWindow_2frames.png"
    "${RES_DIR}/Button_InWindow_2frames_2x.png"
    "${RES_DIR}/Button_Live_4frames.png"
    "${RES_DIR}/Button_Live_4frames_2x.png"
    "${RES_DIR}/Button_SinceReset_2frames.png"
    "${RES_DIR}/Button_SinceReset_2frames_2x.png"
    "${RES_DIR}/Button_small_2frames.png"
    "${RES_DIR}/Button_small_2frames_2x.png"
    "${RES_DIR}/Knob_mini_63frames.png"
    "${RES_DIR}/Knob_mini_63frames_2x.png"
)

# Location of the test cases
set(TEST_DIR "${CMAKE_CURRENT_LIST_DIR}/test/cpp")

# List of test cases
set(test_case_sources
    "${TEST_DIR}/test-ZoomWindow.cpp"
  )

# Finally invoke jamba_add_vst_plugin
jamba_add_vst_plugin(
    TARGET                   "pongasoft_VAC6V" # name of CMake target for the plugin
    RELEASE_FILENAME         "VAC-6V" # filename for the plugin (VAC-6V.vst3)
    ARCHIVE_ARCHITECTURE     "${JAMBA_ARCHIVE_ARCHITECTURE}"
    TARGETS_PREFIX           "jmb_" # prefix used by all targets directly linked to this plugin
    VST_SOURCES              "${vst_sources}" "${vst2_sources}" # the source files that compose the plugin
    INCLUDE_DIRECTORIES      "${VERSION_DIR}" # we add the version folder to the list of includes
    UIDESC                   "${RES_DIR}/VAC6.uidesc" # the main xml file for the GUI
    RESOURCES                "${vst_resources}" # the resources for the GUI (png files)
    TEST_CASE_SOURCES        "${test_case_sources}"
    TEST_SOURCES             "${CPP_SOURCES}/ZoomWindow.cpp"
    TEST_INCLUDE_DIRECTORIES "${CPP_SOURCES}"
    TEST_LINK_LIBRARIES      "jamba"
)
