find_package(Asio QUIET)
if (NOT Asio_FOUND)
    message("WARNING: Boost asio not found, not building ArduPilot plugin.")
    return()
endif()

#--------------------------------------------------------
# Library Creation
#--------------------------------------------------------
SET (LIBRARY_NAME ArduPilot_plugin)
SET (LIB_MAJOR 0)
SET (LIB_MINOR 0)
SET (LIB_RELEASE 1)

file(GLOB SRCS *.cpp)

#FOR Debugging:
#TODO: comment out before committing.
add_definitions (
    -Wfatal-errors
)

include_directories(${Asio_INCLUDE_DIR})

ADD_LIBRARY(${LIBRARY_NAME} SHARED
  ${SRCS}
  )

TARGET_LINK_LIBRARIES(${LIBRARY_NAME}
  Multirotor_plugin
  )

SET (_soversion ${LIB_MAJOR}.${LIB_MINOR}.${LIB_RELEASE})

set_target_properties(${LIBRARY_NAME} PROPERTIES
  SOVERSION ${LIB_MAJOR}
  VERSION ${_soversion}
  LIBRARY_OUTPUT_DIRECTORY ${PROJECT_PLUGIN_LIBS_DIR}
  )

install(TARGETS ${LIBRARY_NAME}
  # IMPORTANT: Add the library to the "export-set"
  EXPORT ${PROJECT_NAME}-targets
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib/${PROJECT_NAME}/plugin_libs
)

# Push up the PROJECT_PLUGINS variable
set(PROJECT_PLUGINS ${PROJECT_PLUGINS} ${LIBRARY_NAME} PARENT_SCOPE)
