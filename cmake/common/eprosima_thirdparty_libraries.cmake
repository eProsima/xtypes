# Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Macro to find cmake built thirdparty libraries.
#
# Arguments:
#   :package: The name of the packge to find. Used for find_package(${package})
#
# Related CMake options:
#   :THIRDPARTY: Activate the use of internal thirdparties [Defaults: ON].
#        Possible values:
#            * ON -> Use submodules only if package is not found elsewhere in the system.
#            * OFF -> Do not use submodules.
#            * FORCE -> Force the use submodules regarless of what is installed in the system.
#   :THIRDPARTY_${package}: Specialization of THIRDPARTY for a specific ${package}. Same possibilities as THIRDPARTY.
#        Unless specified otherwise, it has the same value as THIRDPARTY. Its value has preference over that of
#        THIRDPARTY.
#
# The macro's procedure is as follows:
#   If THIRDPARTY_${package} is ON, update the submodule in thirdparty folder
#
macro(eprosima_download_thirdparty package)
    # Parse arguments.
    set(options REQUIRED)
    set(multiValueArgs OPTIONS)
    cmake_parse_arguments(FIND "${options}" "" "${multiValueArgs}" ${ARGN})

    # Define a list of allowed values for the options THIRDPARTY and THIRDPARTY_${package}
    set(ALLOWED_VALUES ON OFF FORCE)

    # Create the THIRDPARTY variable defaulting to OFF
    set(THIRDPARTY ${XTYPES_THIRDPARTY} CACHE STRING "Activate use of internal submodules.")
    # Define list of values GUI will offer for the variable
    set_property(CACHE THIRDPARTY PROPERTY STRINGS ON OFF FORCE)
    # Check that specified value is allowed
    if(NOT THIRDPARTY IN_LIST ALLOWED_VALUES)
        message(FATAL_ERROR, "Wrong configuration of THIRDPARTY. Allowed values: ${ALLOWED_VALUES}")
    endif()

    # Create the THIRDPARTY_${package} variable defaulting to ${THIRDPARTY}. This way, we only need to check the value
    # of THIRDPARTY_${package} from here on.
    set(THIRDPARTY_${package} ${THIRDPARTY} CACHE STRING "Activate use of internal submodule ${package}.")
    # Define list of values GUI will offer for the variable
    set_property(CACHE THIRDPARTY_${package} PROPERTY STRINGS ON OFF FORCE)
    # Check that specified value is allowed
    if(NOT THIRDPARTY_${package} IN_LIST ALLOWED_VALUES)
        message(FATAL_ERROR, "Wrong configuration of THIRDPARTY_${package}. Allowed values: ${ALLOWED_VALUES}")
    endif()

    option(THIRDPARTY_UPDATE "Activate the auto update of internal thirdparties" ON)

    # Use thirdparty if THIRDPARTY_${package} is set to FORCE, or if the package is not found elsewhere and
    # THIRDPARTY_${package} is set to ON.
    if(THIRDPARTY_${package})
        # Update submodule
        message(STATUS "Updating submodule thirdparty/${package}")
        execute_process(
            COMMAND git submodule update --init "thirdparty/${package}"
            WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
            RESULT_VARIABLE EXECUTE_RESULT
            )
        # A result different than 0 means that the submodule could not be updated.
        if(NOT EXECUTE_RESULT EQUAL 0)
            message(WARNING "Cannot configure Git submodule ${package}")
        endif()
    endif()
endmacro()
