add_library(common STATIC argsparser.hpp argsparser.cpp)
target_include_directories(common PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/")