#####################################################################################
# Top xtypes build system
#####################################################################################
cmake_minimum_required(VERSION 3.5)

project(xtypes
    VERSION "0.1.0"
    LANGUAGES CXX
    )

option(XTYPES_BUILD_TESTS "Build tests." OFF)
option(XTYPES_BUILD_EXAMPLES "Build examples." OFF)
option(XTYPES_EXCEPTIONS "Enable xtypes exceptions in release (which are asserts in debug)." OFF)
option(XTYPES_BUILD_TOOLS "Build tools." OFF)
option(THIRDPARTY "Allow to download thidparty repository when needed." ON)

if(XTYPES_EXCEPTIONS)
    #add_compile_definitions(XTYPES_EXCEPTIONS) #CMake 3.16
    add_definitions(-DXTYPES_EXCEPTIONS)
else()
    remove_definitions(-DXTYPES_EXCEPTIONS)
endif()

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

#####################################################################################
# Load CMake modules
#####################################################################################
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake/modules)
include(${PROJECT_SOURCE_DIR}/cmake/common/eprosima_thirdparty_libraries.cmake)

#####################################################################################
# Thirdparty library
#####################################################################################
find_package(cpp-peglib REQUIRED)

message(STATUS "Found peglib.h: ${cpp-peglib_INCLUDE_DIR}")

#####################################################################################
# Library
#####################################################################################
add_library(${PROJECT_NAME} INTERFACE)

target_include_directories(${PROJECT_NAME}
    INTERFACE
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${cpp-peglib_INCLUDE_DIR}>        # Path to peglib.h
        $<INSTALL_INTERFACE:include>
        $<INSTALL_INTERFACE:thirdparty/cpp-peglib/include>
    )

target_compile_features(${PROJECT_NAME}
    INTERFACE
        cxx_std_17
    )

#####################################################################################
# Tools
#####################################################################################
macro(compile_tool)
    # Parse arguments
    if("${ARGV0}" STREQUAL "NAME")
        set(uniValueArgs NAME)
    else()
        set(TOOL_NAME "${ARGV0}")
    endif()

    set(multiValueArgs SOURCE)
    cmake_parse_arguments(TOOL "" "${uniValueArgs}" "${multiValueArgs}" ${ARGN})

    add_executable(${TOOL_NAME} ${TOOL_SOURCE})

    target_link_libraries(${TOOL_NAME}
        PUBLIC
            Threads::Threads
        PRIVATE
            ${PROJECT_NAME}
        )

    target_compile_options(${TOOL_NAME}
        INTERFACE
            $<$<CXX_COMPILER_ID:GNU>:-Wall>
            $<$<CXX_COMPILER_ID:GNU>:-Wextra>
        )

    set_target_properties(${TOOL_NAME}
        PROPERTIES
            CXX_STANDARD 17
        )
endmacro()

find_program(NODERED_EXECUTABLE NAMES node-red node-red@ PATH_SUFFIXES bin)

if(XTYPES_BUILD_TOOLS OR NODERED_EXECUTABLE)
    compile_tool(${PROJECT_NAME}_idl_validator SOURCE tools/idl_validator.cpp)

    if (NODERED_EXECUTABLE)
        file(WRITE $ENV{HOME}/idl_parser_path.txt "${PROJECT_BINARY_DIR}")
        message("${PROJECT_BINARY_DIR}")
    endif()
    # ...
endif()

#####################################################################################
# Examples
#####################################################################################
macro(compile_example)
    # Parse arguments
    if("${ARGV0}" STREQUAL "NAME")
        set(uniValueArgs NAME)
    else()
        set(EXAMPLE_NAME "${ARGV0}")
    endif()

    set(multiValueArgs SOURCE)
    cmake_parse_arguments(EXAMPLE "" "${uniValueArgs}" "${multiValueArgs}" ${ARGN})

    add_executable(${EXAMPLE_NAME} ${EXAMPLE_SOURCE})

    target_link_libraries(${EXAMPLE_NAME}
        PUBLIC
            Threads::Threads
        PRIVATE
            ${PROJECT_NAME}
        )

    target_compile_options(${EXAMPLE_NAME}
        INTERFACE
            $<$<CXX_COMPILER_ID:GNU>:-Wall>
            $<$<CXX_COMPILER_ID:GNU>:-Wextra>
        )

    set_target_properties(${EXAMPLE_NAME}
        PROPERTIES
            CXX_STANDARD 17
        )
endmacro()

if(XTYPES_BUILD_EXAMPLES)
    compile_example(${PROJECT_NAME}_example_complex_type SOURCE examples/complex_type.cpp)
    compile_example(${PROJECT_NAME}_example_module SOURCE examples/module.cpp)
    compile_example(${PROJECT_NAME}_example_iterators SOURCE examples/iterators.cpp)
    compile_example(${PROJECT_NAME}_example_exceptions_asserts SOURCE examples/exceptions_asserts.cpp)
    # ...
endif()

#####################################################################################
# Tests
#####################################################################################
macro(compile_test)
    # Parse arguments
    if("${ARGV0}" STREQUAL "NAME")
        set(uniValueArgs NAME)
    else()
        set(TEST_NAME "${ARGV0}")
    endif()
    set(multiValueArgs SOURCE)
    cmake_parse_arguments(TEST "" "${uniValueArgs}" "${multiValueArgs}" ${ARGN})

    compile_example(${TEST_NAME} SOURCE ${TEST_SOURCE})

    target_link_libraries(${TEST_NAME}
        PRIVATE
            libgtest
        )

    set_target_properties(${TEST_NAME}
        PROPERTIES
            CXX_STANDARD 17
        )

    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
endmacro()

if(XTYPES_BUILD_TESTS)
    include(CTest)
    include(test/gtest.cmake)
    enable_testing()

    compile_test(${PROJECT_NAME}_test_unitary SOURCE
        test/unitary/xtypes/main.cpp
        test/unitary/xtypes/primitive_types.cpp
        test/unitary/xtypes/collection_types.cpp
        test/unitary/xtypes/struct_type.cpp
        test/unitary/xtypes/union_type.cpp
        test/unitary/xtypes/iterators.cpp
        test/unitary/xtypes/consistency.cpp
        test/unitary/xtypes/integration.cpp
        test/unitary/xtypes/alias_type.cpp
        test/unitary/xtypes/dynamicdata_operators.cpp
        )

    compile_test(${PROJECT_NAME}_test_unitary_no_memcheck SOURCE
        test/unitary/xtypes/main.cpp
        test/unitary/xtypes/no_memcheck_tests.cpp)

    compile_test(${PROJECT_NAME}_idl_parser_test_unitary SOURCE
        test/unitary/parser/module.cpp
        test/unitary/parser/parser_test.cpp)
    compile_test(${PROJECT_NAME}_idl_parser_roundtrip SOURCE test/unitary/parser/roundtrip.cpp)
    compile_test(${PROJECT_NAME}_idl_generator_dependencies SOURCE test/unitary/generator/dependencies.cpp)
    # ...

    # Set test with label NoMemoryCheck
    set_property(TEST ${PROJECT_NAME}_test_unitary_no_memcheck PROPERTY LABELS "NoMemoryCheck")

    # Copy IDL files
    configure_file(
        ${PROJECT_SOURCE_DIR}/test/unitary/idl/test01.idl
        ${PROJECT_BINARY_DIR}/idl/test01.idl
        COPYONLY)
    configure_file(
        ${PROJECT_SOURCE_DIR}/test/unitary/idl/test02.idl
        ${PROJECT_BINARY_DIR}/idl/test02.idl
        COPYONLY)
    configure_file(
        ${PROJECT_SOURCE_DIR}/test/unitary/idl/test03.idl
        ${PROJECT_BINARY_DIR}/idl/test03.idl
        COPYONLY)
    configure_file(
        ${PROJECT_SOURCE_DIR}/test/unitary/idl/test04.idl
        ${PROJECT_BINARY_DIR}/idl/test04.idl
        COPYONLY)
    configure_file(
        ${PROJECT_SOURCE_DIR}/test/unitary/idl/include/test_include.idl
        ${PROJECT_BINARY_DIR}/idl/include/test_include.idl
        COPYONLY)
endif()

#####################################################################################
# Installation
#####################################################################################
include(GNUInstallDirs)
set(BIN_INSTALL_DIR     ${CMAKE_INSTALL_BINDIR}     CACHE PATH "Installation directory for binaries")
set(INCLUDE_INSTALL_DIR ${CMAKE_INSTALL_INCLUDEDIR} CACHE PATH "Installation directory for C headers")
set(LIB_INSTALL_DIR     ${CMAKE_INSTALL_LIBDIR}     CACHE PATH "Installation directory for libraries")
set(DATA_INSTALL_DIR    ${CMAKE_INSTALL_DATADIR}    CACHE PATH "Installation directory for data")

include(CMakePackageConfigHelpers)

configure_package_config_file(
    ${PROJECT_SOURCE_DIR}/cmake/config.cmake.in
    ${PROJECT_BINARY_DIR}/cmake/config/${PROJECT_NAME}-config.cmake
    INSTALL_DESTINATION
        ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/cmake
    PATH_VARS
        BIN_INSTALL_DIR
        INCLUDE_INSTALL_DIR
        LIB_INSTALL_DIR
        DATA_INSTALL_DIR
    )

write_basic_package_version_file(
    ${PROJECT_BINARY_DIR}/cmake/config/${PROJECT_NAME}-config-version.cmake
    VERSION
        ${PROJECT_VERSION}
    COMPATIBILITY
        SameMajorVersion
    )

install(
    TARGETS
        ${PROJECT_NAME}
    EXPORT
        ${PROJECT_NAME}-targets
    RUNTIME DESTINATION
        ${BIN_INSTALL_DIR}
    LIBRARY DESTINATION
        ${LIB_INSTALL_DIR}
    ARCHIVE DESTINATION
        ${LIB_INSTALL_DIR}
    )

install(
    DIRECTORY
        ${PROJECT_SOURCE_DIR}/include/xtypes/
    DESTINATION
        ${INCLUDE_INSTALL_DIR}/xtypes
    FILES_MATCHING
        PATTERN "*.hpp"
    )

install(
    FILES
        ${cpp-peglib_INCLUDE_DIR}/peglib.h
    DESTINATION
        # This could be ${INCLUDE_INSTALL_DIR}/ but is left like this so it does not break compatibilitys
        thirdparty/cpp-peglib/include
    )

install(
    EXPORT
        ${PROJECT_NAME}-targets
    DESTINATION
        ${DATA_INSTALL_DIR}/${PROJECT_NAME}/cmake
    )

install(
    FILES
        ${PROJECT_BINARY_DIR}/cmake/config/${PROJECT_NAME}-config.cmake
        ${PROJECT_BINARY_DIR}/cmake/config/${PROJECT_NAME}-config-version.cmake
    DESTINATION
        ${DATA_INSTALL_DIR}/${PROJECT_NAME}/cmake
    )
