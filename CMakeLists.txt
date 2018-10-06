cmake_minimum_required (VERSION 3.12)

# build VST 2?
option(JAMBA_ENABLE_VST2 "Use VST2" ON)

# build Audio Unit?
option(JAMBA_ENABLE_AUDIO_UNIT "Enable Audio Unit" ON)

set(PLUGIN_MAJOR_VERSION 1)
set(PLUGIN_MINOR_VERSION 0)
set(PLUGIN_PATCH_VERSION 0)
set(PLUGIN_VERSION "${PLUGIN_MAJOR_VERSION}.${PLUGIN_MINOR_VERSION}.${PLUGIN_PATCH_VERSION}")

project(VST_VAC6)

# To disable download/update, you can set those 2 variables to local folders
set(JAMBA_ROOT_DIR ../../pongasoft/jamba)
#set(GOOGLETEST_ROOT_DIR ../../google/googletest)

include(jamba.cmake)

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
jamba_add_vst3plugin(${target} "${vst_sources}")
target_include_directories(${target} PUBLIC ${VSTGUI_ROOT}/vstgui4)
target_link_libraries(${target} PRIVATE base sdk vstgui_support jamba)

###################################################
# Resources
###################################################
jamba_add_vst3_resource(${target} PNG "Background.png")
jamba_add_vst3_resource(${target} PNG "Background_2x.png")
jamba_add_vst3_resource(${target} PNG "Button_Bypass_2frames.png")
jamba_add_vst3_resource(${target} PNG "Button_Bypass_2frames_2x.png")
jamba_add_vst3_resource(${target} PNG "Button_InWindow_2frames.png")
jamba_add_vst3_resource(${target} PNG "Button_InWindow_2frames_2x.png")
jamba_add_vst3_resource(${target} PNG "Button_Live_4frames.png")
jamba_add_vst3_resource(${target} PNG "Button_Live_4frames_2x.png")
jamba_add_vst3_resource(${target} PNG "Button_SinceReset_2frames.png")
jamba_add_vst3_resource(${target} PNG "Button_SinceReset_2frames_2x.png")
jamba_add_vst3_resource(${target} PNG "Button_small_2frames.png")
jamba_add_vst3_resource(${target} PNG "Button_small_2frames_2x.png")
jamba_add_vst3_resource(${target} PNG "Knob_mini_63frames.png")
jamba_add_vst3_resource(${target} PNG "Knob_mini_63frames_2x.png")
jamba_add_vst3_resource(${target} DATA "VAC6.uidesc")

# this must be called after you are done calling jamba_add_vst3_resource for all resources
jamba_gen_vst3_resources(${target} "VAC6V")

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

jamba_add_test(${target}_test "${TEST_SRC_FILES}" "${test_sources}" "")

###################################################
# Dev help
###################################################
jamba_dev_scripts(${target})
