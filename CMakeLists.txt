cmake_minimum_required(VERSION 2.6.0)
project(oscilloscope)

set(CATKIN_CALL "${CATKIN_DEVEL_PREFIX}")
if(CATKIN_CALL)
	message(STATUS "Compiling signal scope as a catkin project")
	catkin_package()
	find_package(catkin REQUIRED)
	list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
else()
	list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
endif()	

set(POD_NAME signal_scope)
include(cmake/pods.cmake)


list(APPEND CMAKE_PREFIX_PATH "${CMAKE_INSTALL_PREFIX}")


find_package(PkgConfig)
pkg_check_modules(DRCLCMTYPES drc-lcmtypes)
set(use_builtin_lcmtypes_default OFF)
if(DRCLCMTYPES_FOUND)
  set(use_builtin_lcmtypes_default ON)
endif()


option(USE_BUILTIN_LCMTYPES "Compile builtin LCM types" ${use_builtin_lcmtypes_default})
option(USE_PYTHON "Enable Python support." ON)

add_subdirectory(src/signal_scope)

