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

using namespace std;
using namespace eprosima::xtypes;


#define UINT8 250
#define INT16 -32760
#define UINT16 65530
#define INT32 -2147483640
#define UINT32 4294967290
#define INT64 -9223372036854775800
#define UINT64 18446744073709551610ULL
#define FLOAT 3.1415927410125732421875f
#define DOUBLE 3.1415926535897931159979631875
#define LDOUBLE 3.14159265358979321159979631875
#define CHAR 'f'
#define WCHAR 34590

#define INNER_STRING_VALUE "lay_down_and_cry"
#define INNER_SEQUENCE_STRING "another_prick_in_the_wall"
#define SECOND_INNER_STRING "paint_it_black"

#define STRUCTS_SIZE 1E1
#define CHECKS_NUMBER 1E2
#define TEN 10

/********************************
 *        DynamicType Tests        *
 ********************************/

TEST (StructType , verify_constructor_and_size)
{
    StructType st("struct_name");
    EXPECT_EQ("struct_name", st.name());
    st.add_member(Member("int", primitive_type<int>()));
    EXPECT_EQ(4, st.memory_size());
}

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

TEST (StructType, cascade_add_member_and_copy)
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

}

TEST (DynamicData, primitive_types)
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

TEST (DynamicData, long_random_sequence)
{
    SequenceType st(primitive_type<double>());

    DynamicData d(st);
    srand48(time(0));

    for(int i = 0; i < 65000; ++i)
    {
        double r = lrand48()/double(RAND_MAX);
        d.push(r);
        EXPECT_EQ(r, d[i].value<double>());
    }
}

TEST (DynamicData, sequence)
{
    StructType s("struct");
    s.add_member(Member("sequence_1", SequenceType(primitive_type<char>())));
    s.add_member(Member("sequence_2", SequenceType(primitive_type<char>())));
    {
        DynamicData d(s);
        d["sequence_1"].push('a');
        d["sequence_1"].push('b');
        d["sequence_1"].push('c');
        d["sequence_1"].push('d');
        d["sequence_1"].push('e');
        d["sequence_2"].push(d["sequence_1"]);
    }
}

DynamicData cdd(StructType &st)
{
    StringType stri_t;
    SequenceType sequ_t(stri_t);
    StructType stru_t("just_a_struct");
    stru_t.add_member(
    Member("sequence", sequ_t));

    StructType sec_stru("another_struct");
    sec_stru.add_member(
        Member("int", primitive_type<uint32_t>()));

    st.add_member("struct", stru_t).add_member("other_struct", sec_stru);

    DynamicData dd(st);
    for(int i = 0; i < 1E1; ++i)
        dd["struct"]["sequence"].push<string>("checking");
    return dd;
}

DynamicData ret_dyn_data(StructType &st, int i )
{
    switch(i){
        case 1:
            st.add_member(
                Member("int", primitive_type<int32_t>()));
            break;
        case 2:
            st.add_member(
                Member("array", ArrayType(primitive_type<uint32_t>(), 10)));
            break;
        default:
            break;
    }
    DynamicData dt(st);
    switch(i){
        case 1:
            dt["int"].value<int32_t>(32);
            break;
        case 2:
            for(size_t j = 0; j < 10; ++j)
                dt["array"][i].value<uint32_t>(32);
            break;
        default:
            break;
    }
    return dt;
}

TEST (DynamicData, cp)
{
    StructType ist("int_st");
    DynamicData idt = ret_dyn_data(ist, 1);

    StructType ast("arr_st");
    DynamicData adt = ret_dyn_data(ast, 2);
}


DynamicData create_dynamic_data(long double pi, StructType& the_struct, StructType& inner_struct, StructType& second_inner_struct)
{
//    StructType inner_struct("inner_struct");
//    StructType second_inner_struct("second_inner_struct");
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

    the_data["long_double"].value<>(pi);

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

TEST (DynamicData, cascade_construction)
{
    long double pi = LDOUBLE;
    StructType the_struct("the_struct");
    StructType inner_struct("inner_struct");
    StructType second_inner_struct("second_inner_struct");

    DynamicData the_data = create_dynamic_data(pi, the_struct, inner_struct, second_inner_struct);

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
    EXPECT_EQ(pi, the_data["long_double"].value<long double>());

    srand48(time(0));

    for (int i = 0; i < CHECKS_NUMBER; ++i)
    {
        size_t idx_4 = lrand48()%int(STRUCTS_SIZE);
        EXPECT_EQ(INNER_STRING_VALUE, the_data["sequence"][idx_4]["inner_string"].value<std::string>());
        EXPECT_EQ(FLOAT, the_data["sequence"][idx_4]["inner_float"].value<float>());
        size_t idx_3 = lrand48()%int(STRUCTS_SIZE);
        EXPECT_EQ(INNER_SEQUENCE_STRING, the_data["sequence"][idx_4]["inner_sequence_string"][idx_3].value<std::string>());
        size_t idx_2 = lrand48()%int(STRUCTS_SIZE);
        EXPECT_EQ(SECOND_INNER_STRING, the_data["sequence"][idx_4]["inner_sequence_struct"][idx_2]["second_inner_string"].value<string>());
        EXPECT_EQ(UINT32, the_data["sequence"][idx_4]["inner_sequence_struct"][idx_2]["second_inner_uint32_t"].value<uint32_t>());

        size_t arr_idx_3 = lrand48()%int(STRUCTS_SIZE);
        size_t arr_idx_2 = lrand48()%int(STRUCTS_SIZE);
        size_t arr_idx_1 = lrand48()%int(STRUCTS_SIZE);

        long double check_over = LDOUBLE;
        EXPECT_EQ(check_over, the_data["array"][arr_idx_3][arr_idx_2].value<long double>());
        EXPECT_EQ(UINT8, the_data["sequence"][idx_4]["inner_sequence_struct"][idx_2]["second_inner_array"][arr_idx_1].value<uint8_t>());
    }
}

TEST (DynamicData, curious_interactions)
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

TEST (DynamicType, testing_is_compatible_string_no_bound)
{
    StringType s;
    StringType r;

    EXPECT_EQ(TypeConsistency::EQUALS, r.is_compatible(s) );
    EXPECT_EQ(TypeConsistency::EQUALS, s.is_compatible(r) );
}

TEST (DynamicType, testing_is_compatible_string_same_bound)
{
    srand48(time(0));
    size_t b = lrand48()%1000;
    StringType s(b);
    StringType r(b);

    EXPECT_EQ(TypeConsistency::EQUALS, r.is_compatible(s) );
    EXPECT_EQ(TypeConsistency::EQUALS, s.is_compatible(r) );
}


TEST (DynamicType, testing_is_compatible_string_different_bound)
{
    StringType s(15);
    StringType r(30);

    EXPECT_NE(0, uint32_t(TypeConsistency::IGNORE_STRING_BOUNDS) & uint32_t(s.is_compatible(r)) );
    EXPECT_NE(0, uint32_t(TypeConsistency::IGNORE_STRING_BOUNDS) & uint32_t(r.is_compatible(s)) );
}


TEST (DynamicType, testing_is_compatible_structure_of_string)
{
    StringType st;
    StructType r("check");
    r.add_member(Member("string", st));
    StructType s("other_check");
    s.add_member(Member("string", st));

    EXPECT_EQ(TypeConsistency::EQUALS , r.is_compatible(s) );
    EXPECT_EQ(TypeConsistency::EQUALS , s.is_compatible(r) );
}

TEST (DynamicType, testing_is_compatible_structure_of_sequence_no_bound)
{
    SequenceType s(primitive_type<uint32_t>());
    StructType the_str("check");
    the_str.add_member(Member("int", s));
    StructType other_str("other_check");
    other_str.add_member(Member("int", s));
    EXPECT_EQ(TypeConsistency::EQUALS , the_str.is_compatible(other_str));
    EXPECT_EQ(TypeConsistency::EQUALS , other_str.is_compatible(the_str));
}

TEST (DynamicType, testing_is_compatible_structure_of_sequence_different_bound)
{
    SequenceType s(primitive_type<uint32_t>(),15);
    SequenceType r(primitive_type<uint32_t>(),19);
    StructType the_str("check");
    the_str.add_member(Member("int", s));
    StructType other_str("other_check");
    other_str.add_member(Member("int", r));
    EXPECT_EQ(TypeConsistency::IGNORE_SEQUENCE_BOUNDS, the_str.is_compatible(other_str));
    EXPECT_EQ(TypeConsistency::IGNORE_SEQUENCE_BOUNDS , other_str.is_compatible(the_str));
}

TEST (DynamicType, testing_is_compatible_structure_of_sequence_same_bound)
{
    SequenceType s(primitive_type<uint32_t>(),15);
    StructType the_str("check");
    the_str.add_member(Member("int", s));
    StructType other_str("other_check");
    other_str.add_member(Member("int", s));
    EXPECT_EQ(TypeConsistency::EQUALS , the_str.is_compatible(other_str));
    EXPECT_EQ(TypeConsistency::EQUALS , other_str.is_compatible(the_str));
}

TEST (DynamicType, testing_is_compatible_structure_of_primitive_type_int)
{
    StructType the_str("check");
    the_str.add_member(Member("int", primitive_type<uint32_t>()));
    StructType other_str("other_check");
    other_str.add_member(Member("int", primitive_type<uint32_t>()));
    EXPECT_EQ(TypeConsistency::EQUALS , the_str.is_compatible(other_str));
    EXPECT_EQ(TypeConsistency::EQUALS , other_str.is_compatible(the_str));
}

TEST (DynamicType, testing_is_compatible_structure_of_primitive_type_float)
{
    StructType the_str("check");
    the_str.add_member(Member("int", primitive_type<long double>()));
    StructType other_str("other_check");
    other_str.add_member(Member("int", primitive_type<long double>()));
    EXPECT_EQ(TypeConsistency::EQUALS , the_str.is_compatible(other_str));
    EXPECT_EQ(TypeConsistency::EQUALS , other_str.is_compatible(the_str));
}

TEST (DynamicType, testing_is_compatible_structure_of_primitive_type_char)
{
    StructType the_str("check");
    the_str.add_member(Member("int", primitive_type<wchar_t>()));
    StructType other_str("other_check");
    other_str.add_member(Member("int", primitive_type<wchar_t>()));
    StructType another_str("another_check");
    another_str.add_member(Member("int", primitive_type<char>()));

    EXPECT_EQ(TypeConsistency::EQUALS , the_str.is_compatible(other_str));
    EXPECT_EQ(TypeConsistency::IGNORE_TYPE_WIDTH, the_str.is_compatible(another_str));
    EXPECT_EQ(TypeConsistency::EQUALS , other_str.is_compatible(the_str));
    EXPECT_EQ(TypeConsistency::IGNORE_TYPE_WIDTH, other_str.is_compatible(another_str));
    EXPECT_EQ(TypeConsistency::IGNORE_TYPE_WIDTH, another_str.is_compatible(the_str));
    EXPECT_EQ(TypeConsistency::IGNORE_TYPE_WIDTH, another_str.is_compatible(other_str));
}

TEST (DynamicType, testing_is_compatible_structure_of_primitive_type_int32_t)
{
    StructType the_str("check");
    the_str.add_member(Member("int", primitive_type<uint32_t>()));
    StructType other_str("other_check");
    other_str.add_member(Member("int", primitive_type<uint32_t>()));
    StructType another_str("another_check");
    another_str.add_member(Member("int", primitive_type<int32_t>()));
    EXPECT_EQ(TypeConsistency::IGNORE_TYPE_SIGN, the_str.is_compatible(another_str));
    EXPECT_EQ(TypeConsistency::EQUALS , other_str.is_compatible(the_str));
    EXPECT_EQ(TypeConsistency::IGNORE_TYPE_SIGN, other_str.is_compatible(another_str));
    EXPECT_EQ(TypeConsistency::IGNORE_TYPE_SIGN, another_str.is_compatible(the_str));
    EXPECT_EQ(TypeConsistency::IGNORE_TYPE_SIGN, another_str.is_compatible(other_str));
}

TEST (DynamicType, testing_is_compatible_structure_of_primitive_type_mixed_int)
{
    StructType the_str("check");
    the_str.add_member(Member("int", primitive_type<uint16_t>()));
    StructType other_str("other_check");
    other_str.add_member(Member("int", primitive_type<uint32_t>()));
    StructType another_str("another_check");
    another_str.add_member(Member("int", primitive_type<int64_t>()));
    EXPECT_EQ(TypeConsistency::IGNORE_TYPE_WIDTH, the_str.is_compatible(other_str));
    EXPECT_EQ(TypeConsistency::IGNORE_TYPE_WIDTH |
              TypeConsistency::IGNORE_TYPE_SIGN, the_str.is_compatible(another_str));
    EXPECT_EQ(TypeConsistency::IGNORE_TYPE_WIDTH |
              TypeConsistency::IGNORE_TYPE_SIGN, other_str.is_compatible(another_str));
}

TEST (DynamicType, testing_is_compatible_structure_of_array_same_bound)
{
    StructType the_str("check");
    ArrayType the_array(primitive_type<uint32_t>(), 10);
    the_str.add_member(Member("arr", the_array));
    StructType other_str("other_check");
    other_str.add_member(Member("arr", the_array));
    EXPECT_EQ(TypeConsistency::EQUALS , the_str.is_compatible(other_str));
    EXPECT_EQ(TypeConsistency::EQUALS , other_str.is_compatible(the_str));
}

TEST (DynamicType, testing_is_compatible_structure_of_array_different_bound_and_type)
{
    StructType the_str("check");
    ArrayType the_array(primitive_type<uint32_t>(), 10);
    the_str.add_member(Member("arr", the_array));

    ArrayType other_array(primitive_type<uint32_t>(), 11);
    StructType other_str("other_check");
    other_str.add_member(Member("arr", other_array));

    ArrayType another_array(primitive_type<int32_t>(), 10);
    StructType another_str("other_check");
    another_str.add_member(Member("arr", another_array));

    EXPECT_EQ(TypeConsistency::IGNORE_ARRAY_BOUNDS , the_str.is_compatible(other_str));
    EXPECT_EQ(TypeConsistency::IGNORE_TYPE_SIGN, the_str.is_compatible(another_str));
    EXPECT_EQ(TypeConsistency::IGNORE_ARRAY_BOUNDS, other_str.is_compatible(the_str));
    EXPECT_EQ(TypeConsistency::IGNORE_ARRAY_BOUNDS | TypeConsistency::IGNORE_TYPE_SIGN, other_str.is_compatible(another_str));
    EXPECT_EQ(TypeConsistency::IGNORE_TYPE_SIGN, another_str.is_compatible(the_str));
    EXPECT_EQ(TypeConsistency::IGNORE_ARRAY_BOUNDS | TypeConsistency::IGNORE_TYPE_SIGN, another_str.is_compatible(other_str));
}

template<typename A, typename B>
void singleCheck(void)
{
    DynamicData dd1(primitive_type<A>());
    DynamicData dd2(primitive_type<A>());
    dd1.value(A(15));
    dd2.value(A(15));
    EXPECT_EQ(dd1, dd2);
    dd2.value(A(16.3));
    EXPECT_NE(dd1, dd2);
}

TEST (DynamicData, testing_equality_check_primitive_type_uint8)
{
    singleCheck<uint8_t, uint8_t>();
}
TEST (DynamicData, testing_equality_check_primitive_type_uint16)
{
    singleCheck<uint16_t, uint16_t>();
}
TEST (DynamicData, testing_equality_check_primitive_type_int16)
{
    singleCheck<int16_t, int16_t>();
}
TEST (DynamicData, testing_equality_check_primitive_typeu_int32)
{
    singleCheck<uint32_t, uint32_t>();
}
TEST (DynamicData, testing_equality_check_primitive_type_int32)
{
    singleCheck<int32_t, int32_t>();
}
TEST (DynamicData, testing_equality_check_primitive_type_uint64)
{
    singleCheck<uint64_t, uint64_t>();
}
TEST (DynamicData, testing_equality_check_primitive_type_int64)
{
    singleCheck<int64_t, int64_t>();
}
TEST (DynamicData, testing_equality_check_primitive_type_float)
{
    singleCheck<float, float>();
}
TEST (DynamicData, testing_equality_check_primitive_type_double)
{
    singleCheck<double, double>();
}
TEST (DynamicData, testing_equality_check_primitive_type_char)
{
    singleCheck<char, char>();
}
TEST (DynamicData, testing_equality_check_primitive_type_wchar)
{
    singleCheck<wchar_t, wchar_t>();
}
TEST (DynamicData, testing_equality_check_primitive_type_double_longdouble)
{
    singleCheck<double, long double>();
}
TEST (DynamicData, testing_equality_check_primitive_type_int16_uint32)
{
    singleCheck<int16_t, uint32_t>();
}
TEST (DynamicData, testing_equality_check_primitive_type_double_uint8)
{
    singleCheck<double, uint8_t>();
}
TEST (DynamicData, testing_equality_check_primitive_type_int32_uint16)
{
    singleCheck<int32_t, uint16_t>();
}

TEST (DynamicData, testing_equality_check_string)
{
    SequenceType s(primitive_type<uint64_t>());
    DynamicData d1(s);
    DynamicData d2(s);
    d1.push<uint64_t>(3456);
    d2.push<uint64_t>(3456);
    EXPECT_EQ(true , d1 == d2);
    d2[0].value<uint64_t>(3457);
    EXPECT_NE(d1, d2);
    d2[0].value<uint64_t>(3456);
    d2.push<uint64_t>(435);
    EXPECT_NE(d1, d2);
}

TEST (DynamicData, test_equality_check_struct)
{
    StructType stru("the_struct");
    stru.add_member(Member("lld", primitive_type<long double>()));

    DynamicData d1(stru);
    d1["lld"].value<long double>(3.1415926);
    DynamicData d2(stru);
    d2["lld"].value<long double>(3.1415926);
    EXPECT_EQ(true, d1 == d2);
    d2["lld"].value<long double>(3.1415925);
    EXPECT_NE(d1, d2);

    stru.add_member(Member("c", primitive_type<char>()));
    DynamicData d3(stru);
    d3["lld"].value<long double>(3.1415926);
    EXPECT_NE(d3, d1);
    d3["c"].value<char>(3.1415926); //just an :"out of the ordinady
    EXPECT_NE(d3, d1);

}

namespace{
    template<typename T>
    ostream& operator << (ostream& o, const vector<T>& v)
    {
        for(auto it = v.begin(); it != v.end(); ++it)
        {
            o << '(' << *it << ')';
        }
        return o;
    }
}

TEST (DynamicData, test_equality_complex_struct)
{
    ArrayType ar(primitive_type<uint32_t>(), 15);
    SequenceType se(primitive_type<uint32_t>(), 15);
    StringType str;
    SequenceType seq(str, 15);
    StructType st("the_type");
    st.add_member(Member("array", ar));
    st.add_member(Member("sequence", se));
    st.add_member(Member("seqstring", seq));
    st.add_member(Member("string", str));
    DynamicData d1(st);
    d1["string"] = "sono io che sono qui";

    for(int i=0; i < 15; ++i)
    {
        d1["array"][i].value<uint32_t>(42);
        d1["sequence"].push<uint32_t>(42);
        d1["seqstring"].push<string>("ci sono anche io");
    }
    vector<char> v = d1["string"].as_vector<char>();
    vector<string> vv = d1["seqstring"].as_vector<string>();
    vector<uint32_t> t = d1["array"].as_vector<uint32_t>();
    vector<uint32_t> s = d1["sequence"].as_vector<uint32_t>();


}

TEST (DynamicType , wstring_and_wstring_struct)
{
    WStringType wst;
    StringType st;
    DynamicData d(wst);
    DynamicData dd(st);

    d = L"sadfsfdasdf";
    dd = "sadfsfdasdf";

    EXPECT_EQ( TypeConsistency::NONE, wst.is_compatible(st) );
    EXPECT_EQ( TypeConsistency::NONE, st.is_compatible(wst) );

    StructType struc1("the_struct");
    struc1.add_member(Member("theString", wst ));


    StructType struc2("the_struct");
    struc2.add_member(Member("theString", st ));

    EXPECT_EQ( TypeConsistency::NONE, struc1.is_compatible(struc2) );
    EXPECT_EQ( TypeConsistency::NONE, struc2.is_compatible(struc1) );
}

TEST (QoS, sequence)
{
    SequenceType s1(primitive_type<uint16_t>(), 10);
    SequenceType s2(primitive_type<uint16_t>(), 20);

    DynamicData d(s1);

    for(int i = 0; i < 10; ++i)
    {
        d.push(uint16_t());
    }
    d[9].value<uint16_t>(256);

    DynamicData dd(d, s2);

    EXPECT_EQ(10, dd.size());
    EXPECT_NE(true, dd != d);
    for (size_t i = 0; i < d.size(); ++i)
    {
        EXPECT_EQ( d[i].value<uint16_t>() , dd[i].value<uint16_t>() );
    }
}

TEST (QoS, other_sequence)
{
    SequenceType s1(primitive_type<uint16_t>(), 20);
    SequenceType s2(primitive_type<uint16_t>(), 10);

    DynamicData d(s1);

    for(size_t i = 0; i < 20; ++i)
    {
        d.push(uint16_t());
    }
    d[9].value<uint16_t>(256);

    DynamicData dd(d, s2);

    EXPECT_EQ(10, dd.size());
    EXPECT_NE(dd , d);
    for (size_t i = 0; i < dd.size(); ++i)
    {
        EXPECT_EQ( d[i].value<uint16_t>() , dd[i].value<uint16_t>() );
    }
}

TEST (QoS, string)
{
    StringType s(20);
    StringType t(10);

    EXPECT_EQ(s.is_compatible(t) , t.is_compatible(s));
    EXPECT_EQ(TypeConsistency::IGNORE_STRING_BOUNDS, t.is_compatible(s));

    DynamicData ds(s);
    ds = "12345678901234567890";

    DynamicData dt(t);
    dt = "1234567890";

    DynamicData dt_as_ds(dt, s);
    EXPECT_EQ(10, dt_as_ds.size());

    DynamicData ds_as_dt(ds, t);
    EXPECT_EQ(10, ds_as_dt.size());

    EXPECT_TRUE(dt == ds_as_dt);
}

TEST (QoS, wstring)
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
}

TEST (QoS, array)
{
    ArrayType a(primitive_type<int>(), 10);
    ArrayType b(primitive_type<int>(), 20);

    DynamicData d(a);
    d[8].value(10);
    DynamicData e(d,b);
    EXPECT_EQ(TypeConsistency::IGNORE_ARRAY_BOUNDS, b.is_compatible(a));
    EXPECT_NE(e , d);
    for (size_t i = 0; i < d.size(); ++i)
    {
        EXPECT_EQ( d[i].value<int>() , e[i].value<int>() );
    }
}

TEST (QoS, other_array)
{
    ArrayType a(primitive_type<int>(), 20);
    ArrayType b(primitive_type<int>(), 10);

    DynamicData d(a);
    d[8].value(10);
    d[13].value(10);
    DynamicData e(d,b);
    EXPECT_EQ(TypeConsistency::IGNORE_ARRAY_BOUNDS, b.is_compatible(a));
    EXPECT_NE(true, e != d);
    for (size_t i = 0; i < e.size(); ++i)
    {
        EXPECT_EQ( d[i].value<int>() , e[i].value<int>() );
    }
}

TEST (QoS, Array_qos)
{

    ArrayType a_arr(primitive_type<uint8_t>(), 10);
    ArrayType b_arr(primitive_type<int32_t>(), 11);

    EXPECT_EQ(TypeConsistency::IGNORE_ARRAY_BOUNDS |
              TypeConsistency::IGNORE_TYPE_WIDTH |
              TypeConsistency::IGNORE_TYPE_SIGN , a_arr.is_compatible(b_arr));

}

TEST (QoS, mixed_types)
{
    StructType a("composition");
    StructType b("composition");
    StringType a_string(10);
    WStringType b_wstring(10);
    SequenceType a_seq(primitive_type<uint32_t>(), 10);
    SequenceType b_seq(primitive_type<int32_t>(), 11);
    ArrayType a_arr(primitive_type<uint16_t>(), 10);
    ArrayType b_arr(primitive_type<int32_t>(), 11);



    //EXPECT_EQ(TypeConsistency::IGNORE_TYPE_WIDTH, a_string.is_compatible(b_wstring)); //This feature is not supported

    EXPECT_EQ(TypeConsistency::IGNORE_SEQUENCE_BOUNDS |
              TypeConsistency::IGNORE_TYPE_SIGN, a_seq.is_compatible(b_seq));

    EXPECT_EQ(TypeConsistency::IGNORE_ARRAY_BOUNDS |
              TypeConsistency::IGNORE_TYPE_WIDTH |
              TypeConsistency::IGNORE_TYPE_SIGN , a_arr.is_compatible(b_arr));

    a.add_member(
            //Member("a_string", a_string)).add_member( //Not supported
            Member("a_seq", a_seq)).add_member(
            Member("a_arr", a_arr)).add_member(
            Member("a_primitive", primitive_type<wchar_t>()));

    b.add_member(
            //Member("b_wstring", b_wstring)).add_member( //Not supported
            Member("b_seq", b_seq)).add_member(
            Member("b_arr", b_arr));

    EXPECT_EQ(TypeConsistency::IGNORE_MEMBER_NAMES|
              TypeConsistency::IGNORE_TYPE_SIGN |
              TypeConsistency::IGNORE_TYPE_WIDTH |
              TypeConsistency::IGNORE_SEQUENCE_BOUNDS |
              TypeConsistency::IGNORE_ARRAY_BOUNDS |
              TypeConsistency::IGNORE_MEMBERS , a.is_compatible(b));
}

TEST (QoS, ignore_member)
{
    StructType a("composition");
    StructType b("composition");

    StringType string(10);
    SequenceType seq(primitive_type<int>(), 10);
    ArrayType arr(primitive_type<float>(), 10.0);

    a.add_member(
            Member("string", string)).add_member(
            Member("seq", seq)).add_member(
            Member("arr", arr)).add_member(
            Member("flying_element", primitive_type<int>()));

    b.add_member(
            Member("string", string)).add_member(
            Member("seq", seq)).add_member(
            Member("arr", arr));

    EXPECT_EQ(TypeConsistency::IGNORE_MEMBERS , a.is_compatible(b));
}

TEST (QoS, ignore_member_simple_primitive)
{
    StructType a("composition");
    StructType b("composition");
    a.add_member(
            Member("x", primitive_type<char>())).add_member(
            Member("y", primitive_type<char>()));

    b.add_member(
            Member("x", primitive_type<char>()));

    EXPECT_EQ(TypeConsistency::IGNORE_MEMBERS , a.is_compatible(b));

}

TEST (Iterators, iterators_tests)
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

TEST (Utilities, for_each_types)
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

// This test checks the correct behavior of Modules, by checking visibility of the different structs from
// each possible scope. The expected results are the same that would apply to the C++ namespaces.
TEST (Utilities, Modules)
{
    StructType inner = StructType("InnerType")
        .add_member("inner_int32", primitive_type<uint32_t>())
        .add_member("inner_float", primitive_type<float>());

    StructType outer = StructType("OuterType")
        .add_member("outer_float", primitive_type<float>());

    StructType b = StructType("BType")
        .add_member("b_float", primitive_type<float>());

    Module root;
    Module& submod_A = root.create_submodule("A");
    Module& submod_B = root.create_submodule("B");
    Module& submod_AA = submod_A.create_submodule("A");
    root.structure(inner);
    submod_AA.structure(outer);
    submod_B.structure(b);

    // (root) - A - A - OuterType
    //        \ B - BType
    //        \ InnerType

    // Check scopes from ROOT
    ASSERT_TRUE(root.has_structure("A::A::OuterType"));
    ASSERT_TRUE(root.has_structure("::A::A::OuterType"));
    ASSERT_FALSE(root.has_structure("::A::OuterType"));
    ASSERT_FALSE(root.has_structure("A::OuterType"));
    ASSERT_TRUE(root.has_structure("::InnerType"));
    ASSERT_TRUE(root.has_structure("InnerType"));
    ASSERT_FALSE(root.has_structure("OuterType"));
    ASSERT_FALSE(root.has_structure("BType"));
    ASSERT_TRUE(root.has_structure("B::BType"));
    ASSERT_TRUE(root.has_structure("::B::BType"));
    ASSERT_FALSE(root.has_structure("A::B::BType"));
    ASSERT_FALSE(root.has_structure("::A::B::BType"));
    ASSERT_FALSE(root.has_structure("::B::B::BType"));

    // Check scopes from A
    ASSERT_TRUE(submod_A.has_structure("::A::A::OuterType"));
    ASSERT_FALSE(submod_A.has_structure("A::A::OuterType"));
    ASSERT_TRUE(submod_A.has_structure("A::OuterType"));
    ASSERT_FALSE(submod_A.has_structure("OuterType"));
    ASSERT_TRUE(submod_A.has_structure("::InnerType"));
    ASSERT_FALSE(submod_A.has_structure("InnerType"));
    ASSERT_FALSE(submod_A.has_structure("BType"));
    ASSERT_TRUE(submod_A.has_structure("B::BType"));
    ASSERT_FALSE(submod_A.has_structure("A::BType"));
    ASSERT_TRUE(submod_A.has_structure("::B::BType"));

    // Check scopes from A::A
    ASSERT_TRUE(submod_AA.has_structure("::A::A::OuterType"));
    ASSERT_FALSE(submod_AA.has_structure("A::A::OuterType"));
    ASSERT_TRUE(submod_AA.has_structure("A::OuterType"));
    ASSERT_TRUE(submod_AA.has_structure("OuterType"));
    ASSERT_TRUE(submod_AA.has_structure("::InnerType"));
    ASSERT_FALSE(submod_AA.has_structure("InnerType"));
    ASSERT_FALSE(submod_AA.has_structure("BType"));
    ASSERT_TRUE(submod_AA.has_structure("B::BType"));
    ASSERT_FALSE(submod_AA.has_structure("A::BType"));
    ASSERT_TRUE(submod_AA.has_structure("::B::BType"));

    // Check scopes from B
    ASSERT_TRUE(submod_B.has_structure("::A::A::OuterType"));
    ASSERT_TRUE(submod_B.has_structure("A::A::OuterType"));
    ASSERT_FALSE(submod_B.has_structure("A::OuterType"));
    ASSERT_FALSE(submod_B.has_structure("OuterType"));
    ASSERT_TRUE(submod_B.has_structure("::InnerType"));
    ASSERT_FALSE(submod_B.has_structure("InnerType"));
    ASSERT_TRUE(submod_B.has_structure("BType"));
    ASSERT_TRUE(submod_B.has_structure("B::BType"));
    ASSERT_FALSE(submod_B.has_structure("A::BType"));
    ASSERT_TRUE(submod_B.has_structure("::B::BType"));

    // Check accesibility and DynamicData creation.
    const StructType& my_struct = root.structure("A::A::OuterType");
    DynamicData my_data(my_struct);
    my_data["outer_float"] = 5.678f;
    ASSERT_EQ(my_data["outer_float"].value<float>(), 5.678f);

    const StructType& same_struct = root["A"]["A"].structure("OuterType"); // ::A::A::OuterType
    ASSERT_EQ(&my_struct, &same_struct); // Both access ways must be equivalent.
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

        ASSERT_DEATH({my_enum.add_enumerator("D", 11);}, "next_value"); // Asserts because 11 == last added value
        ASSERT_DEATH({my_enum.add_enumerator("E", 2);}, "next_value"); // Asserts because 2 < last added value
        ASSERT_DEATH({my_enum.add_enumerator("A");}, "has_enumerator"); // Asserts because A already exists

        DynamicData enum_data(my_enum);
        enum_data = my_enum.value("C");

        ASSERT_DEATH({uint64_t die = enum_data; (void) die;}, "type_index"); // This will assert

        // EnumerationType<uint64_t> my_long_enum("MyLongEnum"); // Static assert, uint64_t isn't allowed

        ASSERT_DEATH({enum_data = static_cast<uint32_t>(2);}, "is_allowed_value"); // Asserts because 2 isn't a valid value (0, 10 and 11).
    }
}

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
