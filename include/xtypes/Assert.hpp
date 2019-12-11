/*
 * Copyright 2019, Proyectos y Sistemas de Mantenimiento SL (eProsima).
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
*/

#ifndef EPROSIMA_XTYPES_ASSERT_HPP_
#define EPROSIMA_XTYPES_ASSERT_HPP_

#include <execinfo.h>
#include <iostream>
#include <sstream>

#if !defined(NDEBUG)

#define xtypes_assert2_(cond, msg) xtypes_assert3_(cond, msg, false)

#define xtypes_assert3_(cond, msg, bt)                                                                              \
    {                                                                                                               \
        if (!(cond))                                                                                                \
        {                                                                                                           \
            std::stringstream ss__;                                                                                 \
            ss__ << "[XTYPES]: ";                                                                                   \
            ss__ << __FILE__ << ":" << __LINE__ << " - ";                                                           \
            ss__ << "Assertion failed with message: ";                                                              \
            ss__ << msg << std::endl;                                                                            \
            if (bt)                                                                                                 \
            {                                                                                                       \
                void* callstack[128];                                                                               \
                int frames = backtrace(callstack, 128);                                                             \
                char** symbols = backtrace_symbols(callstack, frames);                                              \
                ss__ << std::endl << "Backtrace:" << std::endl;                                                     \
                for (int i = 0; i < frames; ++i)                                                                    \
                {                                                                                                   \
                    ss__ << symbols[i] << std::endl;                                                                \
                }                                                                                                   \
                free(symbols);                                                                                      \
            }                                                                                                       \
            std::cerr << ss__.str() << std::endl;                                                                         \
            std::abort();                                                                                           \
        }                                                                                                           \
    }                                                                                                               \

#else
#define xtypes_assert2_(cond, msg)
#define xtypes_assert3_(cond, msg, bt)
#endif

#define GET_MACRO(_1, _2, _3, NAME, ...) NAME
#define xtypes_assert(...) GET_MACRO(__VA_ARGS__, xtypes_assert3_, xtypes_assert2_)(__VA_ARGS__)


#endif // EPROSIMA_XTYPES_ASSERT_HPP_
