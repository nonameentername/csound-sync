# Utility function to make executables. All plugin targets should use this as it
# sets up output directory set in top-level CmakeLists.txt
# and adds an appropriate install target
#
# name - name of executable to produce
# srcs - list of src files
# libs - list of library files to link to
# output_name (OPTIONAL) - overide the name of the generated executable
#
function(make_executable name srcs libs)
    add_executable(${name} ${srcs})
    target_link_libraries (${name} PRIVATE ${libs})
    set_target_properties(${name} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${BUILD_BIN_DIR})

    if(${ARGC} EQUAL 4)
        set_target_properties(${name} PROPERTIES
            OUTPUT_NAME ${ARGV3})
    endif()
    install(TARGETS ${name}
	RUNTIME DESTINATION "${EXECUTABLE_INSTALL_DIR}" )
endfunction()

# Utility function to make a utility executable
#
# name - name of executable to produce
# srcs - list of src files
# libraries - OPTIONAL extra libraries to link
#
function(make_utility name srcs)
    make_executable(${name} "${srcs}" "${CSOUNDLIB}")
    add_dependencies(${name} ${CSOUNDLIB})
    set(i 2)
    while( ${i} LESS ${ARGC} )
        target_link_libraries(${name} PRIVATE ${ARGV${i}})
        math(EXPR i "${i}+1")
    endwhile()
endfunction()

# Checks if dependencies for an enabled target are fulfilled.
# If FAIL_MISSING is true and the dependencies are not fulfilled,
# it will abort the cmake run.
# If FAIL_MISSING is false, it will set the option to OFF.
# If the target is not enabled, it will do nothing.
# example: check_deps(BUILD_NEW_PARSER FLEX_EXECUTABLE
# BISON_EXECUTABLE)
#
function(check_deps option)
    if(${option})
        set(i 1)
        while( ${i} LESS ${ARGC} )
            set(dep ${ARGV${i}})
            if(NOT ${dep})
                if(FAIL_MISSING)
                    message(FATAL_ERROR
                        "${option} is enabled, but ${dep}=\"${${dep}}\"")
                else()
                    message(STATUS "${dep}=\"${${dep}}\", so disabling ${option}")
                    set(${option} OFF PARENT_SCOPE)
                    # Set it in the local scope too
                    set(${option} OFF)
                endif()
            endif()
            math(EXPR i "${i}+1")
        endwhile()
    endif()
    if(${option})
        message(STATUS "${option} is enabled.")
    else()
        message(STATUS "${option} is disabled.")
    endif()
endfunction()

# Utility function to make plugins. All plugin targets should use this as it
# sets up output directory set in top-level CmakeLists.txt
# and adds the appropriate install target
#
# libname - name of library to produce
# srcs - list of src files (must be quoted if a list)
# extralibs (OPTIONAL) - extra libraries to link the plugin to
#
# NB - this was moved here as it needs some VARS defined above
# for setting up the framework
function(make_plugin libname srcs)
    if(APPLE)
        add_library(${libname} SHARED ${srcs})
    else()
        add_library(${libname} MODULE ${srcs})
    endif()

    set(i 2)
    while( ${i} LESS ${ARGC} )
        target_link_libraries(${libname} PRIVATE ${ARGV${i}})
        math(EXPR i "${i}+1")
    endwhile()

    set_target_properties(${libname} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${BUILD_PLUGINS_DIR}
        LIBRARY_OUTPUT_DIRECTORY ${BUILD_PLUGINS_DIR}
        ARCHIVE_OUTPUT_DIRECTORY ${BUILD_PLUGINS_DIR})

    install(TARGETS ${libname}
        LIBRARY DESTINATION "${PLUGIN_INSTALL_DIR}"
        ARCHIVE DESTINATION "${PLUGIN_INSTALL_DIR}" )
endfunction()
