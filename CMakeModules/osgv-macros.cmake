include(CMakeParseArguments)

macro(osgv_setup_sample _NAME)
	cmake_parse_arguments(
        PARSED_ARGS # prefix of output variables
        "" # list of names of the boolean arguments (only defined ones will be true)
        "" # list of names of mono-valued arguments
        "HEADER_FILES;SOURCE_FILES;INCLUDE_DIRS;DEPS;DEFINITIONS" # list of names of multi-valued arguments (output variables are lists)
        ${ARGN} # arguments of the function to parse, here we take the all original ones
    )
	
	add_executable(${_NAME} ${PARSED_ARGS_SOURCE_FILES} ${PARSED_ARGS_HEADER_FILES})

	if(MSVC)
		set(OSG_BIN_DIR $<IF:$<CONFIG:Debug>,${OSG_BIN_DIR_DBG},${OSG_BIN_DIR_REL}>)
		set(OSGPLUGIN_BIN_DIR $<IF:$<CONFIG:Debug>,${OSG_PLUGIN_DIR_DBG},${OSG_PLUGIN_DIR_REL}>)
		set_target_properties(${_NAME} PROPERTIES 
			VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:${_NAME}>"
			VS_DEBUGGER_ENVIRONMENT       "PATH=%PATH%;${OSG_BIN_DIR};${OSGPLUGIN_BIN_DIR}")
	endif()
	
	target_include_directories(${_NAME} PRIVATE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/samples/common>)
	
	foreach(INC_DIR ${PARSED_ARGS_INCLUDE_DIRS})
		target_include_directories(${_NAME} PRIVATE $<BUILD_INTERFACE:${INC_DIR}>)
	endforeach()
	
	target_link_libraries(${_NAME} PRIVATE osgVegetation)
	
	foreach(CUR_DEP ${PARSED_ARGS_DEPS})
		target_link_libraries(${_NAME} PRIVATE ${CUR_DEP})
	endforeach()

	target_compile_definitions(${_NAME} PRIVATE ${PARSED_ARGS_DEFINITIONS})
	set_target_properties(${_NAME} PROPERTIES DEBUG_POSTFIX d)
	set_target_properties(${_NAME} PROPERTIES FOLDER "Samples")

	install(TARGETS ${_NAME})
	
endmacro()