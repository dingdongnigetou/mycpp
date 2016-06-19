project(mycpp_build)

file(GLOB ADO_SRC_LIST ado/*.cpp)
file(GLOB OCI_SRC_LIST ocilib/*.cpp)
file(GLOB MYSQL_SRC_LIST mysql_api/*.cpp)
file(GLOB COMMMON_SRC_LIST common/*.cpp)

add_definitions(-D__LCC__)

if(UNIX)
	set(CMAKE_CXX_COMPLIER "g++")
	set(CMAKE_CXX_FLAGS "-std=c++11 -g -Wall")

	include_directories(../Common/ common/ ado/ ocilib/ ocilib/api/linux/ mysql_api/ mysql_api/api/linux/)
	link_directories(${PROJECT_SOURCE_DIR}/mysql_api/api/linux/)
	link_directories(${PROJECT_SOURCE_DIR}/ocilib/api/linux/)

	add_library(DBAccess SHARED ${OCI_SRC_LIST} ${MYSQL_SRC_LIST} ${COMMMON_SRC_LIST})
	target_link_libraries(DBAccess libocilib.so libmysqlclient.so)

else()
	include_directories(../Common/ common/ ado/ ocilib/ ocilib/api/win/ mysql_api/ mysql_api/api/win/)
	link_directories(${PROJECT_SOURCE_DIR}/mysql_api/api/win/)
	link_directories(${PROJECT_SOURCE_DIR}/ocilib/api/win/)

	add_library(DBAccess SHARED ${ADO_SRC_LIST} ${OCI_SRC_LIST} ${MYSQL_SRC_LIST} ${COMMMON_SRC_LIST})
	target_link_libraries(DBAccess ociliba.lib libmysql.lib)

endif()

