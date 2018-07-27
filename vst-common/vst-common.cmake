#-------------------------------------------------------------------------------
# Options
#-------------------------------------------------------------------------------
option(VST_COMMON_DEBUG_LOGGING "Enable debug logging for vst_common framework" OFF)

#-------------------------------------------------------------------------------
# Including VST3 SDK
#-------------------------------------------------------------------------------

include(${CMAKE_CURRENT_LIST_DIR}/VST3_SDK.cmake)

#-------------------------------------------------------------------------------
# Defining files to include to generate the library
#-------------------------------------------------------------------------------

set(VST_COMMON_ROOT ${CMAKE_CURRENT_LIST_DIR})
set(VST_COMMON_CPP_SOURCES ${CMAKE_CURRENT_LIST_DIR}/src/cpp)
set(LOGURU_IMPL ${VST_COMMON_CPP_SOURCES}/pongasoft/logging/logging.cpp)
include_directories(${VST_COMMON_CPP_SOURCES})

set(VST_COMMON_SRC_COMMON_DIR ${VST_COMMON_CPP_SOURCES}/pongasoft/VST/Common)
set(VST_COMMON_SRC_GUI_DIR ${VST_COMMON_CPP_SOURCES}/pongasoft/VST/GUI)
set(VST_COMMON_SRC_RT_DIR ${VST_COMMON_CPP_SOURCES}/pongasoft/VST/RT)

set(VST_COMMON_sources_h
    ${VST_COMMON_CPP_SOURCES}/pongasoft/logging/logging.h
    ${VST_COMMON_CPP_SOURCES}/pongasoft/logging/loguru.hpp

    ${VST_COMMON_CPP_SOURCES}/pongasoft/Utils/Clock/Clock.h
    ${VST_COMMON_CPP_SOURCES}/pongasoft/Utils/Collection/CircularBuffer.h
    ${VST_COMMON_CPP_SOURCES}/pongasoft/Utils/Concurrent/Concurrent.h
    ${VST_COMMON_CPP_SOURCES}/pongasoft/Utils/Lerp.h
    ${VST_COMMON_CPP_SOURCES}/pongasoft/Utils/Misc.h

    ${VST_COMMON_CPP_SOURCES}/pongasoft/VST/AudioBuffer.h
    ${VST_COMMON_CPP_SOURCES}/pongasoft/VST/AudioUtils.h
    ${VST_COMMON_CPP_SOURCES}/pongasoft/VST/Messaging.h
    ${VST_COMMON_CPP_SOURCES}/pongasoft/VST/NormalizedState.h
    ${VST_COMMON_CPP_SOURCES}/pongasoft/VST/ParamConverters.h
    ${VST_COMMON_CPP_SOURCES}/pongasoft/VST/ParamDef.h
    ${VST_COMMON_CPP_SOURCES}/pongasoft/VST/Parameters.h
    ${VST_COMMON_CPP_SOURCES}/pongasoft/VST/SampleRateBasedClock.h
    ${VST_COMMON_CPP_SOURCES}/pongasoft/VST/Timer.h

    ${VST_COMMON_CPP_SOURCES}/pongasoft/VST/RT/RTParameter.h
    ${VST_COMMON_CPP_SOURCES}/pongasoft/VST/RT/RTProcessor.h
    ${VST_COMMON_CPP_SOURCES}/pongasoft/VST/RT/RTState.h

    ${VST_COMMON_CPP_SOURCES}/pongasoft/VST/GUI/Params/GUIParamCxMgr.h
    ${VST_COMMON_CPP_SOURCES}/pongasoft/VST/GUI/Params/GUIParameter.h
    ${VST_COMMON_CPP_SOURCES}/pongasoft/VST/GUI/Params/GUIParameters.h
    ${VST_COMMON_CPP_SOURCES}/pongasoft/VST/GUI/Params/GUIParamCxAware.h
    ${VST_COMMON_CPP_SOURCES}/pongasoft/VST/GUI/Params/GUIRawParameter.h
    ${VST_COMMON_CPP_SOURCES}/pongasoft/VST/GUI/Params/HostParameters.h

    ${VST_COMMON_CPP_SOURCES}/pongasoft/VST/GUI/Views/CustomControlView.h
    ${VST_COMMON_CPP_SOURCES}/pongasoft/VST/GUI/Views/CustomView.h
    ${VST_COMMON_CPP_SOURCES}/pongasoft/VST/GUI/Views/CustomViewCreator.h
    ${VST_COMMON_CPP_SOURCES}/pongasoft/VST/GUI/Views/CustomViewFactory.h
    ${VST_COMMON_CPP_SOURCES}/pongasoft/VST/GUI/Views/MomentaryButtonView.h
    ${VST_COMMON_CPP_SOURCES}/pongasoft/VST/GUI/Views/ToggleButtonView.h

    ${VST_COMMON_CPP_SOURCES}/pongasoft/VST/GUI/DrawContext.h
    ${VST_COMMON_CPP_SOURCES}/pongasoft/VST/GUI/GUIController.h
    ${VST_COMMON_CPP_SOURCES}/pongasoft/VST/GUI/GUIViewState.h
    ${VST_COMMON_CPP_SOURCES}/pongasoft/VST/GUI/Types.h
    )

set(VST_COMMON_sources_cpp
    ${LOGURU_IMPL}

    ${VST_COMMON_CPP_SOURCES}/pongasoft/VST/Parameters.cpp
    ${VST_COMMON_CPP_SOURCES}/pongasoft/VST/NormalizedState.cpp

    ${VST_COMMON_CPP_SOURCES}/pongasoft/VST/RT/RTParameter.cpp
    ${VST_COMMON_CPP_SOURCES}/pongasoft/VST/RT/RTProcessor.cpp
    ${VST_COMMON_CPP_SOURCES}/pongasoft/VST/RT/RTState.cpp

    ${VST_COMMON_CPP_SOURCES}/pongasoft/VST/GUI/Params/GUIParamCxMgr.cpp
    ${VST_COMMON_CPP_SOURCES}/pongasoft/VST/GUI/Params/GUIParameters.cpp
    ${VST_COMMON_CPP_SOURCES}/pongasoft/VST/GUI/Params/GUIParamCxAware.cpp
    ${VST_COMMON_CPP_SOURCES}/pongasoft/VST/GUI/Params/GUIRawParameter.cpp

    ${VST_COMMON_CPP_SOURCES}/pongasoft/VST/GUI/Views/CustomView.cpp
    ${VST_COMMON_CPP_SOURCES}/pongasoft/VST/GUI/Views/MomentaryButtonView.cpp
    ${VST_COMMON_CPP_SOURCES}/pongasoft/VST/GUI/Views/ToggleButtonView.cpp

    ${VST_COMMON_CPP_SOURCES}/pongasoft/VST/GUI/DrawContext.cpp
    ${VST_COMMON_CPP_SOURCES}/pongasoft/VST/GUI/GUIController.cpp

    )

if (SMTG_CREATE_VST2_VERSION)
  set(VST_COMMON_vst2_sources
      ${VST3_SDK_ROOT}/public.sdk/source/common/memorystream.cpp
      ${VST3_SDK_ROOT}/public.sdk/source/vst/hosting/eventlist.cpp
      ${VST3_SDK_ROOT}/public.sdk/source/vst/hosting/hostclasses.cpp
      ${VST3_SDK_ROOT}/public.sdk/source/vst/hosting/parameterchanges.cpp
      ${VST3_SDK_ROOT}/public.sdk/source/vst/hosting/processdata.cpp
      ${VST3_SDK_ROOT}/public.sdk/source/vst/vst2wrapper/vst2wrapper.cpp
      ${VST3_SDK_ROOT}/public.sdk/source/vst/vst2wrapper/vst2wrapper.h
      ${VST3_SDK_ROOT}/public.sdk/source/vst2.x/audioeffect.cpp
      ${VST3_SDK_ROOT}/public.sdk/source/vst2.x/audioeffectx.cpp
      )
endif()

if(VST_COMMON_DEBUG_LOGGING)
  message(STATUS "Enabling debug logging for vst_common framework")
  add_definitions(-DVST_COMMON_DEBUG_LOGGING)
endif()

###################################################
# vst_create_archive - Create archive (.tgz)
###################################################
function(vst_create_archive target plugin_name)
  if(MAC)
    set(ARCHITECTURE "macOS_64bits")
  elseif(WIN)
    set(ARCHITECTURE "win_64bits")
  endif()

  set(ARCHIVE_NAME ${target}-${ARCHITECTURE}-${PLUGIN_VERSION})
  set(ARCHIVE_PATH ${CMAKE_BINARY_DIR}/archive/${ARCHIVE_NAME})

  message(STATUS "Archive path ${ARCHIVE_PATH}.zip")

  add_custom_command(OUTPUT ${ARCHIVE_PATH}
      COMMAND ${CMAKE_COMMAND} -E make_directory ${ARCHIVE_PATH}
      COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_LIST_DIR}/LICENSE.txt ${ARCHIVE_PATH}
      COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_LIST_DIR}/archive/README-${ARCHITECTURE}.txt ${ARCHIVE_PATH}/README.txt
      )

  if(MAC)
    add_custom_command(OUTPUT ${ARCHIVE_PATH}.zip
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${VST3_OUTPUT_DIR}/${target}.${VST3_EXTENSION} ${ARCHIVE_PATH}/${plugin_name}.vst3
        DEPENDS ${target}
        DEPENDS ${ARCHIVE_PATH}
        WORKING_DIRECTORY archive
        )
  elseif(WIN)
    add_custom_command(OUTPUT ${ARCHIVE_PATH}.zip
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${target}> ${ARCHIVE_PATH}/${plugin_name}.vst3
        DEPENDS ${target}
        DEPENDS ${ARCHIVE_PATH}
        WORKING_DIRECTORY archive
        )
  endif()

  if(MAC OR WIN)
    add_custom_command(OUTPUT ${ARCHIVE_PATH}.zip
        COMMAND ${CMAKE_COMMAND} -E tar cvf ${ARCHIVE_NAME}.zip --format=zip ${ARCHIVE_PATH}
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${ARCHIVE_PATH}
        APPEND
        )

    add_custom_target(archive
        DEPENDS ${ARCHIVE_PATH}.zip
        )
  endif()
endfunction()

###################################################
# vst_fix_vst2
###################################################
function(vst_fix_vst2 target)
  if (SMTG_CREATE_VST2_VERSION)
    message(STATUS "${target} will be VST2 compatible")
    if(MAC)
      # fix missing VSTPluginMain symbol when also building VST 2 version
      smtg_set_exported_symbols(${target} "${VST_COMMON_ROOT}/mac/macexport_vst2.exp")
    endif()
    if (WIN)
      add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    endif()
  endif()
endfunction()

###################################################
# vst_add_test - Testing
###################################################
# Download and unpack googletest at configure time
include(${CMAKE_CURRENT_LIST_DIR}/gtest.cmake)
enable_testing()
include(GoogleTest)

function(vst_add_test PROJECT_TEST_NAME TEST_CASES_FILES TEST_SOURCES TEST_LIBS)
  message(STATUS "Adding target ${PROJECT_TEST_NAME} for test cases: ${TEST_CASES_FILES}" )

  add_executable(${PROJECT_TEST_NAME} ${TEST_CASES_FILES} ${TEST_SOURCES})
  target_link_libraries(${PROJECT_TEST_NAME} gtest_main ${TEST_LIBS})
  target_include_directories(${PROJECT_TEST_NAME} PUBLIC ${PROJECT_SOURCE_DIR})
  target_include_directories(${PROJECT_TEST_NAME} PUBLIC ${GTEST_INCLUDE_DIRS})

  gtest_add_tests(
      TARGET      ${PROJECT_TEST_NAME}
      TEST_LIST   ${PROJECT_TEST_NAME}_targets
  )
endfunction()

###################################################
# Testing - for this framework
###################################################
file(GLOB_RECURSE TEST_SRC_FILES RELATIVE ${PROJECT_SOURCE_DIR} vst-common/test/cpp/*cpp)
set(test_sources ""
    )

vst_add_test(vst-common_test "${TEST_SRC_FILES}" "${test_sources}" "")
