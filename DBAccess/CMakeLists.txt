project(mycpp_build)

# include ado or not
if(UNIX)
	message("system is UNIX.")  
else()
	file(GLOB ADO_SRC_LIST ado/*.cpp)
	file(GLOB OCI_SRC_LIST ocilib/*.cpp)
	file(GLOB MYSQL_SRC_LIST mysql_api/*.cpp)
	file(GLOB COMMMON_SRC_LIST common/*.cpp)

	include_directories(../Common/ common/ ado/ ocilib/ ocilib/api/win/ mysql_api/ mysql_api/api/win/)

	add_definitions(-D__LCC__ -DBACCESS_EXPORTS)
	
	link_directories(mysql_api/api/win/)
	link_directories(ocilib/api/win/)

	add_library(DBAccess SHARED ${ADO_SRC_LIST} ${OCI_SRC_LIST} ${MYSQL_SRC_LIST} ${COMMMON_SRC_LIST})

	target_link_libraries(DBAccess ociliba.lib libmysql.lib)

endif()

