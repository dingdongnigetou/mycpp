PROJECT (mycpp_cmake_test)
aux_source_directory(. SRC_LIST)
file(GLOB INCLUDE_LIST ./*.h**)
ADD_EXECUTABLE(demo ${SRC_LIST} ${INCLUDE_LIST})
