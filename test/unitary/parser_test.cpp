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
#include <xtypes/idl/IDLParser.hpp>
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
    EXPECT_EQ(5.55f, data["my_float"].value<float>());
    EXPECT_EQ("It works!", data["my_string"].value<std::string>());
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
            sequence<char, 66> my_char66_seq;
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

/*
TEST (StructType, build_one_of_each_primitive_type)
{
    StructType st("struct_name");
    EXPECT_EQ("struct_name", st.name());
    EXPECT_EQ(TypeKind::STRUCTURE_TYPE, st.kind());
    size_t mem_size = 0;
    st.add_member(Member("bool", primitive_type<bool>()));
    mem_size+=sizeof(bool);
    EXPECT_EQ(mem_size, st.memory_size());
    st.add_member(Member("uint8_t", primitive_type<uint8_t>()));
    mem_size+=sizeof(uint8_t);
    EXPECT_EQ(mem_size, st.memory_size());
    st.add_member(Member("int16_t", primitive_type<int16_t>()));
    mem_size+=sizeof(int16_t);
    EXPECT_EQ(mem_size, st.memory_size());
    st.add_member(Member("uint16_t", primitive_type<uint16_t>()));
    mem_size+=sizeof(uint16_t);
    EXPECT_EQ(mem_size, st.memory_size());
    st.add_member(Member("int32_t", primitive_type<int32_t>()));
    mem_size+=sizeof(int32_t);
    EXPECT_EQ(mem_size, st.memory_size());
    st.add_member(Member("uint32_t", primitive_type<uint32_t>()));
    mem_size+=sizeof(uint32_t);
    EXPECT_EQ(mem_size, st.memory_size());
    st.add_member(Member("int64_t", primitive_type<int64_t>()));
    mem_size+=sizeof(int64_t);
    EXPECT_EQ(mem_size, st.memory_size());
    st.add_member(Member("uint64_t", primitive_type<uint64_t>()));
    mem_size+=sizeof(uint64_t);
    EXPECT_EQ(mem_size, st.memory_size());
    st.add_member(Member("float", primitive_type<float>()));
    mem_size+=sizeof(float);
    EXPECT_EQ(mem_size, st.memory_size());
    st.add_member(Member("double", primitive_type<double>()));
    mem_size+=sizeof(double);
    EXPECT_EQ(mem_size, st.memory_size());
    st.add_member(Member("long_double", primitive_type<long double>()));
    mem_size+=sizeof(long double);
    EXPECT_EQ(mem_size, st.memory_size());
}
*/

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
