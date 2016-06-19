PROJECT (mycpp_build)

if (UNIX)
	set(CMAKE_CXX_COMPLIER "g++")
	set(CMAKE_CXX_FLAGS "-std=c++11 -g -Wall")
else ()
endif()

aux_source_directory(. SRC_LIST)
file(GLOB INCLUDE_LIST ./*.h**)
ADD_EXECUTABLE(demo ${SRC_LIST} ${INCLUDE_LIST})

