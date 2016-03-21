cmake_minimum_required(VERSION 2.6.0)
project(signal-scope)

set(CATKIN_CALL "${CATKIN_DEVEL_PREFIX}")
if(CATKIN_CALL)
	message(STATUS "Compiling signal scope as a catkin project")
	find_package(catkin REQUIRED)
	add_subdirectory(externals)
	list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
	
	set(PYTHONQT_INCLUDE_DIRS "${CMAKE_INSTALL_PREFIX}/include/PythonQt")
	find_library(PYTHONQT_LIBRARY PythonQt HINTS "${CMAKE_INSTALL_PREFIX}/lib")
	set(PYTHONQT_LIBRARIES ${PYTHONQT_LIBRARY})

	set(CTK_PYTHONCONSOLE_INCLUDE_DIRS "${CMAKE_INSTALL_PREFIX}/include/ctkPythonConsole")
	find_library(CTK_PYTHONCONSOLE_LIBRARY ctkPythonConsole HINTS "${CMAKE_INSTALL_PREFIX}/lib")
	set(CTK_PYTHONCONSOLE_LIBRARIES ${CTK_PYTHONCONSOLE_LIBRARY})
else()
	set(POD_NAME signal_scope)
	include(cmake/pods.cmake)

	list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
	list(APPEND CMAKE_PREFIX_PATH "${CMAKE_INSTALL_PREFIX}")
endif()

find_package(PkgConfig)
pkg_check_modules(DRCLCMTYPES drc-lcmtypes)
set(use_builtin_lcmtypes_default OFF)
if(DRCLCMTYPES_FOUND)
  set(use_builtin_lcmtypes_default ON)
endif()


option(USE_BUILTIN_LCMTYPES "Compile builtin LCM types" ${use_builtin_lcmtypes_default})
option(USE_PYTHON "Enable Python support." ON)

add_subdirectory(src/signal_scope)
