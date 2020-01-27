// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
#include <xtypes/xtypes.hpp>

#include "../utils.hpp"

using namespace eprosima::xtypes;

static const uint8_t UINT8          = 250;
static const int16_t INT16          = -32760;
static const uint16_t UINT16        = 65530;
static const int32_t INT32          = -2147483640;
static const uint32_t UINT32        = 4294967290;
static const int64_t INT64          = -9223372036854775800;
static const uint64_t UINT64        = 18446744073709551610ULL;
static const float FLOAT            = 3.1415927410125732421875f;
static const double DOUBLE          = 3.1415926535897931159979631875;
static const long double LDOUBLE    = 3.14159265358979321159979631875l;
static const char CHAR              = 'f';
static const wchar_t WCHAR          = 34590;

/*********************************************
 *        DynamicType Primitive Tests        *
 *********************************************/

template<typename A, typename B>
bool singleCheck(A av, B bv)
{
    DynamicData dd1(primitive_type<A>());
    DynamicData dd2(primitive_type<B>());
    dd1.value(av);
    dd2.value(bv);
    return dd1.value<A>() == dd2.value<B>();
}

template<typename A>
bool singleCheck(A av, A bv)
{
    return singleCheck<A, A>(av, bv);
}

template<typename A>
void assignCheck(A value)
{
    DynamicData dd1(primitive_type<A>());
    DynamicData dd2(primitive_type<A>());
    dd1 = value;
    dd2 = dd1;
    EXPECT_EQ(dd1, dd2);
    EXPECT_EQ(dd1.value<A>(), dd2.value<A>());
    EXPECT_EQ(dd1.value<A>(), value);
    EXPECT_EQ(value, dd2.value<A>());
    EXPECT_EQ(sizeof(A), dd1.type().memory_size());
    EXPECT_EQ(sizeof(A), dd2.type().memory_size());
}

TEST (PrimitiveTypes, primitive_type_bool)
{
    EXPECT_TRUE(singleCheck<bool>(true, true));
    EXPECT_FALSE(singleCheck<bool>(true, false));
    assignCheck<bool>(true);
    assignCheck<bool>(false);
}

TEST (PrimitiveTypes, primitive_type_uint8)
{
    EXPECT_TRUE(singleCheck<uint8_t>(15, 15));
    EXPECT_FALSE(singleCheck<uint8_t>(15, 16));
    EXPECT_TRUE(singleCheck<uint8_t>(-1, 255));
    assignCheck<uint8_t>(UINT8);
}

TEST (PrimitiveTypes, primitive_type_uint16)
{
    EXPECT_TRUE(singleCheck<uint16_t>(1500, 1500));
    EXPECT_FALSE(singleCheck<uint16_t>(1500, 1501));
    EXPECT_TRUE(singleCheck<uint16_t>(-1, 0xFFFF));
    assignCheck<uint16_t>(UINT16);
}

TEST (PrimitiveTypes, primitive_type_int16)
{
    EXPECT_TRUE(singleCheck<int16_t>(-1500, -1500));
    EXPECT_FALSE(singleCheck<int16_t>(1500, -1500));
    EXPECT_FALSE(singleCheck<int16_t>(1500, 1501));
    EXPECT_TRUE(singleCheck<int16_t>(-1, int16_t(0xFFFF)));
    assignCheck<int16_t>(INT16);
}

TEST (PrimitiveTypes, primitive_type_uint32)
{
    EXPECT_TRUE(singleCheck<uint32_t>(150000, 150000));
    EXPECT_FALSE(singleCheck<uint32_t>(150000, 150001));
    EXPECT_TRUE(singleCheck<uint32_t>(-1, 0xFFFFFFFF));
    assignCheck<uint32_t>(UINT32);
}

TEST (PrimitiveTypes, primitive_type_int32)
{
    EXPECT_TRUE(singleCheck<int32_t>(-150000, -150000));
    EXPECT_FALSE(singleCheck<int32_t>(-150000, 150000));
    EXPECT_FALSE(singleCheck<int32_t>(150000, 150001));
    EXPECT_TRUE(singleCheck<int32_t>(-1, 0xFFFFFFFF));
    assignCheck<int32_t>(INT32);
}

TEST (PrimitiveTypes, primitive_type_uint64)
{
    EXPECT_TRUE(singleCheck<uint64_t>(15000000000, 15000000000));
    EXPECT_FALSE(singleCheck<uint64_t>(15000000000, 15000000001));
    EXPECT_TRUE(singleCheck<uint64_t>(-1, 0xFFFFFFFFFFFFFFFF));
    assignCheck<uint64_t>(UINT64);
}

TEST (PrimitiveTypes, primitive_type_int64)
{
    EXPECT_TRUE(singleCheck<uint64_t>(-15000000000, -15000000000));
    EXPECT_FALSE(singleCheck<uint64_t>(-15000000000, 15000000000));
    EXPECT_FALSE(singleCheck<uint64_t>(15000000000, 15000000001));
    EXPECT_TRUE(singleCheck<uint64_t>(-1, 0xFFFFFFFFFFFFFFFF));
    assignCheck<int64_t>(INT64);
}

TEST (PrimitiveTypes, primitive_type_float)
{
    EXPECT_TRUE(singleCheck(54.5f, 54.5f));
    EXPECT_FALSE(singleCheck(5.56f, 5.55f));
    assignCheck<float>(FLOAT);
}

TEST (PrimitiveTypes, primitive_type_double)
{
    EXPECT_TRUE(singleCheck<double>(5.55e40, 5.55e40));
    EXPECT_FALSE(singleCheck<double>(5.550000001e40, 5.55e40));
    assignCheck<double>(DOUBLE);
}

TEST (PrimitiveTypes, DISABLED_primitive_type_longdouble)
{
    EXPECT_TRUE(singleCheck<long double>(5.55e1200l, 5.55e1200l));
    EXPECT_FALSE(singleCheck<long double>(5.550000001e1200l, 5.55e1200l));
    assignCheck<long double>(LDOUBLE);
}

TEST (PrimitiveTypes, primitive_type_char)
{
    EXPECT_TRUE(singleCheck<char>('a', 'a'));
    EXPECT_FALSE(singleCheck<char>('a', 'b'));
    EXPECT_TRUE((singleCheck<char, uint8_t>('a', 'a')));
    assignCheck<char>(CHAR);
}

TEST (PrimitiveTypes, primitive_type_wchar)
{
    EXPECT_TRUE(singleCheck<wchar_t>(L'a', L'a'));
    EXPECT_FALSE(singleCheck<wchar_t>(L'a', L'b'));
    assignCheck<wchar_t>(WCHAR);
}

TEST (PrimitiveTypes, primitive_type_double_longdouble)
{
    EXPECT_TRUE((singleCheck<double, long double>(55.55e40, 55.55e40)));
    EXPECT_FALSE((singleCheck<double, long double>(55.55000001e40, 55.55e40)));
}

TEST (PrimitiveTypes, primitive_type_int16_uint32)
{
    EXPECT_TRUE((singleCheck<int16_t, uint32_t>(55, 55)));
}

TEST (PrimitiveTypes, primitive_type_double_uint8)
{
    EXPECT_TRUE((singleCheck<double, uint8_t>(22, 22)));
}

TEST (PrimitiveTypes, primitive_type_int32_uint16)
{
    EXPECT_TRUE((singleCheck<int32_t, uint16_t>(55, 55)));
}

TEST(EnumerationType, enumeration_tests)
{
    // Creation and expected operation
    {
        EnumerationType<uint32_t> my_enum("MyEnum");
        my_enum.add_enumerator("A", 0);
        my_enum.add_enumerator("B", 10);
        my_enum.add_enumerator("C");

        DynamicData enum_data(my_enum);
        enum_data = my_enum.value("C");

        uint32_t value = enum_data;
        DynamicData enum_data2 = enum_data;
        uint32_t value2 = enum_data2;

        EXPECT_EQ(value, 11);
        EXPECT_EQ(value, my_enum.value("C"));
        EXPECT_EQ(value, value2);

        enum_data2 = static_cast<uint32_t>(10);
        value2 = enum_data2;
        EXPECT_EQ(10, value2);
    }
    // ASSERT_DEATHS
    {
        EnumerationType<uint32_t> my_enum("MyEnum");
        my_enum.add_enumerator("A", 0);
        my_enum.add_enumerator("B", 10);
        my_enum.add_enumerator("C");

        ASSERT_OR_EXCEPTION({my_enum.add_enumerator("D", 11);}, "greater than"); // Asserts because 11 == last added value
        ASSERT_OR_EXCEPTION({my_enum.add_enumerator("E", 2);}, "greater than"); // Asserts because 2 < last added value
        ASSERT_OR_EXCEPTION({my_enum.add_enumerator("A");}, "already has an enumerator"); // Asserts because A already exists

        DynamicData enum_data(my_enum);
        enum_data = my_enum.value("C");

        ASSERT_OR_EXCEPTION({uint64_t die = enum_data; (void) die;}, "Incompatible"); // This will assert

        // EnumerationType<uint64_t> my_long_enum("MyLongEnum"); // Static assert, uint64_t isn't allowed

        ASSERT_OR_EXCEPTION({enum_data = static_cast<uint32_t>(2);}, "invalid value"); // Asserts because 2 isn't a valid value (0, 10 and 11).
    }
}
