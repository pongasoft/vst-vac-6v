#-------------------------------------------------------------------------------
# Including VST3 SDK
#-------------------------------------------------------------------------------

include(${CMAKE_CURRENT_LIST_DIR}/VST3_SDK.cmake)

#-------------------------------------------------------------------------------
# Defining files to include to generate the library
#-------------------------------------------------------------------------------

set(VST_COMMON_CPP_SOURCES ${CMAKE_CURRENT_LIST_DIR}/src/cpp)
set(LOGURU_IMPL ${VST_COMMON_CPP_SOURCES}/pongasoft/logging/logging.cpp)
include_directories(${VST_COMMON_CPP_SOURCES})

set(VST_COMMON_SRC_COMMON_DIR ${VST_COMMON_CPP_SOURCES}/pongasoft/VST/Common)
set(VST_COMMON_SRC_GUI_DIR ${VST_COMMON_CPP_SOURCES}/pongasoft/VST/GUI)
set(VST_COMMON_SRC_RT_DIR ${VST_COMMON_CPP_SOURCES}/pongasoft/VST/RT)

set(VST_COMMON_sources_h
    ${VST_COMMON_CPP_SOURCES}/pongasoft/logging/loguru.hpp

    ${VST_COMMON_CPP_SOURCES}/pongasoft/Utils/Collection/CircularBuffer.h
    ${VST_COMMON_CPP_SOURCES}/pongasoft/Utils/Concurrent/Concurrent.h
    ${VST_COMMON_CPP_SOURCES}/pongasoft/Utils/Misc.h

    ${VST_COMMON_CPP_SOURCES}/pongasoft/VST/Plugin.h
    ${VST_COMMON_CPP_SOURCES}/pongasoft/VST/ParamConverters.h
    ${VST_COMMON_CPP_SOURCES}/pongasoft/VST/Parameter.h

    )

set(VST_COMMON_sources_cpp
    ${LOGURU_IMPL}

    ${VST_COMMON_CPP_SOURCES}/pongasoft/VST/Plugin.cpp
    )

add_library(pongasoft-vst-common STATIC ${VST_COMMON_sources_h} ${VST_COMMON_sources_cpp})
target_include_directories(pongasoft-vst-common PUBLIC ${VST3_SDK_ROOT})

###################################################
# Testing
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
# Testing
###################################################
file(GLOB_RECURSE TEST_SRC_FILES RELATIVE ${PROJECT_SOURCE_DIR} vst-common/test/cpp/*cpp)
set(test_sources ""
    )

vst_add_test(vst-common_test "${TEST_SRC_FILES}" "${test_sources}" "")
