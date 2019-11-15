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
#include <xtypes/idl/idl.hpp>
#include <xtypes/DynamicData.hpp>
#include <iostream>

using namespace eprosima::xtypes;
using namespace eprosima::xtypes::idl;


TEST (IDLParser, simple_struct_test)
{
    std::map<std::string, DynamicType::Ptr> result;
    result = parse(R"(
        struct SimpleStruct
        {
            boolean my_bool;
            int8 my_int8;
            uint8 my_uint8;
            int16 my_int16;
            uint16 my_uint16;
            int32 my_int32;
            uint32 my_uint32;
            int64 my_int64;
            uint64 my_uint64;
            float my_float;
            double my_double;
            long double my_long_double;
            char my_char;
            wchar my_wchar;
            string my_string;
            wstring my_wstring;
        };
                   )");
    EXPECT_EQ(1, result.size());

    const DynamicType* my_struct = result["SimpleStruct"].get();
    DynamicData data(*my_struct);

    data["my_bool"] = true;
    data["my_int8"] = 'c';
    data["my_uint8"] = static_cast<uint8_t>(55);
    data["my_int16"] = static_cast<int16_t>(-5);
    data["my_uint16"] = static_cast<uint16_t>(6);
    data["my_int32"] = static_cast<int32_t>(-5);
    data["my_uint32"] = static_cast<uint32_t>(6);
    data["my_int64"] = static_cast<int64_t>(-5);
    data["my_uint64"] = static_cast<uint64_t>(6);
    data["my_float"] = 5.55f;
    data["my_double"] = 5.55;
    data["my_long_double"] = 5.55l;
    data["my_char"] = 'e';
    data["my_wchar"] = L'e';
    data["my_string"].string("It works!");
    data["my_wstring"].wstring(L"It works!");
    EXPECT_TRUE(data["my_bool"].value<bool>());
    EXPECT_EQ('c', data["my_int8"].value<char>());
    EXPECT_EQ(55, data["my_uint8"].value<uint8_t>());
    EXPECT_EQ(-5, data["my_int16"].value<int16_t>());
    EXPECT_EQ(6, data["my_uint16"].value<uint16_t>());
    EXPECT_EQ(-5, data["my_int32"].value<int32_t>());
    EXPECT_EQ(6, data["my_uint32"].value<uint32_t>());
    EXPECT_EQ(-5, data["my_int64"].value<int64_t>());
    EXPECT_EQ(6, data["my_uint64"].value<uint64_t>());
    EXPECT_EQ(5.55f, data["my_float"].value<float>());
    EXPECT_EQ(5.55, data["my_double"].value<double>());
    EXPECT_EQ(5.55l, data["my_long_double"].value<long double>());
    EXPECT_EQ('e', data["my_char"].value<char>());
    EXPECT_EQ(L'e', data["my_wchar"].value<wchar_t>());
    EXPECT_EQ("It works!", data["my_string"].value<std::string>());
    EXPECT_EQ(L"It works!", data["my_wstring"].value<std::wstring>());
}

TEST (IDLParser, array_sequence_struct_test)
{
    std::map<std::string, DynamicType::Ptr> result;
    result = parse(R"(
        struct SimpleStruct
        {
            boolean my_bool_5[5];
            int8 my_int8_3_2[3][2];
            string<16> my_string16;
            wstring<32> my_wstring32;
            sequence<int32> my_int_seq;
            sequence<char, 6> my_char6_seq;
        };
                   )");
    EXPECT_EQ(1, result.size());

    const DynamicType* my_struct = result["SimpleStruct"].get();
    DynamicData data(*my_struct);

    data["my_bool_5"][0] = true;
    data["my_bool_5"][1] = true;
    data["my_bool_5"][2] = true;
    data["my_bool_5"][3] = false;
    data["my_bool_5"][4] = true;
    EXPECT_TRUE(data["my_bool_5"][0].value<bool>());
    EXPECT_TRUE(data["my_bool_5"][1].value<bool>());
    EXPECT_TRUE(data["my_bool_5"][2].value<bool>());
    EXPECT_FALSE(data["my_bool_5"][3].value<bool>());
    EXPECT_TRUE(data["my_bool_5"][4].value<bool>());

    data["my_int8_3_2"][0][0] = 'a';
    data["my_int8_3_2"][0][1] = 'b';
    data["my_int8_3_2"][1][0] = 'c';
    data["my_int8_3_2"][1][1] = 'd';
    data["my_int8_3_2"][2][0] = 'e';
    data["my_int8_3_2"][2][1] = 'f';
    EXPECT_EQ(data["my_int8_3_2"][0][0].value<char>(), 'a');
    EXPECT_EQ(data["my_int8_3_2"][0][1].value<char>(), 'b');
    EXPECT_EQ(data["my_int8_3_2"][1][0].value<char>(), 'c');
    EXPECT_EQ(data["my_int8_3_2"][1][1].value<char>(), 'd');
    EXPECT_EQ(data["my_int8_3_2"][2][0].value<char>(), 'e');
    EXPECT_EQ(data["my_int8_3_2"][2][1].value<char>(), 'f');

    EXPECT_EQ(data["my_string16"].bounds(), 16);
    // data["my_string16"] = "0123456789abcdefghijklmnopqrstuvwxyz" ;
    // EXPECT_EQ(data["my_string16"].size(), 16);
    data["my_string16"] = "0123456789" ;
    EXPECT_EQ(data["my_string16"].size(), 10);
    EXPECT_EQ(data["my_string16"].bounds(), 16);
    EXPECT_EQ(data["my_string16"].value<std::string>(), "0123456789");

    EXPECT_EQ(data["my_wstring32"].bounds(), 32);
    // data["my_wstring32"] = L"0123456789abcdefghijklmnñopqrstuvwxyz" ;
    // EXPECT_EQ(data["my_wstring32"].size(), 32);
    data["my_wstring32"] = L"0123456789" ;
    EXPECT_EQ(data["my_wstring32"].size(), 10);
    EXPECT_EQ(data["my_wstring32"].bounds(), 32);
    EXPECT_EQ(data["my_wstring32"].value<std::wstring>(), L"0123456789");

    for (int32_t i = 0; i < 300; ++i)
    {
        data["my_int_seq"].push(i);
    }
    for (int32_t i = 0; i < 300; ++i)
    {
        EXPECT_EQ(data["my_int_seq"][i].value<int32_t>(), i);
    }
    EXPECT_EQ(data["my_int_seq"].size(), 300);

    EXPECT_EQ(data["my_char6_seq"].bounds(), 6);
    for (int32_t i = 0; i < 7; ++i)
    {
        if (i < 6)
        {
            data["my_char6_seq"].push(static_cast<char>(i));
        }
        else
        {
            ASSERT_DEATH(data["my_char6_seq"][i].push(static_cast<char>(i)), "Assertion");
        }
    }
    for (int32_t i = 0; i < 7; ++i)
    {
        if (i < 6)
        {
            EXPECT_EQ(data["my_char6_seq"][i].value<char>(), static_cast<char>(i));
        }
        else
        {
            ASSERT_DEATH(data["my_char6_seq"][i], "Assertion");
        }
    }
    EXPECT_EQ(data["my_char6_seq"].size(), 6);
    EXPECT_EQ(data["my_char6_seq"].bounds(), 6);

}

TEST (IDLParser, inner_struct_test)
{
    std::map<std::string, DynamicType::Ptr> result;
    result = parse(R"(
        struct InnerStruct
        {
            string message;
        };

        struct SuperStruct
        {
            InnerStruct inner;
        };

        struct RecursiveStruct
        {
            RecursiveStruct rec;
        };
                   )");
    EXPECT_EQ(3, result.size());

    const DynamicType* my_struct = result["SuperStruct"].get();
    DynamicData data(*my_struct);
    DynamicData rec_data(*result["RecursiveStruct"].get());

    data["inner"]["message"].string("It works!");
    EXPECT_EQ("It works!", data["inner"]["message"].value<std::string>());
}

TEST (IDLParser, not_yet_supported)
//TEST (IDLParser, DISABLED_not_yet_supported)
{
    std::map<std::string, DynamicType::Ptr> result;
    try
    {
        result = parse(R"(
            const uint32 MAX_SIZE = 32 / 2;

            module A
            {
                struct MyStruct
                {
                    uint64 my_uint64;
                };
            };

            struct ForwardStruct;

            struct FutureStruct
            {
                map<int32, string, 5> my_map;
                A::MyStruct scoped_struct;
                ForwardStruct fwd_struct;
            };

            struct Forward
            {
                string<MAX_SIZE> my_string;
            };

            enum MyEnum
            {
                AAA,
                BBB,
                CCC
            };

            union ForwardUnion;

            union MyUnion switch (MyEnum)
            {
                case AAA: string str_a;
                case BBB: wstring wstr_b;
                case CCC: ForwardUnion union_c;
            };

            union ForwardUnion switch (octet)
            {
                case 0: int32 my_int32;
                case 1: uint64 my_uint64;
                default: string my_string;
            };

            bitset MyBitset
            {
                bitfield<3> a;
                bitfield<1> b;
                bitfield<4>;
                bitfield<10, long> c;
            };

            bitmask MyBitmask
            {
                flag0,
                flag1,
                @position(5) flag5,
                flag6
            };
                       )");
    }
    catch(const Parser::exception& exc)
    {
        std::cout << exc.what() << std::endl;
    }
    /*
    EXPECT_EQ(1, result.size());

    const DynamicType* my_struct = result["FutureStruct"].get();
    DynamicData data(*my_struct);
    */
}

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
