#####################################################################################
#                             Top xtypes build system
#####################################################################################

if(MSVC)
    cmake_minimum_required(VERSION 3.22)
else()
    cmake_minimum_required(VERSION 3.16.3..3.22)
endif()

project(xtypes
    VERSION "0.2.0"
    LANGUAGES CXX
    )

option(XTYPES_BUILD_TESTS "Build tests." OFF)
option(XTYPES_BUILD_EXAMPLES "Build examples." OFF)
option(XTYPES_EXCEPTIONS "Enable xtypes exceptions in release (which are asserts in debug)." OFF)
option(XTYPES_BUILD_TOOLS "Build tools." OFF)

include(CMakeDependentOption)

cmake_dependent_option(ENABLE_PDB_INDEXING
    [=[Uses the PDBStr tool to modify the pdb (symbol files).
    The new files will download the sources from github simplifying
    post mortem debugging of failing tests dumps (WER). ]=]
    OFF "MSVC" OFF)

cmake_dependent_option(ENABLE_WER
    [=[Allow windows crash dump generation on crashes and timeouts.
    WER can only be used if the binaries are launched for each individual
    test otherwise some of the tests may not be executed. ]=]
    OFF "WIN32" OFF)

#[=[
    Timeout in seconds to raise a WER exception.
    If combined with CTest timeout make sure CTest value is larger.
]=]
if(ENABLE_WER)
    set(WER_TIMEOUT_TIME 10)
endif()

set(XTYPES_PEGLIB_VERSION "master" CACHE STRING "yhirose/cpp-peglib version to use.")

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

#####################################################################################
#                                    Library
#####################################################################################
add_library(xtypes INTERFACE)
add_library(eprosima::xtypes ALIAS xtypes)

if(XTYPES_EXCEPTIONS)
    target_compile_definitions(xtypes INTERFACE XTYPES_EXCEPTIONS)
elseif(MSVC)
    target_link_libraries(xtypes INTERFACE Dbghelp.lib)
endif()

# Download the cpp-peglib header file needed
file(DOWNLOAD
        https://raw.githubusercontent.com/yhirose/cpp-peglib/${XTYPES_PEGLIB_VERSION}/peglib.h
        ${PROJECT_BINARY_DIR}/thirdparty/cpp-peglib/peglib.h
    )

target_include_directories(xtypes
    INTERFACE
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/thirdparty/cpp-peglib>
        $<INSTALL_INTERFACE:include>
        $<INSTALL_INTERFACE:thirdparty/cpp-peglib/include>
    )

target_compile_features(xtypes INTERFACE cxx_std_17 cxx_variadic_macros)

if(MSVC)

    # MSVC always returns __cplusplus = 199711 unles we use the flag /Zc:__cplusplus then it follows the GNU behavior
    execute_process(COMMAND
        powershell -Command "$binarydir = \"${PROJECT_BINARY_DIR}\";" [=[
            $header = gi "$binarydir/thirdparty/cpp-peglib/peglib.h";
            (Get-Content -Encoding UTF8 -Path $header |
            % { $_ -replace '__cplusplus', '_MSVC_LANG' } ) |
            Set-Content -Encoding UTF8 -Path $header
            ]=]
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
       )

    # Preprocessor:
    #  target_compile_features() only enforces standard values like cxx_std_17. All other values like
    #  cxx_variadic_macros are not enforced using flags but only will generate an error if not available.
    #  CMake docs is misguiding (see cmGeneratorTarget::ComputeCompileFeatures implementation).
    # Encoding:
    #  The header uses non-basic character set characters in narrow strings. Execution charset is hinted to allow UTF-8
    target_compile_options(xtypes INTERFACE /Zc:preprocessor /execution-charset:UTF-8)

    # Check if source indexing tools are available
    find_program(PDBSTR_PATH pdbstr
        PATHS "[HKLM/SOFTWARE/Microsoft/Windows Kits/Installed Roots;KitsRoot10]" ENV WindowsSdkDir
        PATH_SUFFIXES srcsrv "Debuggers/x64/srcsrv")

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
    # allow node-red to locate it loading the overlay
    install(TARGETS xtypes_idl_validator)
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

    if(ENABLE_PDB_INDEXING AND PDBSTR_PATH)
        # get the tools path
        get_filename_component(PDB_TOOLS_PATH "${PDBSTR_PATH}" DIRECTORY ABSOLUTE)
        # add post build event
        add_custom_command(
            TARGET ${EXAMPLE_NAME} POST_BUILD
            COMMENT "Adding source indexing to pdb symbols"
            COMMAND "$<IF:$<CONFIG:Debug,RelWithDebInfo>,powershell,exit>"
                       -ExecutionPolicy Bypass
                       -File "${CMAKE_SOURCE_DIR}/test/add_source_listing.ps1"
                       -pdbfiles "$<TARGET_PDB_FILE:${EXAMPLE_NAME}>"
                       -toolsPath "${PDB_TOOLS_PATH}"
                       # -Verbose
            VERBATIM
            )
    endif()

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
    set(multiValueArgs SOURCE PROPERTIES)
    cmake_parse_arguments(TEST "" "${uniValueArgs}" "${multiValueArgs}" ${ARGN})

    compile_example(${TEST_NAME} SOURCE ${TEST_SOURCE})

if(MSVC)
    target_compile_options(${TEST_NAME} PRIVATE /bigobj)
endif(MSVC)

    target_link_libraries(${TEST_NAME}
        PRIVATE
            GTest::gtest
            $<TARGET_NAME_IF_EXISTS:eprosima::wer>
        )

    if(ENABLE_WER)
        set(XTYPES_GTEST_FLAGS EXTRA_ARGS --gtest_catch_exceptions=0)
    endif()

    gtest_discover_tests(${TEST_NAME}
            ${XTYPES_GTEST_FLAGS}
            PROPERTIES ${TEST_PROPERTIES} ${MSVC_TEST_PROPERTIES})
endmacro()

if(XTYPES_BUILD_TESTS)
    include(CTest)
    enable_testing()

    # apt install libgtest-dev
    find_package(GTest CONFIG REQUIRED)
    include(GoogleTest)

    if(ENABLE_WER)
        add_subdirectory(test/WER) # first because the others may depend on it
    endif()

    if(MSVC)

        # Populate test environment if required
        find_path(CL_BIN cl.exe)

        # if preprocessor is already available not populate
        if(NOT CL_BIN)

            set(PREPROCESS_GENERATOR_PLATFORM ${CMAKE_GENERATOR_PLATFORM})
            set(PREPROCESS_GENERATOR_TOOLSET ${CMAKE_GENERATOR_TOOLSET})

            if(NOT CMAKE_GENERATOR_PLATFORM)
                set(PREPROCESS_GENERATOR_PLATFORM x64)
            endif()

            if(NOT CMAKE_GENERATOR_TOOLSET)
                set(PREPROCESS_GENERATOR_TOOLSET x64,host=x64)
            endif()

            execute_process(
                COMMAND powershell -File "${CMAKE_CURRENT_LIST_DIR}\\test\\environment.ps1"
                                   -Platform ${PREPROCESS_GENERATOR_PLATFORM}
                                   -Toolset ${PREPROCESS_GENERATOR_TOOLSET}
                OUTPUT_VARIABLE MSVC_TEST_PROPERTIES)
        endif()

        # Remove any console line feeds
        string(STRIP "${MSVC_TEST_PROPERTIES}" MSVC_TEST_PROPERTIES)

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
        test/unitary/xtypes/no_memcheck_tests.cpp
        PROPERTIES LABELS "NoMemoryCheck")

    compile_test(xtypes_idl_parser_test_unitary SOURCE
        test/unitary/parser/module.cpp
        test/unitary/parser/parser_test.cpp)
    compile_test(xtypes_idl_parser_roundtrip SOURCE test/unitary/parser/roundtrip.cpp)
    compile_test(xtypes_idl_generator_dependencies SOURCE test/unitary/generator/dependencies.cpp)

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
