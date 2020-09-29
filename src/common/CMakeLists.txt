add_library(common STATIC argsparser.h argsparser.cc)
target_include_directories(common PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/")