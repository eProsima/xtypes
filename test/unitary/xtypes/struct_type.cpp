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

using namespace std;
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

static const std::string INNER_STRING_VALUE = "lay_down_and_cry";
static const std::string INNER_SEQUENCE_STRING = "another_prick_in_the_wall";
static const std::string SECOND_INNER_STRING = "paint_it_black";

static const float STRUCTS_SIZE = 1E1;

/**********************************
 *        StructType Tests        *
 **********************************/

TEST (StructType, primitive_struct)
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

TEST (StructType, cascade_api_and_copy)
{
    StructType st("struct_name");
    st.add_member(
        Member("bool", primitive_type<bool>())).add_member(
        Member("uint8_t", primitive_type<uint8_t>())).add_member(
        Member("int16_t", primitive_type<int16_t>())).add_member(
        Member("uint16_t", primitive_type<uint16_t>())).add_member(
        Member("int32_t", primitive_type<int32_t>())).add_member(
        Member("uint32_t", primitive_type<uint32_t>())).add_member(
        Member("int64_t", primitive_type<int64_t>())).add_member(
        Member("uint64_t", primitive_type<uint64_t>())).add_member(
        Member("float", primitive_type<float>())).add_member(
        Member("double", primitive_type<double>())).add_member(
        Member("long_double", primitive_type<long double>()));

    size_t mem_size = 0;
    mem_size+=sizeof(bool);
    mem_size+=sizeof(uint8_t);
    mem_size+=sizeof(int16_t);
    mem_size+=sizeof(uint16_t);
    mem_size+=sizeof(int32_t);
    mem_size+=sizeof(uint32_t);
    mem_size+=sizeof(int64_t);
    mem_size+=sizeof(uint64_t);
    mem_size+=sizeof(float);
    mem_size+=sizeof(double);
    mem_size+=sizeof(long double);

    StructType cp = st;
    EXPECT_EQ("struct_name", cp.name());
    EXPECT_EQ(mem_size, cp.memory_size());
}

TEST (StructType, self_assign)
{
    StructType st("struct_name");
    st.add_member(
        Member("long_double", primitive_type<long double>())).add_member(
        Member("uint64_t", primitive_type<uint64_t>())).add_member(
        Member("uint8_t", primitive_type<uint8_t>()));

    StructType in("struct_name");
    in.add_member(
        Member("long_double", primitive_type<long double>())).add_member(
        Member("uint64_t", primitive_type<uint64_t>())).add_member(
        Member("uint8_t", primitive_type<uint32_t>()));

    st.add_member(Member("in_member_name", in));
    st.add_member(Member("selfassign_member_name", st));
    EXPECT_EQ(TypeKind::STRUCTURE_TYPE, st.member("in_member_name").type().kind());
    EXPECT_EQ(TypeKind::STRUCTURE_TYPE, st.member("selfassign_member_name").type().kind());
    EXPECT_EQ(TypeKind::FLOAT_128_TYPE,
        static_cast<const StructType&>(st.member("selfassign_member_name").type()).member("long_double").type().kind());
    EXPECT_EQ(TypeKind::UINT_64_TYPE,
        static_cast<const StructType&>(st.member("selfassign_member_name").type()).member("uint64_t").type().kind());
    EXPECT_EQ(TypeKind::UINT_8_TYPE,
        static_cast<const StructType&>(st.member("selfassign_member_name").type()).member("uint8_t").type().kind() );
    size_t mem_size_in = 0;
    mem_size_in+=sizeof(long double);
    mem_size_in+=sizeof(uint64_t);
    mem_size_in+=sizeof(uint32_t);
    EXPECT_EQ(mem_size_in, st.member("in_member_name").type().memory_size());

    mem_size_in+=sizeof(long double);
    mem_size_in+=sizeof(uint64_t);
    mem_size_in+=sizeof(uint8_t);
    EXPECT_EQ(mem_size_in, st.member("selfassign_member_name").type().memory_size());
}

TEST (StructType, type_verify_test)
{
    StructType st("struct_name");
    st.add_member(
        Member("bool", primitive_type<bool>())).add_member(
        Member("uint8_t", primitive_type<uint8_t>())).add_member(
        Member("int16_t",  primitive_type<int16_t>())).add_member(
        Member("uint16_t",  primitive_type<uint16_t>())).add_member(
        Member("int32_t",  primitive_type<int32_t>())).add_member(
        Member("uint32_t", primitive_type<uint32_t>())).add_member(
        Member("int64_t", primitive_type<int64_t>())).add_member(
        Member("uint64_t", primitive_type<uint64_t>())).add_member(
        Member("float",  primitive_type<float>())).add_member(
        Member("double", primitive_type<double>())).add_member(
        Member("long double", primitive_type<long double>())).add_member(
        Member("char", primitive_type<char>())).add_member(
        Member("char16_t", primitive_type<wchar_t>()));


    EXPECT_EQ(TypeKind::BOOLEAN_TYPE, st.member("bool").type().kind());
    EXPECT_EQ(TypeKind::UINT_8_TYPE, st.member("uint8_t").type().kind());
    EXPECT_EQ(TypeKind::INT_16_TYPE , st.member("int16_t").type().kind());
    EXPECT_EQ(TypeKind::UINT_16_TYPE, st.member("uint16_t").type().kind());
    EXPECT_EQ(TypeKind::INT_32_TYPE, st.member("int32_t").type().kind());
    EXPECT_EQ(TypeKind::UINT_32_TYPE, st.member("uint32_t").type().kind());
    EXPECT_EQ(TypeKind::INT_64_TYPE, st.member("int64_t").type().kind());
    EXPECT_EQ(TypeKind::UINT_64_TYPE, st.member("uint64_t").type().kind());
    EXPECT_EQ(TypeKind::FLOAT_32_TYPE, st.member("float").type().kind());
    EXPECT_EQ(TypeKind::FLOAT_64_TYPE, st.member("double").type().kind());
    EXPECT_EQ(TypeKind::FLOAT_128_TYPE, st.member("long double").type().kind());
    EXPECT_EQ(TypeKind::CHAR_8_TYPE, st.member("char").type().kind());
    EXPECT_EQ(TypeKind::CHAR_16_TYPE, st.member("char16_t").type().kind());
    EXPECT_EQ(TypeKind::STRUCTURE_TYPE, st.kind());

    EXPECT_NE(0, uint32_t(TypeKind::PRIMITIVE_TYPE) & uint32_t(st.member("bool").type().kind()));
    EXPECT_NE(0, uint32_t(TypeKind::PRIMITIVE_TYPE) & uint32_t(st.member("uint8_t").type().kind()));
    EXPECT_NE(0, uint32_t(TypeKind::PRIMITIVE_TYPE) & uint32_t(st.member("int16_t").type().kind()));
    EXPECT_NE(0, uint32_t(TypeKind::PRIMITIVE_TYPE) & uint32_t(st.member("uint16_t").type().kind()));
    EXPECT_NE(0, uint32_t(TypeKind::PRIMITIVE_TYPE) & uint32_t(st.member("int32_t").type().kind()));
    EXPECT_NE(0, uint32_t(TypeKind::PRIMITIVE_TYPE) & uint32_t(st.member("uint32_t").type().kind()));
    EXPECT_NE(0, uint32_t(TypeKind::PRIMITIVE_TYPE) & uint32_t(st.member("int64_t").type().kind()));
    EXPECT_NE(0, uint32_t(TypeKind::PRIMITIVE_TYPE) & uint32_t(st.member("uint64_t").type().kind()));
    EXPECT_NE(0, uint32_t(TypeKind::PRIMITIVE_TYPE) & uint32_t(st.member("float").type().kind()));
    EXPECT_NE(0, uint32_t(TypeKind::PRIMITIVE_TYPE) & uint32_t(st.member("double").type().kind()));
    EXPECT_NE(0, uint32_t(TypeKind::PRIMITIVE_TYPE) & uint32_t(st.member("long double").type().kind()));
    EXPECT_NE(0, uint32_t(TypeKind::PRIMITIVE_TYPE) & uint32_t(st.member("char").type().kind()));
    EXPECT_NE(0, uint32_t(TypeKind::PRIMITIVE_TYPE) & uint32_t(st.member("char16_t").type().kind()));
    EXPECT_NE(0, uint32_t(TypeKind::STRUCTURE_TYPE) & uint32_t(st.kind()));

    DynamicData d(st);

    d["bool"].value<bool>(true);
    d["uint8_t"].value<uint8_t>(UINT8);
    d["int16_t"].value<int16_t>(INT16);
    d["uint16_t"].value<uint16_t>(UINT16);
    d["int32_t"].value<int32_t>(INT32);
    d["uint32_t"].value<uint32_t>(UINT32);
    d["int64_t"].value<int64_t>(INT64);
    d["uint64_t"].value<uint64_t>(UINT64);
    d["float"].value<float>(FLOAT);
    d["double"].value<double>(DOUBLE);
    d["long double"].value<long double>(LDOUBLE);
    d["char"].value<char>(CHAR);
    d["char16_t"].value<wchar_t>(WCHAR);

    EXPECT_EQ(true, d["bool"].value<bool>());
    EXPECT_EQ(UINT8, d["uint8_t"].value<uint8_t>());
    EXPECT_EQ(INT16, d["int16_t"].value<int16_t>());
    EXPECT_EQ(UINT16,d["uint16_t"].value<uint16_t>());
    EXPECT_EQ(INT32, d["int32_t"].value<int32_t>());
    EXPECT_EQ(UINT32, d["uint32_t"].value<uint32_t>());
    EXPECT_EQ(INT64, d["int64_t"].value<int64_t>());
    EXPECT_EQ(UINT64,d["uint64_t"].value<uint64_t>());
    EXPECT_EQ( float(FLOAT) , d["float"].value<float>());
    EXPECT_EQ( double(DOUBLE) , d["double"].value<double>());
    long double ld = LDOUBLE;
    EXPECT_EQ( ld , d["long double"].value<long double>());
    EXPECT_EQ( CHAR , d["char"].value<char>());
    EXPECT_EQ( WCHAR , d["char16_t"].value<wchar_t>());

}

TEST (StructType, empty_struct_data)
{
    StructType empty("empty_struct");
    DynamicData empty_data(empty);
    EXPECT_EQ(0, empty.memory_size());
    ASSERT_DEATH(empty_data[0], "index < size()");
}

DynamicData create_dynamic_data(
        StructType& the_struct,
        StructType& inner_struct,
        StructType& second_inner_struct)
{
    second_inner_struct.add_member(
        Member("second_inner_string", StringType())).add_member(
        Member("second_inner_uint32_t", primitive_type<uint32_t>())).add_member(
        Member("second_inner_array", ArrayType(primitive_type<uint8_t>(), 10)));
    StringType st;
    inner_struct.add_member(
        Member("inner_string", st)).add_member(
        Member("inner_float", primitive_type<float>())).add_member(
        Member("inner_sequence_string", SequenceType(st))).add_member(
        Member("inner_sequence_struct", SequenceType(second_inner_struct)));

    the_struct.add_member(
        Member("bool", primitive_type<bool>())).add_member(
        Member("uint8_t", primitive_type<uint8_t>())).add_member(
        Member("int16_t", primitive_type<int16_t>())).add_member(
        Member("uint16_t", primitive_type<uint16_t>())).add_member(
        Member("int32_t", primitive_type<int32_t>())).add_member(
        Member("uint32_t", primitive_type<uint32_t>())).add_member(
        Member("int64_t", primitive_type<int64_t>())).add_member(
        Member("uint64_t", primitive_type<uint64_t>())).add_member(
        Member("float", primitive_type<float>())).add_member(
        Member("double", primitive_type<double>())).add_member(
        Member("long_double", primitive_type<long double>())).add_member(
        Member("array", ArrayType(ArrayType(primitive_type<long double>(), 10), 10))).add_member(
        Member("sequence", SequenceType(inner_struct)));

    DynamicData the_data(the_struct);
    the_data["bool"].value(true);
    the_data["uint8_t"].value<uint8_t>(UINT8);
    the_data["int16_t"].value<int16_t>(INT16);
    the_data["uint16_t"].value<uint16_t>(UINT16);
    the_data["int32_t"].value<int32_t>(INT32);
    the_data["uint32_t"].value<uint32_t>(UINT32);
    the_data["int64_t"].value<int64_t>(INT64);
    the_data["uint64_t"].value<uint64_t>(UINT64);
    the_data["float"].value<float>(FLOAT);
    the_data["double"].value<double>(DOUBLE);

    the_data["long_double"].value<>(LDOUBLE);

    for(int i = 0; i < STRUCTS_SIZE; ++i) // creating "sequence"
    {
        DynamicData tmp_data(inner_struct);
        tmp_data["inner_string"] = INNER_STRING_VALUE;
        tmp_data["inner_float"].value<float>(FLOAT);
        for (int j = 0; j < STRUCTS_SIZE; ++j) // creating "sequence.inner_sequence_string"
        {
            tmp_data["inner_sequence_string"].push<string>(INNER_SEQUENCE_STRING);
        }

        for (int j = 0; j < STRUCTS_SIZE; ++j) // creating "sequence.inner_sequence_struct"
        {
            DynamicData tmp_inner_data(second_inner_struct);
            tmp_inner_data["second_inner_string"] = SECOND_INNER_STRING;
            tmp_inner_data["second_inner_uint32_t"].value<uint32_t>(UINT32);
            for(int k = 0; k < STRUCTS_SIZE; ++k) //creating "sequence.inner_sequence_struct.second_inner_array"
            {
                tmp_inner_data["second_inner_array"][k].value<uint8_t>(UINT8);
            }
            tmp_data["inner_sequence_struct"].push(tmp_inner_data);
        }
        for(int j = 0; j < STRUCTS_SIZE; ++j)
        {
            for(int k = 0; k < STRUCTS_SIZE; ++k)
            {
                the_data["array"][j][k].value<long double>(LDOUBLE);
            }
        }
        the_data["sequence"].push(tmp_data);
    }

    return the_data;
}

TEST (StructType, complex_and_member_access)
{
    StructType the_struct("the_struct");
    StructType inner_struct("inner_struct");
    StructType second_inner_struct("second_inner_struct");

    DynamicData the_data = create_dynamic_data(the_struct, inner_struct, second_inner_struct);

    EXPECT_EQ(UINT32, the_data["uint32_t"].value<uint32_t>());
    EXPECT_EQ(INT32, the_data["int32_t"].value<int32_t>());
    EXPECT_EQ(UINT16, the_data["uint16_t"].value<uint16_t>());
    EXPECT_EQ(INT16, the_data["int16_t"].value<int16_t>());
    EXPECT_EQ(true, the_data["bool"].value<bool>());
    EXPECT_EQ(UINT8, the_data["uint8_t"].value<uint8_t>());
    EXPECT_EQ(INT64, the_data["int64_t"].value<int64_t>());
    EXPECT_EQ(UINT64, the_data["uint64_t"].value<uint64_t>());
    EXPECT_EQ(FLOAT, the_data["float"].value<float>());
    EXPECT_EQ(DOUBLE, the_data["double"].value<double>());
    EXPECT_EQ(LDOUBLE, the_data["long_double"].value<long double>());

    EXPECT_EQ(true, the_data[0].value<bool>());
    EXPECT_EQ(UINT8, the_data[1].value<uint8_t>());
    EXPECT_EQ(INT16, the_data[2].value<int16_t>());
    EXPECT_EQ(UINT16, the_data[3].value<uint16_t>());
    EXPECT_EQ(INT32, the_data[4].value<int32_t>());
    EXPECT_EQ(UINT32, the_data[5].value<uint32_t>());
    EXPECT_EQ(INT64, the_data[6].value<int64_t>());
    EXPECT_EQ(UINT64, the_data[7].value<uint64_t>());
    EXPECT_EQ(FLOAT, the_data[8].value<float>());
    EXPECT_EQ(DOUBLE, the_data[9].value<double>());
    EXPECT_EQ(LDOUBLE, the_data[10].value<long double>());

    for (size_t i = 0; i < STRUCTS_SIZE; ++i)
    {
        EXPECT_EQ(the_data["sequence"][i]["inner_string"].value<std::string>(), INNER_STRING_VALUE);
        EXPECT_EQ(the_data[12][i][0].value<std::string>(), INNER_STRING_VALUE);
        EXPECT_EQ(the_data["sequence"][i]["inner_float"].value<float>(), FLOAT);
        EXPECT_EQ(the_data[12][i][1].value<float>(), FLOAT);
        for (size_t j = 0; j < STRUCTS_SIZE; ++j)
        {
            EXPECT_EQ(the_data["sequence"][i]["inner_sequence_string"][j].value<string>(), INNER_SEQUENCE_STRING);
            EXPECT_EQ(the_data[12][i][2][j].value<string>(), INNER_SEQUENCE_STRING);
        }

        for (int j = 0; j < STRUCTS_SIZE; ++j)
        {
            EXPECT_EQ(
                the_data["sequence"][i]["inner_sequence_struct"][j]["second_inner_string"].value<std::string>(),
                SECOND_INNER_STRING);
            EXPECT_EQ(
                the_data["sequence"][i]["inner_sequence_struct"][j]["second_inner_uint32_t"].value<uint32_t>(),
                UINT32);
            EXPECT_EQ(
                the_data[12][i][3][j][0].value<std::string>(),
                SECOND_INNER_STRING);
            EXPECT_EQ(
                the_data[12][i][3][j][1].value<uint32_t>(),
                UINT32);
            for(int k = 0; k < STRUCTS_SIZE; ++k)
            {
                EXPECT_EQ(
                    the_data["sequence"][i]["inner_sequence_struct"][j]["second_inner_array"][k].value<uint8_t>(),
                    UINT8);
                EXPECT_EQ(
                    the_data[12][i][3][j][2][k].value<uint8_t>(),
                    UINT8);
            }
        }

        for(int j = 0; j < STRUCTS_SIZE; ++j)
        {
            EXPECT_EQ(the_data["array"][i][j].value<long double>(), LDOUBLE);
            EXPECT_EQ(the_data[11][i][j].value<long double>(), LDOUBLE);
        }
    }
}

TEST (StructType, simple_string_sequence_struct)
{
    StructType st("st");
    st.add_member(
    Member("seq", SequenceType(StringType())));

    DynamicData the_data(st);

    StringType str;
    DynamicData dstr(str);
    dstr = "all_this_stuff";

    the_data["seq"].push(dstr);
    EXPECT_EQ("all_this_stuff", the_data["seq"][0].value<std::string>());
}
