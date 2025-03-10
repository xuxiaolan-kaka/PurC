# This file is for macros that are used by multiple projects. If your macro is
# exclusively needed in only one subdirectory of Source (e.g. only needed by
# PurC), then put it there instead.

macro(PURC_COMPUTE_SOURCES _framework)
    set(_derivedSourcesPath ${${_framework}_DERIVED_SOURCES_DIR})

    foreach (_sourcesListFile IN LISTS ${_framework}_UNIFIED_SOURCE_LIST_FILES)
      configure_file("${CMAKE_CURRENT_SOURCE_DIR}/${_sourcesListFile}" "${_derivedSourcesPath}/${_sourcesListFile}" COPYONLY)
      message(STATUS "Using source list file: ${_sourcesListFile}")

      list(APPEND _sourceListFileTruePaths "${CMAKE_CURRENT_SOURCE_DIR}/${_sourcesListFile}")
    endforeach ()

    if (ENABLE_UNIFIED_BUILDS)
        execute_process(COMMAND ${RUBY_EXECUTABLE} ${WTF_SCRIPTS_DIR}/generate-unified-source-bundles.rb
            "--derived-sources-path" "${_derivedSourcesPath}"
            "--source-tree-path" ${CMAKE_CURRENT_SOURCE_DIR}
            "--print-bundled-sources"
            ${_sourceListFileTruePaths}
            RESULT_VARIABLE _resultTmp
            OUTPUT_VARIABLE _outputTmp)

        if (${_resultTmp})
             message(FATAL_ERROR "generate-unified-source-bundles.rb exited with non-zero status, exiting")
        endif ()

        foreach (_sourceFileTmp IN LISTS _outputTmp)
            set_source_files_properties(${_sourceFileTmp} PROPERTIES HEADER_FILE_ONLY ON)
            list(APPEND ${_framework}_HEADERS ${_sourceFileTmp})
        endforeach ()
        unset(_sourceFileTmp)

        execute_process(COMMAND ${RUBY_EXECUTABLE} ${WTF_SCRIPTS_DIR}/generate-unified-source-bundles.rb
            "--derived-sources-path" "${_derivedSourcesPath}"
            "--source-tree-path" ${CMAKE_CURRENT_SOURCE_DIR}
            ${_sourceListFileTruePaths}
            RESULT_VARIABLE  _resultTmp
            OUTPUT_VARIABLE _outputTmp)

        if (${_resultTmp})
            message(FATAL_ERROR "generate-unified-source-bundles.rb exited with non-zero status, exiting")
        endif ()

        list(APPEND ${_framework}_SOURCES ${_outputTmp})
        unset(_resultTmp)
        unset(_outputTmp)
    else ()
        execute_process(COMMAND ${RUBY_EXECUTABLE} ${WTF_SCRIPTS_DIR}/generate-unified-source-bundles.rb
            "--derived-sources-path" "${_derivedSourcesPath}"
            "--source-tree-path" ${CMAKE_CURRENT_SOURCE_DIR}
            "--print-all-sources"
            ${_sourceListFileTruePaths}
            RESULT_VARIABLE _resultTmp
            OUTPUT_VARIABLE _outputTmp)

        if (${_resultTmp})
             message(FATAL_ERROR "generate-unified-source-bundles.rb exited with non-zero status, exiting")
        endif ()

        list(APPEND ${_framework}_SOURCES ${_outputTmp})
        unset(_resultTmp)
        unset(_outputTmp)
    endif ()
endmacro()

macro(PURC_INCLUDE_CONFIG_FILES_IF_EXISTS)
    set(_file ${CMAKE_CURRENT_SOURCE_DIR}/Platform${PORT}.cmake)
    if (EXISTS ${_file})
        message(STATUS "Using platform-specific CMakeLists: ${_file}")
        include(${_file})
    else ()
        message(STATUS "Platform-specific CMakeLists not found: ${_file}")
    endif ()
endmacro()

# Append the given dependencies to the source file
macro(PURC_ADD_SOURCE_DEPENDENCIES _source _deps)
    set(_tmp)
    get_source_file_property(_tmp ${_source} OBJECT_DEPENDS)
    if (NOT _tmp)
        set(_tmp "")
    endif ()

    foreach (f ${_deps})
        list(APPEND _tmp "${f}")
    endforeach ()

    set_source_files_properties(${_source} PROPERTIES OBJECT_DEPENDS "${_tmp}")
    unset(_tmp)
endmacro()

macro(PURC_ADD_PRECOMPILED_HEADER _header _cpp _source)
    if (MSVC)
        get_filename_component(PrecompiledBasename ${_cpp} NAME_WE)
        file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${_source}")
        set(PrecompiledBinary "${CMAKE_CURRENT_BINARY_DIR}/${_source}/${PrecompiledBasename}.pch")
        set(_sources ${${_source}})

        # clang-cl requires /FI with /Yc
        if (COMPILER_IS_CLANG_CL)
            set_source_files_properties(${_cpp}
                PROPERTIES COMPILE_FLAGS "/Yc\"${_header}\" /Fp\"${PrecompiledBinary}\" /FI\"${_header}\""
                OBJECT_OUTPUTS "${PrecompiledBinary}")
        else ()
            set_source_files_properties(${_cpp}
                PROPERTIES COMPILE_FLAGS "/Yc\"${_header}\" /Fp\"${PrecompiledBinary}\""
                OBJECT_OUTPUTS "${PrecompiledBinary}")
        endif ()
        set_source_files_properties(${_sources}
            PROPERTIES COMPILE_FLAGS "/Yu\"${_header}\" /FI\"${_header}\" /Fp\"${PrecompiledBinary}\"")

        foreach (_src ${_sources})
            PURC_ADD_SOURCE_DEPENDENCIES(${_src} ${PrecompiledBinary})
        endforeach ()

        list(APPEND ${_source} ${_cpp})
    endif ()
    #FIXME: Add support for Xcode.
endmacro()

macro(PURC_WRAP_SOURCELIST)
    foreach (_file ${ARGN})
        get_filename_component(_basename ${_file} NAME_WE)
        get_filename_component(_path ${_file} PATH)

        if (NOT _file MATCHES "${DERIVED_SOURCES_PURC_DIR}")
            string(REGEX REPLACE "/" "\\\\\\\\" _sourcegroup "${_path}")
            source_group("${_sourcegroup}" FILES ${_file})
        endif ()
    endforeach ()

    source_group("DerivedSources" REGULAR_EXPRESSION "${DERIVED_SOURCES_PURC_DIR}")
endmacro()

macro(PURC_FRAMEWORK_DECLARE _target)
    # add_library() without any source files triggers CMake warning
    # Addition of dummy "source" file does not result in any changes in generated build.ninja file
    add_library(${_target} ${${_target}_LIBRARY_TYPE} "${CMAKE_BINARY_DIR}/cmakeconfig.h")
endmacro()

macro(PURC_EXECUTABLE_DECLARE _target)
    add_executable(${_target} "${CMAKE_BINARY_DIR}/cmakeconfig.h")
endmacro()

macro(PURC_PARSER_GEN_IN _rel _name _auxilary)
    set(_src_path ${CMAKE_CURRENT_SOURCE_DIR}/${_rel})
    set(_dst_path ${CMAKE_CURRENT_BINARY_DIR}/${_rel})
    set(_fullname ${_name}${_auxilary})
    set(_src_name ${_src_path}/${_name})
    set(_dst_name ${_dst_path}/${_fullname})
    set(_y        ${_src_name}.y)
    set(_l        ${_src_name}.l)
    set(_tab_c ${_src_path}/${_fullname}_tab.c)
    set(_lex_c ${_src_path}/${_fullname}_lex.c)
    set(_parser ${_fullname}_parser)
    set(_scanner ${_fullname}_scanner)

    BISON_TARGET(${_parser} ${_y} ${_dst_name}.tab.c
        COMPILE_FLAGS "--warnings=error -Dapi.prefix={${_fullname}_yy}")
    FLEX_TARGET(${_scanner} ${_l} ${_dst_name}.lex.c
        COMPILE_FLAGS "--header-file=${_dst_name}.lex.h --prefix=${_fullname}_yy")
    ADD_FLEX_BISON_DEPENDENCY(${_scanner} ${_parser})

    set(_bison_outputs ${BISON_${_fullname}_parser_OUTPUTS})
    set(_flex_outputs ${FLEX_${_fullname}_scanner_OUTPUTS})
    set(_outputs ${_bison_outputs} ${_flex_outputs})

    set_source_files_properties(${_tab_c}
            PROPERTY COMPILE_FLAGS "-I${_dst_path}")
    set_source_files_properties(${_tab_c}
            PROPERTY OBJECT_DEPENDS "${_outputs}")

    unset(_bison_outputs)
    unset(_flex_outputs)
    unset(_outputs)

    unset(_src_path)
    unset(_dst_path)
    unset(_fullname)
    unset(_src_name)
    unset(_dst_name)
    unset(_y)
    unset(_l)
    unset(_tab_c)
    unset(_lex_c)
    unset(_parser)
    unset(_scanner)
endmacro()

macro(PURC_PARSERS_GEN_IN _rel _names _auxilaries)
    add_subdirectory(${_rel}) # you shall add an empty CMakeLists.txt in <path-to-relpath>
    set(_arr_names "${ARGV1}")
    foreach (_name IN LISTS _arr_names)
        PURC_PARSER_GEN_IN(${_rel} ${_name} "")
        set(_arr_auxilaries "${ARGV2}")
        foreach (_auxilary IN LISTS _arr_auxilaries)
            PURC_PARSER_GEN_IN(${_rel} ${_name} ${_auxilary})
        endforeach()
        unset(_arr_auxilaries)
    endforeach()
    unset(_arr_names)
endmacro()

# Private macro for setting the properties of a target.
# Rather than just having _target like PURC_FRAMEWORK and PURC_EXECUTABLE the parameters are
# split into _target_logical_name, which is used for variable expansion, and _target_cmake_name.
# This is done to support PURC_WRAP_EXECUTABLE which uses the _target_logical_name variables
# but requires a different _target_cmake_name.
macro(_PURC_TARGET _target_logical_name _target_cmake_name)
    target_sources(${_target_cmake_name} PRIVATE
        ${${_target_logical_name}_HEADERS}
        ${${_target_logical_name}_SOURCES}
    )
    target_include_directories(${_target_cmake_name} PUBLIC "$<BUILD_INTERFACE:${${_target_logical_name}_INCLUDE_DIRECTORIES}>")
    target_include_directories(${_target_cmake_name} SYSTEM PRIVATE "$<BUILD_INTERFACE:${${_target_logical_name}_SYSTEM_INCLUDE_DIRECTORIES}>")
    target_include_directories(${_target_cmake_name} PRIVATE "$<BUILD_INTERFACE:${${_target_logical_name}_PRIVATE_INCLUDE_DIRECTORIES}>")

    target_compile_definitions(${_target_cmake_name} PRIVATE "BUILDING_${_target_logical_name}")
    if (${_target_logical_name}_DEFINITIONS)
        target_compile_definitions(${_target_cmake_name} PUBLIC ${${_target_logical_name}_DEFINITIONS})
    endif ()
    if (${_target_logical_name}_PRIVATE_DEFINITIONS)
        target_compile_definitions(${_target_cmake_name} PRIVATE ${${_target_logical_name}_PRIVATE_DEFINITIONS})
    endif ()

    if (${_target_logical_name}_LIBRARIES)
        target_link_libraries(${_target_cmake_name} PUBLIC ${${_target_logical_name}_LIBRARIES})
    endif ()
    if (${_target_logical_name}_PRIVATE_LIBRARIES)
        target_link_libraries(${_target_cmake_name} PRIVATE ${${_target_logical_name}_PRIVATE_LIBRARIES})
    endif ()

    if (${_target_logical_name}_DEPENDENCIES)
        add_dependencies(${_target_cmake_name} ${${_target_logical_name}_DEPENDENCIES})
    endif ()
endmacro()

macro(_PURC_TARGET_ANALYZE _target)
    if (ClangTidy_EXE)
        set(_clang_path_and_options
            ${ClangTidy_EXE}
            # Include all non system headers
            --header-filter=.*
        )
        set_target_properties(${_target} PROPERTIES
            C_CLANG_TIDY "${_clang_path_and_options}"
            CXX_CLANG_TIDY "${_clang_path_and_options}"
        )
    endif ()

    if (IWYU_EXE)
        set(_iwyu_path_and_options
            ${IWYU_EXE}
            # Suggests the more concise syntax introduced in C++17
            -Xiwyu --cxx17ns
            # Tells iwyu to always keep these includes
            -Xiwyu --keep=**/config.h
        )
        if (MSVC)
            list(APPEND _iwyu_path_and_options --driver-mode=cl)
        endif ()
        set_target_properties(${_target} PROPERTIES
            CXX_INCLUDE_WHAT_YOU_USE "${_iwyu_path_and_options}"
        )
    endif ()
endmacro()

macro(PURC_FRAMEWORK _target)
    _PURC_TARGET(${_target} ${_target})
    _PURC_TARGET_ANALYZE(${_target})

    if (${_target}_OUTPUT_NAME)
        set_target_properties(${_target} PROPERTIES OUTPUT_NAME ${${_target}_OUTPUT_NAME})
    endif ()

    if (${_target}_PRE_BUILD_COMMAND)
        add_custom_target(_${_target}_PreBuild COMMAND ${${_target}_PRE_BUILD_COMMAND} VERBATIM)
        add_dependencies(${_target} _${_target}_PreBuild)
    endif ()

    if (${_target}_POST_BUILD_COMMAND)
        add_custom_command(TARGET ${_target} POST_BUILD COMMAND ${${_target}_POST_BUILD_COMMAND} VERBATIM)
    endif ()

    if (APPLE AND NOT PORT STREQUAL "GTK" AND NOT ${${_target}_LIBRARY_TYPE} MATCHES STATIC)
        # set_target_properties(${_target} PROPERTIES FRAMEWORK TRUE)
        install(TARGETS ${_target} FRAMEWORK DESTINATION ${LIB_INSTALL_DIR})
    endif ()
endmacro()

# FIXME Move into PURC_FRAMEWORK after all libraries are using this macro
macro(PURC_FRAMEWORK_TARGET _target)
    add_library(${_target}_PostBuild INTERFACE)
    target_link_libraries(${_target}_PostBuild INTERFACE ${${_target}_INTERFACE_LIBRARIES})
    target_include_directories(${_target}_PostBuild INTERFACE ${${_target}_INTERFACE_INCLUDE_DIRECTORIES})
    add_dependencies(${_target}_PostBuild ${${_target}_INTERFACE_DEPENDENCIES})
    add_library(PurC::${_target} ALIAS ${_target}_PostBuild)
endmacro()

macro(PURC_EXECUTABLE _target)
    _PURC_TARGET(${_target} ${_target})
    _PURC_TARGET_ANALYZE(${_target})

    if (${_target}_OUTPUT_NAME)
        set_target_properties(${_target} PROPERTIES OUTPUT_NAME ${${_target}_OUTPUT_NAME})
    endif ()
    if (WIN32)
        if (WTF_CPU_X86)
            set(_processor_architecture "x86")
        elseif (WTF_CPU_X86_64)
            set(_processor_architecture "amd64")
        else ()
            set(_processor_architecture "*")
        endif ()
        target_link_options(${_target} PRIVATE "/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='${_processor_architecture}' publicKeyToken='6595b64144ccf1df' language='*'")
    endif ()
endmacro()

macro(PURC_WRAP_EXECUTABLE _target)
    set(oneValueArgs TARGET_NAME)
    set(multiValueArgs SOURCES LIBRARIES)
    cmake_parse_arguments(opt "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if (opt_TARGET_NAME)
        set(_wrapped_target_name ${opt_TARGET_NAME})
    else ()
        set(_wrapped_target_name ${_target}Lib)
    endif ()

    add_library(${_wrapped_target_name} SHARED "${CMAKE_BINARY_DIR}/cmakeconfig.h")

    _PURC_TARGET(${_target} ${_wrapped_target_name})
    _PURC_TARGET_ANALYZE(${_wrapped_target_name})

    # Unset values
    unset(${_target}_HEADERS)
    unset(${_target}_DEFINITIONS)
    unset(${_target}_PRIVATE_DEFINITIONS)
    unset(${_target}_INCLUDE_DIRECTORIES)
    unset(${_target}_SYSTEM_INCLUDE_DIRECTORIES)
    unset(${_target}_PRIVATE_INCLUDE_DIRECTORIES)

    # Reset the sources
    set(${_target}_SOURCES ${opt_SOURCES})
    set(${_target}_LIBRARIES ${opt_LIBRARIES})
    set(${_target}_DEPENDENCIES ${_wrapped_target_name})
endmacro()

macro(PURC_CREATE_FORWARDING_HEADER _target_directory _file)
    get_filename_component(_source_path "${CMAKE_SOURCE_DIR}/Source/" ABSOLUTE)
    get_filename_component(_absolute "${_file}" ABSOLUTE)
    get_filename_component(_name "${_file}" NAME)
    set(_target_filename "${_target_directory}/${_name}")

    # Try to make the path in the forwarding header relative to the Source directory
    # so that these forwarding headers are compatible with the ones created by the
    # PurC2 generate-forwarding-headers script.
    string(REGEX REPLACE "${_source_path}/" "" _relative ${_absolute})

    set(_content "#include \"${_relative}\"\n")

    if (EXISTS "${_target_filename}")
        file(READ "${_target_filename}" _old_content)
    endif ()

    if (NOT _old_content STREQUAL _content)
        file(WRITE "${_target_filename}" "${_content}")
    endif ()
endmacro()

macro(PURC_CREATE_FORWARDING_HEADERS _framework)
    # On Windows, we copy the entire contents of forwarding headers.
    if (NOT WIN32)
        set(_processing_directories 0)
        set(_processing_files 0)
        set(_target_directory "${FORWARDING_HEADERS_DIR}/${_framework}")

        file(GLOB _files "${_target_directory}/*.h")
        foreach (_file ${_files})
            file(READ "${_file}" _content)
            string(REGEX MATCH "^#include \"([^\"]*)\"" _matched ${_content})
            if (_matched AND NOT EXISTS "${CMAKE_SOURCE_DIR}/Source/${CMAKE_MATCH_1}")
               file(REMOVE "${_file}")
            endif ()
        endforeach ()

        foreach (_currentArg ${ARGN})
            if ("${_currentArg}" STREQUAL "DIRECTORIES")
                set(_processing_directories 1)
                set(_processing_files 0)
            elseif ("${_currentArg}" STREQUAL "FILES")
                set(_processing_directories 0)
                set(_processing_files 1)
            elseif (_processing_directories)
                file(GLOB _files "${_currentArg}/*.h")
                foreach (_file ${_files})
                    PURC_CREATE_FORWARDING_HEADER(${_target_directory} ${_file})
                endforeach ()
            elseif (_processing_files)
                PURC_CREATE_FORWARDING_HEADER(${_target_directory} ${_currentArg})
            endif ()
        endforeach ()
    endif ()
endmacro()

function(PURC_MAKE_FORWARDING_HEADERS framework)
    set(options FLATTENED)
    set(oneValueArgs DESTINATION TARGET_NAME)
    set(multiValueArgs DIRECTORIES FILES)
    cmake_parse_arguments(opt "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
    set(headers ${opt_FILES})
    file(MAKE_DIRECTORY ${opt_DESTINATION})
    foreach (dir IN LISTS opt_DIRECTORIES)
        file(GLOB files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${dir}/*.h)
        list(APPEND headers ${files})
    endforeach ()
    set(fwd_headers)
    foreach (header IN LISTS headers)
        if (IS_ABSOLUTE ${header})
            set(src_header ${header})
        else ()
            set(src_header ${CMAKE_CURRENT_SOURCE_DIR}/${header})
        endif ()
        if (opt_FLATTENED)
            get_filename_component(header_filename ${header} NAME)
            set(fwd_header ${opt_DESTINATION}/${header_filename})
        else ()
            get_filename_component(header_dir ${header} DIRECTORY)
            file(MAKE_DIRECTORY ${opt_DESTINATION}/${header_dir})
            set(fwd_header ${opt_DESTINATION}/${header})
        endif ()
        add_custom_command(OUTPUT ${fwd_header}
            COMMAND ${CMAKE_COMMAND} -E copy ${src_header} ${fwd_header}
            MAIN_DEPENDENCY ${header}
            VERBATIM
        )
        list(APPEND fwd_headers ${fwd_header})
    endforeach ()
    if (opt_TARGET_NAME)
        set(target_name ${opt_TARGET_NAME})
    else ()
        set(target_name ${framework}ForwardingHeaders)
    endif ()
    add_custom_target(${target_name} DEPENDS ${fwd_headers})
    add_dependencies(${framework} ${target_name})
endfunction()

function(PURC_COPY_FILES target_name)
    set(options FLATTENED)
    set(oneValueArgs DESTINATION)
    set(multiValueArgs FILES)
    cmake_parse_arguments(opt "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
    set(files ${opt_FILES})
    set(dst_files)
    foreach (file IN LISTS files)
        if (IS_ABSOLUTE ${file})
            set(src_file ${file})
        else ()
            set(src_file ${CMAKE_CURRENT_SOURCE_DIR}/${file})
        endif ()
        if (opt_FLATTENED)
            get_filename_component(filename ${file} NAME)
            set(dst_file ${opt_DESTINATION}/${filename})
        else ()
            get_filename_component(file_dir ${file} DIRECTORY)
            file(MAKE_DIRECTORY ${opt_DESTINATION}/${file_dir})
            set(dst_file ${opt_DESTINATION}/${file})
        endif ()
        add_custom_command(OUTPUT ${dst_file}
            COMMAND ${CMAKE_COMMAND} -E copy ${src_file} ${dst_file}
            MAIN_DEPENDENCY ${file}
            VERBATIM
        )
        list(APPEND dst_files ${dst_file})
    endforeach ()
    add_custom_target(${target_name} ALL DEPENDS ${dst_files})
endfunction()

# Helper macros for debugging CMake problems.
macro(PURC_DEBUG_DUMP_COMMANDS)
    set(CMAKE_VERBOSE_MAKEFILE ON)
endmacro()

macro(PURC_DEBUG_DUMP_VARIABLES)
    set_cmake_property(_variableNames VARIABLES)
    foreach (_variableName ${_variableNames})
       message(STATUS "${_variableName}=${${_variableName}}")
    endforeach ()
endmacro()

# Append the given flag to the target property.
# Builds on top of get_target_property() and set_target_properties()
macro(PURC_ADD_TARGET_PROPERTIES _target _property _flags)
    get_target_property(_tmp ${_target} ${_property})
    if (NOT _tmp)
        set(_tmp "")
    endif (NOT _tmp)

    foreach (f ${_flags})
        set(_tmp "${_tmp} ${f}")
    endforeach (f ${_flags})

    set_target_properties(${_target} PROPERTIES ${_property} ${_tmp})
    unset(_tmp)
endmacro()

macro(PURC_POPULATE_LIBRARY_VERSION library_name)
    if (NOT DEFINED ${library_name}_VERSION_MAJOR)
        set(${library_name}_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
    endif ()
    if (NOT DEFINED ${library_name}_VERSION_MINOR)
        set(${library_name}_VERSION_MINOR ${PROJECT_VERSION_MINOR})
    endif ()
    if (NOT DEFINED ${library_name}_VERSION_MICRO)
        set(${library_name}_VERSION_MICRO ${PROJECT_VERSION_MICRO})
    endif ()
    if (NOT DEFINED ${library_name}_VERSION)
        set(${library_name}_VERSION ${PROJECT_VERSION})
    endif ()
endmacro()

macro(PURC_CREATE_SYMLINK target src dest)
    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND ln -sf ${src} ${dest}
        DEPENDS ${dest}
        COMMENT "Create symlink from ${src} to ${dest}")
endmacro()

