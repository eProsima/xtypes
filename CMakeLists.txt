#####################################################################################
#                             Top xtypes build system
#####################################################################################
cmake_minimum_required(VERSION 3.16.3 FATAL_ERROR)

project(xtypes
    VERSION "0.1.0"
    LANGUAGES CXX
    )

option(XTYPES_BUILD_TESTS "Build tests." OFF)
option(XTYPES_BUILD_EXAMPLES "Build examples." OFF)
option(XTYPES_EXCEPTIONS "Enable xtypes exceptions in release (which are asserts in debug)." OFF)
option(XTYPES_BUILD_TOOLS "Build tools." OFF)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

#####################################################################################
#                                    Library
#####################################################################################
add_library(xtypes INTERFACE)
add_library(eprosima::xtypes ALIAS xtypes)

if(XTYPES_EXCEPTIONS)
    target_compile_definitions(xtypes INTERFACE XTYPES_EXCEPTIONS)
endif()

# Download the cpp-peglib header file needed
file(DOWNLOAD
        https://raw.githubusercontent.com/yhirose/cpp-peglib/master/peglib.h
        ${PROJECT_BINARY_DIR}/thirdparty/cpp-peglib/peglib.h
    )

# MSVC always returns __cplusplus = 199711 unles we use the flag /Zc:__cplusplus then it follows the GNU behavior
if(MSVC)
    execute_process(COMMAND 
        powershell -Command "$binarydir = \"${PROJECT_BINARY_DIR}\";" [=[
            $header = gi "$binarydir/thirdparty/cpp-peglib/peglib.h";
            (Get-Content -Encoding UTF8 -Path $header |
            % { $_ -replace '__cplusplus', '_MSVC_LANG' } ) |
            Set-Content -Encoding UTF8 -Path $header
            ]=]
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
       )
endif(MSVC)

target_include_directories(xtypes
    INTERFACE
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/thirdparty/cpp-peglib>
        $<INSTALL_INTERFACE:include>
        $<INSTALL_INTERFACE:thirdparty/cpp-peglib/include>
    )

target_compile_features(xtypes INTERFACE cxx_std_17 cxx_variadic_macros)

if(MSVC)
    # Preprocessor:
    #  target_compile_features() only enforces standard values like cxx_std_17. All other values like
    #  cxx_variadic_macros are not enforced using flags but only will generate an error if not available.
    #  CMake docs is misguiding (see cmGeneratorTarget::ComputeCompileFeatures implementation).
    # Encoding:
    #  The header uses non-basic character set characters in narrow strings. Execution charset is hinted to allow UTF-8
    target_compile_options(xtypes INTERFACE /Zc:preprocessor /execution-charset:UTF-8)
endif(MSVC)

#####################################################################################
#                                    Tools
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
            xtypes
        )

    target_compile_options(${TOOL_NAME}
        PRIVATE
            $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra>
        )
endmacro()

find_program(NODERED_EXECUTABLE NAMES node-red node-red@ PATH_SUFFIXES bin)

if(XTYPES_BUILD_TOOLS OR NODERED_EXECUTABLE)
    compile_tool(xtypes_idl_validator SOURCE tools/idl_validator.cpp)

    # WSL for example lacks TMP
    if(NOT ENV{TMP})
        set(TMP_ROOT "/tmp")
    else()
        set(TMP_ROOT "$ENV{TMP}")
    endif()

    if (NODERED_EXECUTABLE)
        file(WRITE ${TMP_ROOT}/idl_parser_path.txt "${PROJECT_BINARY_DIR}")
        message("${PROJECT_BINARY_DIR}")
    endif()

    unset(TMP_ROOT)
endif()

#####################################################################################
#                                    Examples
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
            xtypes
        )

    target_compile_options(${EXAMPLE_NAME}
        PRIVATE
            $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Wno-sign-compare -Wno-multichar>
        )
endmacro()

if(XTYPES_BUILD_EXAMPLES)
    compile_example(xtypes_example_complex_type SOURCE examples/complex_type.cpp)
    compile_example(xtypes_example_module SOURCE examples/module.cpp)
    compile_example(xtypes_example_iterators SOURCE examples/iterators.cpp)
    compile_example(xtypes_example_exceptions_asserts SOURCE examples/exceptions_asserts.cpp)
    # ...
endif()

#####################################################################################
#                                      Tests
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
            GTest::gtest
        )

    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})

    if(TEST_EXTRA_VARIABLES)
        set_property(TEST ${TEST_NAME} PROPERTY ENVIRONMENT "${TEST_EXTRA_VARIABLES}")
    endif()

endmacro()

if(XTYPES_BUILD_TESTS)
    include(CTest)
    enable_testing()

    # apt install libgtest-dev
    find_package(GTest CONFIG REQUIRED)

    if(MSVC)
        # Populate test environment if required
        find_path(CL_BIN cl.exe)

        # if preprocessor is already available not populate
        if(NOT CL_BIN)
            execute_process(
                COMMAND powershell -File "${CMAKE_CURRENT_LIST_DIR}\\test\\environment.ps1"
                OUTPUT_VARIABLE EXTRA_ENVIRONMENT_OUTPUT)
        endif()

        if(EXTRA_ENVIRONMENT_OUTPUT)
            string(REGEX MATCH "^(.*)>><<(.*)$" EXTRA_ENVIRONMENT_OUTPUT "${EXTRA_ENVIRONMENT_OUTPUT}")
        endif()

        set(TEST_EXTRA_VARIABLES "${CMAKE_MATCH_1}")
        set(TEST_EXTRA_PATHS "Path=$ENV{Path};${CMAKE_MATCH_2}")
        string(REPLACE ";" "\\;" TEST_EXTRA_PATHS "${TEST_EXTRA_PATHS}")
        set(TEST_EXTRA_VARIABLES "${TEST_EXTRA_VARIABLES};${TEST_EXTRA_PATHS}")
        unset(TEST_EXTRA_PATHS)
        unset(EXTRA_ENVIRONMENT_OUTPUT)

    endif(MSVC)

    compile_test(xtypes_test_unitary SOURCE
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

    compile_test(xtypes_test_unitary_no_memcheck SOURCE
        test/unitary/xtypes/main.cpp
        test/unitary/xtypes/no_memcheck_tests.cpp)

    compile_test(xtypes_idl_parser_test_unitary SOURCE
        test/unitary/parser/module.cpp
        test/unitary/parser/parser_test.cpp)
    compile_test(xtypes_idl_parser_roundtrip SOURCE test/unitary/parser/roundtrip.cpp)
    compile_test(xtypes_idl_generator_dependencies SOURCE test/unitary/generator/dependencies.cpp)
    # ...

    # Set test with label NoMemoryCheck
    set_property(TEST xtypes_test_unitary_no_memcheck PROPERTY LABELS "NoMemoryCheck")

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
#                                   Installation
#####################################################################################
include(GNUInstallDirs)
set(BIN_INSTALL_DIR     ${CMAKE_INSTALL_BINDIR}     CACHE PATH "Installation directory for binaries")
set(INCLUDE_INSTALL_DIR ${CMAKE_INSTALL_INCLUDEDIR} CACHE PATH "Installation directory for C headers")
set(LIB_INSTALL_DIR     ${CMAKE_INSTALL_LIBDIR}     CACHE PATH "Installation directory for libraries")
set(DATA_INSTALL_DIR    ${CMAKE_INSTALL_DATADIR}    CACHE PATH "Installation directory for data")

include(CMakePackageConfigHelpers)

configure_package_config_file(
    ${PROJECT_SOURCE_DIR}/cmake/config.cmake.in
    ${PROJECT_BINARY_DIR}/cmake/config/xtypes-config.cmake
    INSTALL_DESTINATION
        ${CMAKE_INSTALL_DATADIR}/xtypes/cmake
    PATH_VARS
        BIN_INSTALL_DIR
        INCLUDE_INSTALL_DIR
        LIB_INSTALL_DIR
        DATA_INSTALL_DIR
    )

write_basic_package_version_file(
    ${PROJECT_BINARY_DIR}/cmake/config/xtypes-config-version.cmake
    VERSION
        ${PROJECT_VERSION}
    COMPATIBILITY
        SameMajorVersion
    )

install(
    TARGETS
        xtypes
    EXPORT
        xtypes-targets
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
    DIRECTORY
        ${PROJECT_BINARY_DIR}/thirdparty/cpp-peglib/
    DESTINATION
        thirdparty/cpp-peglib/include
    FILES_MATCHING
        PATTERN "*.h"
    )

install(
    EXPORT
        xtypes-targets
#   NAMESPACE
#       eprosima::
    DESTINATION
        ${DATA_INSTALL_DIR}/xtypes/cmake
    )

install(
    FILES
        ${PROJECT_BINARY_DIR}/cmake/config/xtypes-config.cmake
        ${PROJECT_BINARY_DIR}/cmake/config/xtypes-config-version.cmake
    DESTINATION
        ${DATA_INSTALL_DIR}/xtypes/cmake
    )
