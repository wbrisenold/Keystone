cmake_minimum_required(VERSION 3.20)
project(KeystoneOFX LANGUAGES C CXX OBJCXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
if(APPLE)
  set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "Build universal macOS binary" FORCE)
  set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "Minimum macOS version" FORCE)
endif()
add_library(KeystoneOFX MODULE
  src/KeystoneOFX.cpp src/KeystoneCPU.cpp src/NeutralAnalyzer.cpp src/LutLoader.cpp src/MetalBridge.mm src/ColorGradrBridge.cpp)
target_include_directories(KeystoneOFX PRIVATE src generated)
target_compile_options(KeystoneOFX PRIVATE
  $<$<COMPILE_LANGUAGE:CXX>:-Wall -Wextra -Wpedantic -Werror=return-type>
  $<$<COMPILE_LANGUAGE:OBJCXX>:-Wall -Wextra -Wpedantic -Werror=return-type>)
if(APPLE)
  target_link_libraries(KeystoneOFX PRIVATE "-framework Foundation" "-framework Metal" "-framework QuartzCore")
  set_target_properties(KeystoneOFX PROPERTIES PREFIX "" SUFFIX ".ofx" BUNDLE TRUE BUNDLE_EXTENSION "ofx.bundle" MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/src/Info.plist.in)
  add_custom_command(TARGET KeystoneOFX POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_BUNDLE_DIR:KeystoneOFX>/Contents/Resources/ColorGradr"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
      "${CMAKE_CURRENT_SOURCE_DIR}/vendor/ColorGradr/colorgradr.ofx.bundle"
      "$<TARGET_BUNDLE_DIR:KeystoneOFX>/Contents/Resources/ColorGradr/colorgradr.ofx.bundle")
else()
  set_target_properties(KeystoneOFX PROPERTIES PREFIX "" SUFFIX ".ofx")
endif()
add_executable(keystone_model_tests tests/keystone_model_tests.cpp src/KeystoneCPU.cpp src/NeutralAnalyzer.cpp src/LutLoader.cpp)
target_include_directories(keystone_model_tests PRIVATE src generated)
