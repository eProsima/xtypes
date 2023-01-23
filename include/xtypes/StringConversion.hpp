/*
 * Copyright 2023, Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef EPROSIMA_STRING_CONVERSION_HPP_
#define EPROSIMA_STRING_CONVERSION_HPP_

#include <locale>
#include <version>

#ifdef __cpp_char8_t
#   define XTYPES_CHAR char8_t
#   define XTYPES_CHAR_LITERAL(text) u8 ## text
#else
#   define XTYPES_CHAR char
#   define XTYPES_CHAR_LITERAL(text) text
#endif

namespace eprosima {

// The purpose of this classes is workaround C++17 deprecation of STL string conversion APIs
namespace detail {

template<typename in, typename out>
struct choose_codecvt;

template<typename wchar>
struct choose_codecvt_base
{
    using cvt = std::codecvt<wchar, XTYPES_CHAR, std::mbstate_t>;
};

template<typename wchar>
struct choose_codecvt<XTYPES_CHAR, wchar>
    : choose_codecvt_base<wchar>
{
    template<typename facet>
    static std::codecvt_base::result translate(const facet& f,
            std::mbstate_t& state,
            const XTYPES_CHAR* from,
            const XTYPES_CHAR* from_end,
            const XTYPES_CHAR*& from_next,
            wchar* to,
            wchar* to_end,
            wchar*& to_next)
    {
        return f.in(state, from, from_end, from_next, to, to_end, to_next);
    }
};

template<typename wchar>
struct choose_codecvt<wchar, XTYPES_CHAR>
    : choose_codecvt_base<wchar>
{
    template<typename facet>
    static std::codecvt_base::result translate(const facet& f,
            std::mbstate_t& state,
            const wchar* from,
            const wchar* from_end,
            const wchar*& from_next,
            XTYPES_CHAR* to,
            XTYPES_CHAR* to_end,
            XTYPES_CHAR*& to_next)
    {
        return f.out(state, from, from_end, from_next, to, to_end, to_next);
    }
};

} // detail

template<typename out, typename in>
std::basic_string<out> code_conversion_tool(const std::basic_string_view<in>& input)
{
    using namespace std;
    using namespace detail;
    using choose = choose_codecvt<in, out>;

    locale loc;
    // on mac STL ALL codecvt partial specializations use XTYPES_CHAR as external type
    auto& conv_facet = use_facet<typename choose::cvt>(loc);

    const unsigned int buffer_size = 64;
    out buffer[buffer_size];

    mbstate_t mb{};
    out* to_next;
    std::basic_string<out> output;
    const in* from_next = input.data();
    const in* from_end = from_next + input.size();
    bool processing = true;

    // iterate if buffer gets filled
    while (processing)
    {
        codecvt_base::result res = choose::translate(
             conv_facet,
             mb,
             from_next,
             from_end,
             from_next,
             buffer,
             buffer + buffer_size,
             to_next);

        switch (res)
        {
            case codecvt_base::ok:
                // gcc implementation mixes up ok and partial
                if(from_next == from_end)
                {
                    // we are done
                    processing = false;
                    // append the contents remember
                    output.append(buffer, to_next - buffer);
                    break;
                }
                [[fallthrough]];
            case codecvt_base::partial:
                // insert current buffer content
                output.append(buffer, buffer_size);
                break;

            case codecvt_base::error:
                throw std::range_error("encountered a character that could not be converted");

            case codecvt_base::noconv:
                break;
                // case codecvt_base::noconv doesn't apply in this overload but not including it arises a warning
        }
    }

    return output;
}

template<typename out, typename in>
std::basic_string<out> code_conversion_tool(const std::basic_string<in>& input)
{
    return code_conversion_tool<out>(std::basic_string_view<in>{input});
}

} // eprosima

#endif // EPROSIMA_STRING_CONVERSION_HPP_

