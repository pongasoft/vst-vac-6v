cmake_minimum_required (VERSION 3.9)

# build VST 2 by default
option(SMTG_CREATE_VST2_VERSION "Use VST2" ON)
option(SMTG_CREATE_VST3_LINK "" OFF)

set(JAMBA_GIT_REPO "https://github.com/pongasoft/jamba" CACHE STRING "Jamba git repository url" FORCE)
set(JAMBA_GIT_TAG 09c340734785fbe469fa3b4a678ed7b934d2665a CACHE STRING "Jamba git tag" FORCE)

set(PLUGIN_MAJOR_VERSION 1)
set(PLUGIN_MINOR_VERSION 0)
set(PLUGIN_PATCH_VERSION 0)
set(PLUGIN_VERSION "${PLUGIN_MAJOR_VERSION}.${PLUGIN_MINOR_VERSION}.${PLUGIN_PATCH_VERSION}")

project(VST_VAC6)

# for working on the framework at the same time...
#set(JAMBA_ROOT_DIR ../jamba)

if(NOT JAMBA_ROOT_DIR)
	include(jamba.cmake)
endif()

include(${JAMBA_ROOT_DIR}/jamba.cmake)

set(CPP_SOURCES src/cpp)

configure_file(${CPP_SOURCES}/version.h.in ${CMAKE_BINARY_DIR}/generated/version.h)
include_directories(${CMAKE_BINARY_DIR}/generated/)

if (SMTG_CREATE_VST2_VERSION)
	set(vst2_sources
			${CPP_SOURCES}/VAC6VST2.cpp
			)
endif()

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

# VST2 is only defined for macOS and Windows
if(MAC OR WIN)
	set(vst_sources ${vst_sources} ${vst2_sources})
endif()

set(target pongasoft_VAC6V)
smtg_add_vst3plugin(${target} ${VST3_SDK_ROOT} ${vst_sources})
target_include_directories(${target} PUBLIC ${VSTGUI_ROOT}/vstgui4)
target_link_libraries(${target} PRIVATE base sdk vstgui_support jamba)

smtg_add_vst3_resource(${target} "resource/Background.png")
smtg_add_vst3_resource(${target} "resource/Background_2x.png")
smtg_add_vst3_resource(${target} "resource/Button_Bypass_2frames.png")
smtg_add_vst3_resource(${target} "resource/Button_Bypass_2frames_2x.png")
smtg_add_vst3_resource(${target} "resource/Button_InWindow_2frames.png")
smtg_add_vst3_resource(${target} "resource/Button_InWindow_2frames_2x.png")
smtg_add_vst3_resource(${target} "resource/Button_Live_4frames.png")
smtg_add_vst3_resource(${target} "resource/Button_Live_4frames_2x.png")
smtg_add_vst3_resource(${target} "resource/Button_SinceReset_2frames.png")
smtg_add_vst3_resource(${target} "resource/Button_SinceReset_2frames_2x.png")
smtg_add_vst3_resource(${target} "resource/Button_small_2frames.png")
smtg_add_vst3_resource(${target} "resource/Button_small_2frames_2x.png")
smtg_add_vst3_resource(${target} "resource/Knob_mini_63frames.png")
smtg_add_vst3_resource(${target} "resource/Knob_mini_63frames_2x.png")
smtg_add_vst3_resource(${target} "resource/VAC6.uidesc")

if(MAC)
	smtg_set_bundle(${target} INFOPLIST "${CMAKE_CURRENT_LIST_DIR}/mac/Info.plist" PREPROCESS)
elseif(WIN)
	target_sources(${target} PRIVATE resource/VAC6V.rc)
endif()

# fix for vst2
jamba_fix_vst2(${target})

###################################################
# Create archive (.tgz)
###################################################
jamba_create_archive(${target} VAC-6V)

####################################################
## Testing
####################################################
file(GLOB_RECURSE TEST_SRC_FILES RELATIVE ${PROJECT_SOURCE_DIR} test/cpp/*cpp)
set(test_sources ${test_sources}
		${LOGURU_IMPL}
		${CPP_SOURCES}/ZoomWindow.cpp
		)

jamba_add_test(${PROJECT_NAME}_test "${TEST_SRC_FILES}" "${test_sources}" "")

###################################################
# Dev help
###################################################
jamba_dev_scripts(${target})
