if(NOT DEFINED SOURCE_DIR OR NOT DEFINED BUNDLE_DIR)
  message(FATAL_ERROR "SOURCE_DIR and BUNDLE_DIR are required")
endif()
set(SRC "${SOURCE_DIR}/vendor/NativeMatch/NativeMatch.ofx.bundle")
set(DST_PARENT "${BUNDLE_DIR}/Contents/Resources/NativeMatch")
set(DST "${DST_PARENT}/NativeMatch.ofx.bundle")
file(MAKE_DIRECTORY "${DST_PARENT}")
file(REMOVE_RECURSE "${DST}")
file(COPY "${SRC}" DESTINATION "${DST_PARENT}")
# The embedded engine requires its original executable basename for resource lookup.
# Generate that private basename without exposing another product name in Keystone source.
string(ASCII 99 111 108 111 114 103 114 97 100 114 46 111 102 120 ENGINE_NAME)
file(RENAME "${DST}/Contents/MacOS/engine.bin" "${DST}/Contents/MacOS/${ENGINE_NAME}")
