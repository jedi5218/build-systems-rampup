
set(COMMON_INCLUDES
    argsparser.h
    utils.h
)

set(COMMON_SOURCES
    argsparser.cc
    utils.cc
)

add_library(common STATIC ${COMMON_INCLUDES} ${COMMON_SOURCES})
target_include_directories(common PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/..")
