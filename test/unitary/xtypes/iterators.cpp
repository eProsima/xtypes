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
#include <xtypes/Module.hpp>
#include <iostream>

#include <cmath>
#include <bitset>

using namespace eprosima::xtypes;

/********************************
 *        Iterators Tests       *
 ********************************/

TEST (Iterators, dynamic_data)
{
    //SECTION("StringType")
    {
        StringType string;
        DynamicData str1(string);

        str1.value<std::string>("Hola!");
        size_t idx = 0;
        for (ReadableDynamicDataRef&& elem : str1)
        {
            EXPECT_EQ(elem.value<char>(), str1[idx++].value<char>());
        }

        for (WritableDynamicDataRef&& elem : str1)
        {
            elem = 'X';
        }

        for (ReadableDynamicDataRef&& elem : str1)
        {
            EXPECT_EQ(elem.value<char>(), 'X');
        }
    }

    //SECTION("WStringType")
    {
        WStringType string;
        DynamicData str1(string);

        str1.value<std::wstring>(L"Hola!");

        size_t idx = 0;
        for (ReadableDynamicDataRef&& elem : str1)
        {
            EXPECT_EQ(elem.value<wchar_t>(), str1[idx++].value<wchar_t>());
        }

        for (WritableDynamicDataRef&& elem : str1)
        {
            elem = L'a';
        }

        for (ReadableDynamicDataRef&& elem : str1)
        {
            EXPECT_EQ(elem.value<wchar_t>(), L'a');
        }
    }

    //SECTION("ArrayType")
    {
        ArrayType array_type(primitive_type<int32_t>(), 10);
        DynamicData array(array_type);

        for (int i = 0; i < 10; ++i)
        {
            array[i] = 5 * i;
        }

        size_t idx = 0;
        int32_t check_sum = 0;
        for (ReadableDynamicDataRef&& elem : array)
        {
            EXPECT_EQ(elem.value<int32_t>(), array[idx++].value<int32_t>());
            check_sum += elem.value<int32_t>();
        }

        for (WritableDynamicDataRef&& elem : array)
        {
            elem = elem.value<int32_t>() * 2;
        }

        int32_t double_check_sum = 0;
        for (ReadableDynamicDataRef&& elem : array)
        {
            double_check_sum += elem.value<int32_t>();
        }
        ASSERT_EQ(check_sum * 2, double_check_sum);
    }

    //SECTION("SequenceType")
    {
        SequenceType seq_type(primitive_type<int32_t>());
        DynamicData seq(seq_type);

        for (int i = 0; i < 10; ++i)
        {
            seq.push(5 * i);
        }

        ReadableDynamicDataRef::Iterator it = seq.begin();
        WritableDynamicDataRef::Iterator wit = seq.begin();
        ASSERT_EQ((*it++).value<int32_t>(), 0);
        ASSERT_EQ((*wit++).value<int32_t>(), 0);
        ASSERT_EQ((*it).value<int32_t>(), 5);
        ASSERT_EQ((*wit).value<int32_t>(), 5);

        size_t idx = 0;
        int32_t check_sum = 0;
        for (ReadableDynamicDataRef&& elem : seq)
        {
            EXPECT_EQ(elem.value<int32_t>(), seq[idx++].value<int32_t>());
            check_sum += elem.value<int32_t>();
        }

        for (WritableDynamicDataRef&& elem : seq)
        {
            elem = elem.value<int32_t>() * 2;
        }

        int32_t double_check_sum = 0;
        for (ReadableDynamicDataRef&& elem : seq)
        {
            double_check_sum += elem.value<int32_t>();
        }
        ASSERT_EQ(check_sum * 2, double_check_sum);
    }

    //SECTION("StructType")
    {
        StructType my_struct("MyStruct");
        my_struct.add_member("my_int", primitive_type<int32_t>());
        my_struct.add_member("my_double", primitive_type<double>());

        DynamicData my_data(my_struct);
        my_data["my_int"] = 55;
        my_data["my_double"] = -23.44;

        //ReadableDynamicDataRef::MemberIterator it = my_data.citems().begin();
        auto it = my_data.citems().begin();
        auto wit = my_data.items().begin();
        ASSERT_EQ((*it++).member().name(), "my_int");
        ASSERT_EQ((*wit++).member().name(), "my_int");
        ASSERT_EQ((*it).member().name(), "my_double");
        ASSERT_EQ((*wit).member().name(), "my_double");

        for (ReadableDynamicDataRef::MemberPair&& elem : my_data.items())
        {
            switch(elem.kind())
            {
            case TypeKind::INT_32_TYPE:
                ASSERT_EQ(elem.member().name(), "my_int");
                ASSERT_EQ(elem.data().value<int32_t>(), 55);
                break;
            case TypeKind::FLOAT_64_TYPE:
                ASSERT_EQ(elem.member().name(), "my_double");
                ASSERT_EQ(elem.data().value<double>(), -23.44);
                break;
            default:
                break;
            }
        }

        for (WritableDynamicDataRef::MemberPair&& elem : my_data.items())
        {
            switch(elem.kind())
            {
            case TypeKind::INT_32_TYPE:
                elem.data() = elem.data().value<int32_t>() * 2;
                break;
            case TypeKind::FLOAT_64_TYPE:
                elem.data() = elem.data().value<double>() * 2;
                break;
            default:
                break;
            }
        }

        for (ReadableDynamicDataRef::MemberPair&& elem : my_data.items())
        {
            switch(elem.kind())
            {
            case TypeKind::INT_32_TYPE:
                ASSERT_EQ(elem.member().name(), "my_int");
                ASSERT_EQ(elem.data().value<int32_t>(), 110);
                break;
            case TypeKind::FLOAT_64_TYPE:
                ASSERT_EQ(elem.member().name(), "my_double");
                ASSERT_EQ(elem.data().value<double>(), -46.88);
                break;
            default:
                break;
            }
        }
    }
}

TEST (Iterators, for_each_types)
{
    StructType l2 = StructType("Level2")
        .add_member("l2m1", primitive_type<uint32_t>())
        .add_member("l2m2", primitive_type<float>())
        .add_member("l2m3", StringType())
        .add_member("l2m4", WStringType());

    StructType l1 = StructType("Level1")
        .add_member("l1m1", SequenceType(primitive_type<uint32_t>()))
        .add_member("l1m2", SequenceType(l2))
        .add_member("l1m3", ArrayType(primitive_type<uint32_t>(), 2))
        .add_member("l1m4", ArrayType(l2, 4))
        .add_member("l1m5", l2);

    StructType l0 = StructType("Level0")
        .add_member("l0m1", l1)
        .add_member("l0m2", l2);

    std::vector<std::string> expected_output =
    {
        "Level0",
        "Level1",
        "sequence_uint32_t",
        "uint32_t",
        "sequence_Level2",
        "Level2",
        "uint32_t",
        "float",
        "std::string",
        "std::wstring",
        "array_2_uint32_t",
        "uint32_t",
        "array_4_Level2",
        "Level2",
        "uint32_t",
        "float",
        "std::string",
        "std::wstring",
        "Level2",
        "uint32_t",
        "float",
        "std::string",
        "std::wstring",
        "Level2",
        "uint32_t",
        "float",
        "std::string",
        "std::wstring",
    };

    size_t i = 0;

    l0.for_each([&](const DynamicType::TypeNode& node)
    {
        EXPECT_EQ(expected_output[i], node.type().name());
        i++;
    });

    EXPECT_EQ(i, expected_output.size());
}
