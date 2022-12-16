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

/*************************************************************************
 * Tests that cannot be tested using valgrind without any kind of error. *
 *************************************************************************/
static const long double LDOUBLE    = 3.14159265358979321159979631875l;

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

// Primitive long double fails when running on valgrind because valgrind uses 64bit to represent long double,
// so the EXPECT_FALSE check fails.
// On windows long double == double
TEST (PrimitiveTypes, primitive_type_longdouble)
{

#ifdef WIN32
    EXPECT_TRUE(singleCheck<long double>(1.797e308L, 1.797e308L));
    EXPECT_FALSE(singleCheck<long double>(1.7970000000000001e308L, 1.797e308L));
#else
    EXPECT_TRUE(singleCheck<long double>(5.55e1200l, 5.55e1200l));
    EXPECT_FALSE(singleCheck<long double>(5.550000000000000001e1200l, 5.55e1200l));
#endif // WIN32

    assignCheck<long double>(LDOUBLE);
}

// When testing using valgrind, the constructor fails and raises an assert or an exception, causing
// that the internal "instance_" memory cannot be freed.
TEST (CollectionTypes, map_asserts)
{
    MapType m10_1(primitive_type<uint32_t>(), primitive_type<uint16_t>(), 10);
    MapType m10_3(StringType(), primitive_type<uint16_t>(), 10);
    MapType m20_1(primitive_type<bool>(), primitive_type<uint16_t>(), 20);
    DynamicData d(m10_1);
    ASSERT_OR_EXCEPTION({DynamicData fails(d, m20_1);}, "Cannot copy data from different types");
    ASSERT_OR_EXCEPTION({DynamicData fails(d, m10_3);}, "Incompatible types");
}
