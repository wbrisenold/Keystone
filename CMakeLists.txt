cmake_minimum_required(VERSION 3.20)
project(KeystoneOFX LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(APPLE)
  enable_language(OBJCXX)
  # Set these BEFORE ncnn/sidecar targets are created. Target architecture is initialized at
  # creation time, so doing this later can leave an optional sidecar arm64-only on an arm64 runner.
  set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "Build universal macOS binary" FORCE)
  set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "Minimum macOS version" FORCE)
endif()

option(KEYSTONE_SCENE_ENGINE "Build the optional semantic Scene Grade sidecar" ON)

# IMPORTANT: the inference runtime is never linked into KeystoneOFX itself.
# Resolve must be able to enumerate/load Keystone even when the optional scene engine fails.
# The sidecar is dlopen()'d only when Analyze Scene is pressed.
if(APPLE AND KEYSTONE_SCENE_ENGINE)
  include(FetchContent)
  set(NCNN_VULKAN OFF CACHE BOOL "" FORCE)
  set(NCNN_BUILD_BENCHMARK OFF CACHE BOOL "" FORCE)
  set(NCNN_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
  set(NCNN_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
  set(NCNN_BUILD_TESTS OFF CACHE BOOL "" FORCE)
  set(NCNN_OPENMP OFF CACHE BOOL "" FORCE)
  set(NCNN_SHARED_LIB OFF CACHE BOOL "" FORCE)
  set(NCNN_SIMPLEOCV OFF CACHE BOOL "" FORCE)
  FetchContent_Declare(ncnn
    GIT_REPOSITORY https://github.com/Tencent/ncnn.git
    GIT_TAG 5e66f094bf7c597b4569cc014a8be84104748678)
  FetchContent_MakeAvailable(ncnn)

  add_library(KeystoneSceneEngine SHARED src/SceneEngine.cpp)
  target_link_libraries(KeystoneSceneEngine PRIVATE ncnn)
  target_compile_features(KeystoneSceneEngine PRIVATE cxx_std_17)
  target_compile_options(KeystoneSceneEngine PRIVATE -Wall -Wextra -Wpedantic -Werror=return-type)
  set_target_properties(KeystoneSceneEngine PROPERTIES
    PREFIX ""
    OUTPUT_NAME "KeystoneSceneEngine"
    SUFFIX ".dylib")
endif()
set(KEYSTONE_OFX_SRCS
  src/KeystoneOFX.cpp
  src/KeystoneCPU.cpp
  src/NeutralAnalyzer.cpp
  src/LutLoader.cpp
  src/ColorGradrBridge.cpp
  src/SceneGrade.cpp)
if(APPLE)
  list(APPEND KEYSTONE_OFX_SRCS src/MetalBridge.mm)
endif()
add_library(KeystoneOFX MODULE ${KEYSTONE_OFX_SRCS})
target_include_directories(KeystoneOFX PRIVATE src generated)
target_compile_options(KeystoneOFX PRIVATE
  $<$<COMPILE_LANGUAGE:CXX>:-Wall -Wextra -Wpedantic -Werror=return-type>
  $<$<COMPILE_LANGUAGE:OBJCXX>:-Wall -Wextra -Wpedantic -Werror=return-type>)
if(APPLE)
  target_link_libraries(KeystoneOFX PRIVATE "-framework Foundation" "-framework Metal" "-framework QuartzCore")
  if(TARGET KeystoneSceneEngine)
    add_dependencies(KeystoneOFX KeystoneSceneEngine)
  endif()
  set_target_properties(KeystoneOFX PROPERTIES PREFIX "" SUFFIX ".ofx" BUNDLE TRUE BUNDLE_EXTENSION "ofx.bundle" MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/src/Info.plist.in)
  add_custom_command(TARGET KeystoneOFX POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_BUNDLE_DIR:KeystoneOFX>/Contents/Resources/ColorGradr"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
      "${CMAKE_CURRENT_SOURCE_DIR}/vendor/ColorGradr/colorgradr.ofx.bundle"
      "$<TARGET_BUNDLE_DIR:KeystoneOFX>/Contents/Resources/ColorGradr/colorgradr.ofx.bundle"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
      "${CMAKE_CURRENT_SOURCE_DIR}/resources/SceneModel"
      "$<TARGET_BUNDLE_DIR:KeystoneOFX>/Contents/Resources/SceneModel"
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_BUNDLE_DIR:KeystoneOFX>/Contents/Resources/Licenses"
    COMMAND ${CMAKE_COMMAND} -E copy
      "${CMAKE_CURRENT_SOURCE_DIR}/vendor/licenses/ADE20K-model-Apache-2.0.txt"
      "${CMAKE_CURRENT_SOURCE_DIR}/vendor/licenses/ncnn-BSD-3-Clause.txt"
      "$<TARGET_BUNDLE_DIR:KeystoneOFX>/Contents/Resources/Licenses/")
  if(TARGET KeystoneSceneEngine)
    add_custom_command(TARGET KeystoneOFX POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:KeystoneSceneEngine>"
        "$<TARGET_BUNDLE_DIR:KeystoneOFX>/Contents/Resources/KeystoneSceneEngine.dylib")
  endif()
else()
  set_target_properties(KeystoneOFX PROPERTIES PREFIX "" SUFFIX ".ofx")
endif()
add_executable(keystone_model_tests tests/keystone_model_tests.cpp src/KeystoneCPU.cpp src/NeutralAnalyzer.cpp src/LutLoader.cpp)
target_include_directories(keystone_model_tests PRIVATE src generated)
