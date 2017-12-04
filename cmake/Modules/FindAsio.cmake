#Find Asio libraries:
#
# Web site: http://think-async.com/Asio
#
# libasio-dev package on Ubuntu has these.
#
# Once done, the following will be defined
#
#  Asio_FOUND - bool indicating if ASIO was found
#  Asio_INCLUDE_DIR - the asio include directory

include(LibFindMacros)
include(MacroCommonPaths)

MacroCommonPaths(Asio asio)

find_path(Asio_INCLUDE_DIR
    NAMES is_read_buffered.hpp
    PATHS ${COMMON_INCLUDE_PATHS_Asio}
)

if(Asio_INCLUDE_DIR) 
    set(Asio_FOUND 1)
endif()
