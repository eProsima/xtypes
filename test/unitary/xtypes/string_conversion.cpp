// Copyright 2023 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <gtest/gtest.h>
#include <xtypes/StringConversion.hpp>

using namespace std;
using namespace eprosima;

#define MBSTRING(name, text) std::basic_string<XTYPES_CHAR> name{XTYPES_CHAR_LITERAL(text)};

TEST (StringConversion, from_utf8_to_utf16_short)
{
    MBSTRING(short_str, "test")
    u16string expected{u"test"};
    ASSERT_EQ(code_conversion_tool<char16_t>(short_str), expected);
}

TEST (StringConversion, from_utf8_to_utf16_long)
{
    MBSTRING(long_str, "test")
    u16string expected{u"test"};

    for(int i = 0; i < 1000; ++i)
    {
        int car = i % 74 + 48;
        long_str.append({static_cast<XTYPES_CHAR>(car)});
        expected.append({static_cast<char16_t>(car)});
    }

    ASSERT_EQ(code_conversion_tool<char16_t>(long_str), expected);
}

TEST (StringConversion, from_utf16_to_utf8_short)
{
    u16string short_str{u"test"};
    MBSTRING(expected, "test")
    ASSERT_EQ(code_conversion_tool<XTYPES_CHAR>(short_str), expected);
}

TEST (StringConversion, from_utf16_to_utf8_long)
{
    u16string long_str{u"test"};
    MBSTRING(expected, "test")

    for(int i = 0; i < 1000; ++i)
    {
        int car = i % 74 + 48;
        long_str.append({static_cast<char16_t>(car)});
        expected.append({static_cast<XTYPES_CHAR>(car)});
    }

    ASSERT_EQ(code_conversion_tool<XTYPES_CHAR>(long_str), expected);
}
