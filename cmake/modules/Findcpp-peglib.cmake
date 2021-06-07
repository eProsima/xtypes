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

# THIRDPARTY

message(STATUS "Module Find cpp-peglib")

find_package(cpp-peglib CONFIG)
if (NOT cpp-peglib_INCLUDE_DIR)
    message(STATUS "Package cpp-peglib not found, looking for header")
    find_path(cpp-peglib_INCLUDE_DIR NAMES peglib.h PATH_SUFFIXES include)
endif()

if (NOT cpp-peglib_INCLUDE_DIR)
    message(STATUS "Path to peglib.h not found, looking in thirdparty")

    # Using thirdparty macros
    eprosima_download_thirdparty(cpp-peglib)
    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/thirdparty/cpp-peglib)
    find_path(cpp-peglib_INCLUDE_DIR NAMES peglib.h PATHS ${PROJECT_SOURCE_DIR}/thirdparty/cpp-peglib REQUIRED)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(cpp-peglib DEFAULT_MSG cpp-peglib_INCLUDE_DIR)

mark_as_advanced(cpp-peglib_INCLUDE_DIR)

message(STATUS "Module Find cpp-peglib end. Found peglib.h in ${cpp-peglib_INCLUDE_DIR}")
