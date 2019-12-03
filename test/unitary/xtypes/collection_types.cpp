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
#include <iostream>

#include <cmath>
#include <bitset>

using namespace eprosima::xtypes;

/**********************************************
 *        DynamicType Collection Tests        *
 **********************************************/

TEST (CollectionTypes, array)
{
    ArrayType array10_1(primitive_type<int>(), 10);
    ArrayType array10_2(primitive_type<int>(), 10);
    ArrayType array100_1(primitive_type<int>(), 100);

    EXPECT_EQ(array10_1.dimension(), 10);
    EXPECT_EQ(array10_2.dimension(), 10);
    EXPECT_EQ(array100_1.dimension(), 100);

    DynamicData data_1(array10_1);

    for (size_t i = 0; i < data_1.size(); ++i)
    {
        data_1[i] = int(i);
    }

    DynamicData data_2(data_1, array10_2);
    EXPECT_EQ(data_2, data_1);

    for (size_t i = 0; i < data_1.size(); ++i)
    {
        EXPECT_EQ(data_1[i].value<int>(), data_2[i].value<int>());
        EXPECT_EQ(data_1[i].value<int>(), int(i));
    }

    DynamicData data_3(data_1, array100_1);
    EXPECT_NE(data_3, data_1);

    for (size_t i = 0; i < data_1.size(); ++i)
    {
        EXPECT_EQ(data_1[i].value<int>(), data_3[i].value<int>());
    }
}

TEST (CollectionTypes, multi_array)
{
    ArrayType simple_array(primitive_type<uint32_t>(), 5);
    ArrayType array_array(simple_array, 4);
    ArrayType array_array_array(array_array, 3);

    EXPECT_EQ(array_array_array.dimension(), 3);
    EXPECT_EQ(array_array.dimension(), 4);
    EXPECT_EQ(simple_array.dimension(), 5);

    EXPECT_EQ(array_array_array.memory_size(), 3 * 4 * 5 * sizeof(uint32_t));

    DynamicData data(array_array_array);

    EXPECT_EQ(data.bounds(), 3);

    for (size_t i = 0; i < 3; ++i)
    {
        EXPECT_EQ(data[i].bounds(), 4);
        for (size_t j = 0; j < 4; ++j)
        {
            EXPECT_EQ(data[i][j].bounds(), 5);
            for (size_t k = 0; k < 5; ++k)
            {
                data[i][j][k] = static_cast<uint32_t>(i + j + k);
                uint32_t temp = data[i][j][k];
                EXPECT_EQ(data[i][j][k].value<uint32_t>(), static_cast<uint32_t>(i + j + k));
                EXPECT_EQ(temp, static_cast<uint32_t>(i + j + k));
            }
        }
    }

}

template<typename T>
void check_primitive_array(T value)
{
    ArrayType type(primitive_type<T>(), 1);
    EXPECT_EQ(type.dimension(), 1);
    EXPECT_EQ(type.memory_size(), sizeof(T));

    DynamicData data(type);
    EXPECT_EQ(data.bounds(), 1);
    data[0] = value;
    EXPECT_EQ(value, data[0].value<T>());
    T v = data[0];
    EXPECT_EQ(value, v);
}

TEST (CollectionTypes, primitive_arrays)
{
    check_primitive_array<bool>(true);
    check_primitive_array<bool>(false);
    check_primitive_array<uint8_t>(250);
    check_primitive_array<int16_t>(-2500);
    check_primitive_array<uint16_t>(2500);
    check_primitive_array<int32_t>(-250000);
    check_primitive_array<uint32_t>(250000);
    check_primitive_array<int64_t>(-25000000000);
    check_primitive_array<uint64_t>(25000000000);
    check_primitive_array<float>(250.76653);
    check_primitive_array<double>(250.76653e40);
    check_primitive_array<long double>(250.76653e1000l);
    check_primitive_array<char>('L');
    check_primitive_array<wchar_t>(L'G');
}

TEST (CollectionTypes, sequence_array)
{
    SequenceType seq(primitive_type<double>());
    ArrayType seq_array(seq, 5);

    EXPECT_EQ(seq_array.dimension(), 5);

    DynamicData data(seq_array);
    EXPECT_EQ(data.bounds(), 5);
    EXPECT_EQ(data.size(), 5);

    for (size_t i = 0; i < 5; ++i)
    {
        EXPECT_EQ(data[i].bounds(), 0);
        EXPECT_EQ(data[i].size(), 0);
        for (size_t j = 0; j < i; ++j)
        {
            data[i].push(j * 1.1);
            double temp = data[i][j];
            EXPECT_EQ(data[i][j].value<double>(), static_cast<double>(j * 1.1));
            EXPECT_EQ(temp, static_cast<double>(j * 1.1));
        }
        EXPECT_EQ(data[i].bounds(), 0);
        EXPECT_EQ(data[i].size(), i);
    }
}

TEST (CollectionTypes, sequence)
{
    SequenceType s10_1(primitive_type<uint16_t>(), 10);
    SequenceType s10_2(primitive_type<uint16_t>(), 10);
    SequenceType s20_1(primitive_type<uint16_t>(), 20);
    SequenceType su_1(primitive_type<uint16_t>());

    DynamicData d(s10_1);
    EXPECT_EQ(0, d.size());
    EXPECT_EQ(10, d.bounds());

    for(int i = 0; i < 10; ++i)
    {
        d.push(uint16_t(i));
    }

    EXPECT_EQ(10, d.size());
    EXPECT_EQ(10, d.bounds());

    DynamicData dd(d, s20_1);

    EXPECT_EQ(10, dd.size());
    EXPECT_EQ(20, dd.bounds());
    EXPECT_FALSE(d != dd);

    DynamicData ddd(d, s10_2);
    EXPECT_FALSE(ddd != d);

    DynamicData dddd(d, su_1); // This ctor isn't copying content, and it should!
    EXPECT_EQ(10, dddd.size()); // This EXPECT will fail because the lack of content copy. Keep failing as a reminder.
    EXPECT_EQ(0, dddd.bounds());
    EXPECT_FALSE(dddd != d);

    for (size_t i = 0; i < d.size(); ++i)
    {
        EXPECT_EQ(d[i].value<uint16_t>(), dd[i].value<uint16_t>());
        EXPECT_EQ(d[i].value<uint16_t>(), ddd[i].value<uint16_t>());
        //EXPECT_EQ(d[i].value<uint16_t>(), dddd[i].value<uint16_t>()); // index < size() assert raising due to the lack of copy.
        EXPECT_EQ(d[i].value<uint16_t>(), uint16_t(i));
        uint16_t temp = dd[i];
        EXPECT_EQ(temp, uint16_t(i));
    }
}

TEST (CollectionTypes, resize_sequence)
{
    SequenceType seq(primitive_type<uint16_t>(), 100);

    DynamicData d(seq);
    EXPECT_EQ(0, d.size());
    EXPECT_EQ(100, d.bounds());

    for(uint16_t i = 0; i < 10; ++i)
    {
        d.push(i);
    }

    EXPECT_EQ(10, d.size());

    for (size_t i = 0; i < d.size(); ++i)
    {
        EXPECT_EQ(d[i].value<uint16_t>(), i);
    }

    // Check nothing happens
    d.resize(2);
    EXPECT_EQ(10, d.size());
    EXPECT_EQ(100, d.bounds());
    for (size_t i = 0; i < d.size(); ++i)
    {
        EXPECT_EQ(d[i].value<uint16_t>(), i);
    }

    // A resize must keep the size and content,
    d.resize(20);
    EXPECT_EQ(20, d.size());
    EXPECT_EQ(100, d.bounds());
    for (size_t i = 0; i < 10; ++i) // From 10 to 20, the values are default initialized, so uninteresting to us now.
    {
        EXPECT_EQ(d[i].value<uint16_t>(), i);
    }

    // And of course, allow to modify the new elements
    for(size_t i = 0; i < 10; ++i)
    {
        d[i + 10] = uint16_t(i);
    }
    EXPECT_EQ(20, d.size());
    for (size_t i = 0; i < d.size(); ++i)
    {
        EXPECT_EQ(d[i].value<uint16_t>(), i%10);
    }

    // Cannot grow over the bounds
    ASSERT_DEATH({d.resize(101);}, "bounds()");
}

TEST (CollectionTypes, multi_sequence)
{
    SequenceType simple_seq(primitive_type<uint32_t>(), 5);
    SequenceType seq_seq(simple_seq, 4);
    SequenceType seq_seq_seq(seq_seq, 0); // Must be called explicitely to avoid copy ctor.

    DynamicData data(seq_seq_seq);

    EXPECT_EQ(data.bounds(), 0);
    EXPECT_EQ(data.size(), 0);

    for (size_t i = 0; i < 3; ++i)
    {
        data.push(DynamicData(seq_seq));
        EXPECT_EQ(data[i].size(), 0);
        for (size_t j = 0; j < 4; ++j)
        {
            data[i].push(DynamicData(simple_seq));
            EXPECT_EQ(data[i][j].size(), 0);
            for (size_t k = 0; k < 5; ++k)
            {
                data[i][j].push(static_cast<uint32_t>(i + j + k));
                uint32_t temp = data[i][j][k];
                EXPECT_EQ(data[i][j][k].value<uint32_t>(), static_cast<uint32_t>(i + j + k));
                EXPECT_EQ(temp, static_cast<uint32_t>(i + j + k));
                EXPECT_EQ(data[i][j].size(), k + 1);
            }
            EXPECT_EQ(data[i][j].bounds(), 5);
            EXPECT_EQ(data[i].size(), j + 1);
        }
        EXPECT_EQ(data[i].bounds(), 4);
        EXPECT_EQ(data.size(), i + 1);
    }
    EXPECT_EQ(data.bounds(), 0);
}

TEST (CollectionTypes, array_sequence)
{
    ArrayType array(primitive_type<double>(), 5);
    SequenceType array_seq(array);

    DynamicData data(array_seq);

    EXPECT_EQ(data.bounds(), 0);
    EXPECT_EQ(data.size(), 0);

    for (size_t i = 0; i < 3; ++i)
    {
        data.push(DynamicData(array));
        EXPECT_EQ(data.size(), i + 1);
        EXPECT_EQ(data[i].bounds(), 5);
        EXPECT_EQ(data[i].size(), 5);
        for (size_t j = 0; j < 5; ++j)
        {
            data[i][j] = j * 1.1;
            double temp = data[i][j];
            EXPECT_EQ(data[i][j].value<double>(), static_cast<double>(j * 1.1));
            EXPECT_EQ(temp, static_cast<double>(j * 1.1));
        }
        EXPECT_EQ(data[i].bounds(), 5);
        EXPECT_EQ(data[i].size(), 5);
    }
    EXPECT_EQ(data.bounds(), 0);
}

template<typename T>
void check_primitive_seq(T value)
{
    SequenceType type(primitive_type<T>(), 1);
    DynamicData data(type);
    EXPECT_EQ(data.size(), 0);
    EXPECT_EQ(data.bounds(), 1);
    data.push(value);
    EXPECT_EQ(data.size(), 1);
    EXPECT_EQ(value, data[0].value<T>());
    T v = data[0];
    EXPECT_EQ(value, v);
}

TEST (CollectionTypes, primitive_sequences)
{
    check_primitive_seq<bool>(true);
    check_primitive_seq<bool>(false);
    check_primitive_seq<uint8_t>(250);
    check_primitive_seq<int16_t>(-2500);
    check_primitive_seq<uint16_t>(2500);
    check_primitive_seq<int32_t>(-250000);
    check_primitive_seq<uint32_t>(250000);
    check_primitive_seq<int64_t>(-25000000000);
    check_primitive_seq<uint64_t>(25000000000);
    check_primitive_seq<float>(250.76653);
    check_primitive_seq<double>(250.76653e40);
    check_primitive_seq<long double>(250.76653e1000l);
    check_primitive_seq<char>('L');
    check_primitive_seq<wchar_t>(L'G');
}

TEST (CollectionTypes, string)
{
    StringType s(20);
    StringType t(10);

    DynamicData ds(s);
    ds = "12345678901234567890";

    DynamicData dt(t);
    dt = "1234567890";

    DynamicData dt_as_ds(dt, s);
    EXPECT_EQ(10, dt_as_ds.size());

    DynamicData ds_as_dt(ds, t);
    EXPECT_EQ(10, ds_as_dt.size());

    EXPECT_TRUE(dt == ds_as_dt);

    StringType u;
    DynamicData du(u);
    du = "123456789012345678901234567890";

    ASSERT_DEATH(dt = du, "TypeConsistency::EQUALS");

    dt = "01234567890123456789";
    EXPECT_EQ(dt.value<std::string>(), "0123456789");
    EXPECT_EQ(dt.size(), 10);
}

TEST (CollectionTypes, wstring)
{
    WStringType s(20);
    WStringType t(10);

    EXPECT_EQ(s.is_compatible(t) , t.is_compatible(s));
    EXPECT_EQ(TypeConsistency::IGNORE_STRING_BOUNDS, t.is_compatible(s));

    DynamicData ds(s);
    ds = L"12345678901234567890";

    DynamicData dt(t);
    dt = L"1234567890";

    DynamicData dt_as_ds(dt, s);
    EXPECT_EQ(10, dt_as_ds.size());

    DynamicData ds_as_dt(ds, t);
    EXPECT_EQ(10, ds_as_dt.size());

    EXPECT_TRUE(dt == ds_as_dt);

    WStringType u;
    DynamicData du(u);
    du = L"123456789012345678901234567890";

    ASSERT_DEATH(dt = du, "TypeConsistency::EQUALS");

    dt = L"01234567890123456789";
    EXPECT_EQ(dt.value<std::wstring>(), L"0123456789");
    EXPECT_EQ(dt.size(), 10);
}
