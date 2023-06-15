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

#include "../utils.hpp"

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

TEST (CollectionTypes, multi_array_constructor)
{

    ArrayType array_array_array(primitive_type<uint32_t>(), {3, 4, 5});
    const ArrayType& inner = static_cast<const ArrayType&>(array_array_array.content_type());
    const ArrayType& inner_inner = static_cast<const ArrayType&>(inner.content_type());

    EXPECT_EQ(array_array_array.dimension(), 3);
    EXPECT_EQ(inner.dimension(), 4);
    EXPECT_EQ(inner_inner.dimension(), 5);

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
            }
        }
    }

    for (size_t i = 0; i < 3; ++i)
    {
        EXPECT_EQ(data[i].bounds(), 4);
        for (size_t j = 0; j < 4; ++j)
        {
            EXPECT_EQ(data[i][j].bounds(), 5);
            for (size_t k = 0; k < 5; ++k)
            {
                EXPECT_EQ(data[i][j][k].value<uint32_t>(), static_cast<uint32_t>(i + j + k));
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
#ifdef _MSC_VER
    check_primitive_array<long double>(1.797e308L);
#else
    check_primitive_array<long double>(250.76653e1000l);
#endif // _MSC_VER
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

    DynamicData dddd(d, su_1);
    EXPECT_EQ(10, dddd.size());
    EXPECT_EQ(0, dddd.bounds());
    EXPECT_FALSE(dddd != d);

    for (size_t i = 0; i < d.size(); ++i)
    {
        EXPECT_EQ(d[i].value<uint16_t>(), dd[i].value<uint16_t>());
        EXPECT_EQ(d[i].value<uint16_t>(), ddd[i].value<uint16_t>());
        EXPECT_EQ(d[i].value<uint16_t>(), dddd[i].value<uint16_t>());
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
    ASSERT_OR_EXCEPTION({d.resize(101);}, "is bigger than maximum allowed");
}

TEST (CollectionType, resize_complex_sequence)
{
    StructType str("my_struct");
    str.add_member("st0", StringType());
    str.add_member("st1", primitive_type<uint32_t>());
    SequenceType seq(str, 0);
    DynamicData data(seq);

    EXPECT_EQ(data.bounds(), 0);
    EXPECT_EQ(data.size(), 0);

    for (size_t i = 0; i < 3; ++i)
    {
        std::stringstream ss;
        ss << "data_" << i;
        DynamicData item(str);
        item["st0"] = ss.str();
        item["st1"] = static_cast<uint32_t>(i);
        data.push(item);
    }
    EXPECT_EQ(data.size(), 3);

    data.resize(5);
    EXPECT_EQ(data.bounds(), 0);
    EXPECT_EQ(data.size(), 5);

    for (size_t i = 3; i < 5; ++i)
    {
        std::stringstream ss;
        ss << "new_data_" << i;
        data[i]["st0"] = ss.str();
        data[i]["st1"] = 2*static_cast<uint32_t>(i);
    }

    for (size_t i = 0; i < 5; ++i)
    {
        std::stringstream ss;
        ss << (i < 3 ? "data_" : "new_data_") << i;
        EXPECT_EQ(data[i]["st0"].value<std::string>(), ss.str());
        EXPECT_EQ(data[i]["st1"].value<uint32_t>(), i < 3 ? i : 2*i);
    }
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
#ifdef _MSC_VER
    check_primitive_seq<long double>(1.79e308l);
#else
    check_primitive_seq<long double>(250.76653e1000l);
#endif // _MSC_VER
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

    ASSERT_OR_EXCEPTION(dt = du;, "Cannot assign DynamicData of type");

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

    ASSERT_OR_EXCEPTION(dt = du;, "Cannot assign DynamicData of type");

    dt = L"01234567890123456789";
    EXPECT_EQ(dt.value<std::wstring>(), L"0123456789");
    EXPECT_EQ(dt.size(), 10);
}

TEST (CollectionTypes, string16)
{
    String16Type s(20);
    String16Type t(10);

    EXPECT_EQ(s.is_compatible(t) , t.is_compatible(s));
    EXPECT_EQ(TypeConsistency::IGNORE_STRING_BOUNDS, t.is_compatible(s));

    DynamicData ds(s);
    ds = u"12345678901234567890";

    DynamicData dt(t);
    dt = u"1234567890";

    DynamicData dt_as_ds(dt, s);
    EXPECT_EQ(10, dt_as_ds.size());

    DynamicData ds_as_dt(ds, t);
    EXPECT_EQ(10, ds_as_dt.size());

    EXPECT_TRUE(dt == ds_as_dt);

    String16Type u;
    DynamicData du(u);
    du = u"123456789012345678901234567890";

    ASSERT_OR_EXCEPTION(dt = du;, "Cannot assign DynamicData of type");

    dt = u"01234567890123456789";
    EXPECT_EQ(dt.value<std::u16string>(), u"0123456789");
    EXPECT_EQ(dt.size(), 10);
}

// TEST (CollectionTypes, map_asserts) moved to no_memcheck_tests.cpp

TEST (CollectionTypes, map)
{
    MapType m10_1(primitive_type<uint32_t>(), primitive_type<uint16_t>(), 10);
    MapType m10_2(primitive_type<uint32_t>(), primitive_type<uint16_t>(), 10);
    MapType m10_3(StringType(), primitive_type<uint16_t>(), 10);
    MapType m20_1(primitive_type<bool>(), primitive_type<uint16_t>(), 20);
    MapType m20_2(primitive_type<uint32_t>(), primitive_type<uint16_t>(), 20);
    MapType mu_1(primitive_type<uint32_t>(), primitive_type<uint16_t>());

    DynamicData d(m10_1);
    EXPECT_EQ(0, d.size());
    EXPECT_EQ(10, d.bounds());

    DynamicData uint32_data(primitive_type<uint32_t>());
    // Fill the map
    for(uint32_t i = 0; i < 10; ++i)
    {
        uint32_data = i;
        d[uint32_data] = uint16_t(i);
    }
    // Check the map contents
    for(uint32_t i = 0; i < 10; ++i)
    {
        uint32_data = i;
        ASSERT_EQ(d.at(uint32_data).value<uint16_t>(), i);
    }

    EXPECT_EQ(10, d.size());
    EXPECT_EQ(10, d.bounds());

    DynamicData dd(d, m20_2);
    EXPECT_EQ(10, dd.size());
    EXPECT_EQ(20, dd.bounds());
    EXPECT_FALSE(d != dd);

    DynamicData ddd(d, m10_2);
    EXPECT_FALSE(ddd != d);

    DynamicData dddd(d, mu_1);
    EXPECT_EQ(10, dddd.size());
    EXPECT_EQ(0, dddd.bounds());
    EXPECT_FALSE(dddd != d);

    for (size_t idx = 0; idx < d.size(); ++idx)
    {
        uint32_data = uint32_t(idx);
        uint16_t value = d[uint32_data];
        EXPECT_EQ(value, dd[uint32_data].value<uint16_t>());
        EXPECT_EQ(value, ddd[uint32_data].value<uint16_t>());
        EXPECT_EQ(value, dddd[uint32_data].value<uint16_t>());
        EXPECT_EQ(value, uint16_t(idx));
        uint16_t temp = dd[uint32_data];
        EXPECT_EQ(temp, uint16_t(idx));
    }
}

TEST (CollectionTypes, map_bounds)
{
    //MapType map(StringType(), primitive_type<uint16_t>(), 100);
    MapType map(primitive_type<uint8_t>(), primitive_type<uint16_t>(), 10);

    DynamicData d(map);
    EXPECT_EQ(0, d.size());
    EXPECT_EQ(10, d.bounds());

    //StringType str_type;
    //DynamicData index(str_type);
    DynamicData index(primitive_type<uint8_t>());
    for(uint16_t i = 0; i < 10; ++i)
    {
        //index = std::to_string(i);
        index = uint8_t(i);
        d[index] = i;
    }

    EXPECT_EQ(10, d.size());

    for (size_t i = 0; i < d.size(); ++i)
    {
        //index = std::to_string(i);
        index = uint8_t(i);
        EXPECT_EQ(d[index].value<uint16_t>(), i);
    }

    // Cannot grow over the bounds
    ASSERT_OR_EXCEPTION({d.resize(1);}, "only available for sequence");
    index = uint8_t(100); // The map if full already
    ASSERT_OR_EXCEPTION({d[index] = 50;}, "Cannot insert new element into map.");
}

TEST (CollectionTypes, string_key_map)
{
    MapType map(StringType(), primitive_type<uint16_t>(), 100);

    DynamicData d(map);
    EXPECT_EQ(0, d.size());
    EXPECT_EQ(100, d.bounds());

    StringType str_type;
    DynamicData index(str_type);
    for(uint16_t i = 0; i < 10; ++i)
    {
        index = std::to_string(i);
        d[index] = i;
    }

    EXPECT_EQ(10, d.size());

    for (size_t i = 0; i < d.size(); ++i)
    {
        index = std::to_string(i);
        EXPECT_EQ(d[index].value<uint16_t>(), i);
    }
}

TEST (CollectionTypes, string_hash)
{
    StringType str;
    DynamicData str1(str);
    DynamicData str2(str);
    DynamicData str3(str);
    str1 = "Hola";
    str2 = "Hola";
    str3 = "Hola_";
    EXPECT_EQ(str1.hash(), str2.hash());
    EXPECT_NE(str1.hash(), str3.hash());
    str3 = "Hola";
    str2 = "_Hola";
    EXPECT_NE(str1.hash(), str2.hash());
    EXPECT_EQ(str1.hash(), str3.hash());
}

TEST (CollectionTypes, multi_map_map_struct_key)
{
    StructType key_type("KeyStruct");
    key_type.add_member("st0", StringType());
    key_type.add_member("st1", primitive_type<uint32_t>());

    MapType simple_map(key_type, primitive_type<uint32_t>(), 5);
    MapType map_map(key_type, simple_map, 4);
    MapType map_map_map(key_type, map_map);

    DynamicData data(map_map_map);

    EXPECT_EQ(data.bounds(), 0);
    EXPECT_EQ(data.size(), 0);

    DynamicData index_1(key_type);
    DynamicData index_2(key_type);
    DynamicData index_3(key_type);
    for (size_t i = 0; i < 3; ++i)
    {
        index_1["st0"] = std::to_string(i);
        index_1["st1"] = uint32_t(i);
        data[index_1] = DynamicData(map_map);
        EXPECT_EQ(data[index_1].size(), 0);
        for (size_t j = 0; j < 4; ++j)
        {
            index_2["st0"] = std::to_string(j);
            index_2["st1"] = uint32_t(j);
            data[index_1][index_2] = DynamicData(simple_map);
            EXPECT_EQ(data[index_1][index_2].size(), 0);
            for (size_t k = 0; k < 5; ++k)
            {
                index_3["st0"] = std::to_string(k);
                index_3["st1"] = uint32_t(k);
                data[index_1][index_2][index_3] = static_cast<uint32_t>(i + j + k);
                uint32_t temp = data[index_1][index_2][index_3];
                EXPECT_EQ(data[index_1][index_2][index_3].value<uint32_t>(), static_cast<uint32_t>(i + j + k));
                EXPECT_EQ(temp, static_cast<uint32_t>(i + j + k));
                EXPECT_EQ(data[index_1][index_2].size(), k + 1);
            }
            EXPECT_EQ(data[index_1][index_2].bounds(), 5);
            EXPECT_EQ(data[index_1].size(), j + 1);
        }
        EXPECT_EQ(data[index_1].bounds(), 4);
        EXPECT_EQ(data.size(), i + 1);
    }
    EXPECT_EQ(data.bounds(), 0);
}

TEST (CollectionTypes, multi_map_struct_key)
{
    StructType key_type("KeyStruct");
    key_type.add_member("st0", primitive_type<float>());
    key_type.add_member("st1", primitive_type<uint32_t>());

    MapType simple_map(key_type, primitive_type<uint32_t>(), 5);
    MapType map_map(key_type, simple_map, 4);

    DynamicData data(map_map);

    EXPECT_EQ(data.bounds(), 4);
    EXPECT_EQ(data.size(), 0);

    DynamicData index_2(key_type);
    DynamicData index_3(key_type);
    for (size_t j = 0; j < 4; ++j)
    {
        index_2["st0"] = float(j);
        index_2["st1"] = uint32_t(j);
        data[index_2] = DynamicData(simple_map);
        EXPECT_EQ(data[index_2].size(), 0);
        for (size_t k = 0; k < 5; ++k)
        {
            index_3["st0"] = float(k);
            index_3["st1"] = uint32_t(k);
            data[index_2][index_3] = static_cast<uint32_t>(j + k);
            uint32_t temp = data[index_2][index_3];
            EXPECT_EQ(data[index_2][index_3].value<uint32_t>(), static_cast<uint32_t>(j + k));
            EXPECT_EQ(temp, static_cast<uint32_t>(j + k));
            EXPECT_EQ(data[index_2].size(), k + 1);
        }
        EXPECT_EQ(data[index_2].bounds(), 5);
        EXPECT_EQ(data.size(), j + 1);
    }
}

TEST (CollectionTypes, multi_map_simple_key)
{
    const DynamicType& key_type = primitive_type<int32_t>();

    MapType simple_map(key_type, primitive_type<uint32_t>(), 5);
    MapType map_map(key_type, simple_map, 4);

    DynamicData data(map_map);

    EXPECT_EQ(data.bounds(), 4);
    EXPECT_EQ(data.size(), 0);

    DynamicData index_2(key_type);
    DynamicData index_3(key_type);
    for (size_t j = 0; j < 4; ++j)
    {
        index_2 = int32_t(j);
        data[index_2] = DynamicData(simple_map);
        EXPECT_EQ(data[index_2].size(), 0);
        for (size_t k = 0; k < 5; ++k)
        {
            index_3 = int32_t(k);
            data[index_2][index_3] = static_cast<uint32_t>(j + k);
            uint32_t temp = data[index_2][index_3];
            EXPECT_EQ(data[index_2][index_3].value<uint32_t>(), static_cast<uint32_t>(j + k));
            EXPECT_EQ(temp, static_cast<uint32_t>(j + k));
            EXPECT_EQ(data[index_2].size(), k + 1);
        }
        EXPECT_EQ(data[index_2].bounds(), 5);
        EXPECT_EQ(data.size(), j + 1);
    }
}

TEST (CollectionTypes, map_struct_key)
{
    StructType key_type("KeyStruct");
    key_type.add_member("st0", primitive_type<float>());
    key_type.add_member("st1", primitive_type<uint32_t>());

    MapType simple_map(key_type, primitive_type<uint32_t>(), 3);

    DynamicData data(simple_map);

    EXPECT_EQ(data.bounds(), 3);
    EXPECT_EQ(data.size(), 0);

    DynamicData index(key_type);
    for (size_t i = 0; i < 3; ++i)
    {
        index["st0"] = float(i);
        index["st1"] = uint32_t(i);
        data[index] = uint32_t(i + 2);
        EXPECT_EQ(data.size(), i + 1);
    }

    for (size_t i = 0; i < 3; ++i)
    {
        index["st0"] = float(i);
        index["st1"] = uint32_t(i);
        EXPECT_EQ(data[index].value<uint32_t>(), uint32_t(i + 2));
    }
}
