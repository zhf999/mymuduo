cmake_minimum_required(VERSION 3.28)
project(mymuduo)

set(CMAKE_CXX_STANDARD 20)
include(GNUInstallDirs)
# set library os path
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib) # dynamic
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib) # static

file(GLOB_RECURSE SOURCE CONFIGURE_DEPENDS "src/*.cpp")

add_library(mymuduo SHARED ${SOURCE})

target_include_directories(mymuduo PUBLIC ${PROJECT_SOURCE_DIR}/include)

install(
	TARGETS mymuduo
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(
	DIRECTORY ${PROJECT_SOURCE_DIR}/include/
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/mymuduo
	FILES_MATCHING PATTERN "*.h"
)

add_subdirectory(tests)