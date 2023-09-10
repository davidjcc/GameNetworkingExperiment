
# Raylib
set(RAYLIB_VERSION 4.5.0)
find_package(raylib ${RAYLIB_VERSION} QUIET) # QUIET or REQUIRED
if (NOT raylib_FOUND) # If there's none, fetch and build raylib
  include(FetchContent)
  FetchContent_Declare(
    raylib
    URL https://github.com/raysan5/raylib/archive/refs/tags/${RAYLIB_VERSION}.tar.gz
  )
  FetchContent_GetProperties(raylib)
  if (NOT raylib_POPULATED) # Have we downloaded raylib yet?
    set(FETCHCONTENT_QUIET NO)
    FetchContent_Populate(raylib)
    set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE) # don't build the supplied examples
    add_subdirectory(${raylib_SOURCE_DIR} ${raylib_BINARY_DIR})
  endif()
endif()

set(RAYGUI_VERSION 3.6)
find_package(raygui ${RAYGUI_VERSION} QUIET) 
if (NOT raygui_FOUND) 
  include(FetchContent)
  FetchContent_Declare(
    raygui
    URL https://github.com/raysan5/raygui/archive/refs/tags/${RAYGUI_VERSION}.tar.gz
  )
  FetchContent_GetProperties(raylib)
  if (NOT raygui_POPULATED) 
    set(FETCHCONTENT_QUIET NO)
    FetchContent_Populate(raygui)
  endif()
endif()

