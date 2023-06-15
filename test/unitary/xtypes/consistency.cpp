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

using namespace eprosima::xtypes;

/***********************************
 *        Consistency Tests        *
 ***********************************/

TEST (Consistency, testing_is_compatible_string_no_bound)
{
    StringType s;
    StringType r;

    EXPECT_EQ(TypeConsistency::EQUALS, r.is_compatible(s) );
    EXPECT_EQ(TypeConsistency::EQUALS, s.is_compatible(r) );
}

TEST (Consistency, testing_is_compatible_short_string_same_bound)
{
    size_t b = 15;
    StringType s(b);
    StringType r(b);

    EXPECT_EQ(TypeConsistency::EQUALS, r.is_compatible(s) );
    EXPECT_EQ(TypeConsistency::EQUALS, s.is_compatible(r) );
}

TEST (Consistency, testing_is_compatible_long_string_same_bound)
{
    size_t b = 1000;
    StringType s(b);
    StringType r(b);

    EXPECT_EQ(TypeConsistency::EQUALS, r.is_compatible(s) );
    EXPECT_EQ(TypeConsistency::EQUALS, s.is_compatible(r) );
}

TEST (Consistency, testing_is_compatible_string_different_bound)
{
    StringType s(15);
    StringType r(30);

    EXPECT_NE(0, uint32_t(TypeConsistency::IGNORE_STRING_BOUNDS) & uint32_t(s.is_compatible(r)) );
    EXPECT_NE(0, uint32_t(TypeConsistency::IGNORE_STRING_BOUNDS) & uint32_t(r.is_compatible(s)) );
}

TEST (Consistency, testing_is_compatible_string_bound_vs_no_bound)
{
    StringType s;
    StringType r(30);

    EXPECT_NE(0, uint32_t(TypeConsistency::IGNORE_STRING_BOUNDS) & uint32_t(s.is_compatible(r)) );
    EXPECT_NE(0, uint32_t(TypeConsistency::IGNORE_STRING_BOUNDS) & uint32_t(r.is_compatible(s)) );
}

TEST (Consistency, testing_is_compatible_structure_of_string)
{
    StringType st;
    StructType r("check");
    r.add_member(Member("string", st));
    StructType s("other_check");
    s.add_member(Member("string", st));

    EXPECT_EQ(TypeConsistency::EQUALS , r.is_compatible(s) );
    EXPECT_EQ(TypeConsistency::EQUALS , s.is_compatible(r) );
}

TEST (Consistency, testing_is_compatible_structure_of_sequence_no_bound)
{
    SequenceType s(primitive_type<uint32_t>());
    StructType the_str("check");
    the_str.add_member(Member("int", s));
    StructType other_str("other_check");
    other_str.add_member(Member("int", s));
    EXPECT_EQ(TypeConsistency::EQUALS , the_str.is_compatible(other_str));
    EXPECT_EQ(TypeConsistency::EQUALS , other_str.is_compatible(the_str));
}

TEST (Consistency, testing_is_compatible_structure_of_sequence_bound_vs_no_bound)
{
    SequenceType s(primitive_type<uint32_t>());
    SequenceType r(primitive_type<uint32_t>(),19);
    StructType the_str("check");
    the_str.add_member(Member("int", s));
    StructType other_str("other_check");
    other_str.add_member(Member("int", r));
    EXPECT_EQ(TypeConsistency::IGNORE_SEQUENCE_BOUNDS, the_str.is_compatible(other_str));
    EXPECT_EQ(TypeConsistency::IGNORE_SEQUENCE_BOUNDS , other_str.is_compatible(the_str));
}

TEST (Consistency, testing_is_compatible_structure_of_sequence_different_bound)
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

TEST (Consistency, testing_is_compatible_structure_of_sequence_small_bound)
{
    SequenceType s(primitive_type<uint32_t>(),15);
    StructType the_str("check");
    the_str.add_member(Member("int", s));
    StructType other_str("other_check");
    other_str.add_member(Member("int", s));
    EXPECT_EQ(TypeConsistency::EQUALS , the_str.is_compatible(other_str));
    EXPECT_EQ(TypeConsistency::EQUALS , other_str.is_compatible(the_str));
}

TEST (Consistency, testing_is_compatible_structure_of_sequence_large_bound)
{
    SequenceType s(primitive_type<uint32_t>(),15000);
    StructType the_str("check");
    the_str.add_member(Member("int", s));
    StructType other_str("other_check");
    other_str.add_member(Member("int", s));
    EXPECT_EQ(TypeConsistency::EQUALS , the_str.is_compatible(other_str));
    EXPECT_EQ(TypeConsistency::EQUALS , other_str.is_compatible(the_str));
}

TEST (Consistency, testing_is_compatible_structure_of_primitive_type_char)
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

TEST (Consistency, testing_is_compatible_structure_of_primitive_type_int32_t)
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

TEST (Consistency, testing_is_compatible_structure_of_primitive_type_mixed_int)
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

TEST (Consistency, testing_is_compatible_structure_of_primitive_type_float)
{
    StructType the_str("check");
    the_str.add_member(Member("int", primitive_type<long double>()));
    StructType other_str("other_check");
    other_str.add_member(Member("int", primitive_type<long double>()));
    StructType another_str("another_check");
    another_str.add_member(Member("int", primitive_type<float>()));
    StructType another_more_str("another_more_check");
    another_more_str.add_member(Member("int", primitive_type<double>()));
    EXPECT_EQ(TypeConsistency::EQUALS , the_str.is_compatible(other_str));
    EXPECT_EQ(TypeConsistency::EQUALS , other_str.is_compatible(the_str));
    EXPECT_EQ(TypeConsistency::IGNORE_TYPE_WIDTH, the_str.is_compatible(another_str));
    EXPECT_EQ(TypeConsistency::IGNORE_TYPE_WIDTH, another_str.is_compatible(another_more_str));
#ifndef _MSC_VER
    // on visual studio std::is_same<long double, double>::value == true
    EXPECT_EQ(TypeConsistency::IGNORE_TYPE_WIDTH, the_str.is_compatible(another_more_str));
#endif
}

TEST (Consistency, testing_is_compatible_structure_of_array_small_bound)
{
    StructType the_str("check");
    ArrayType the_array(primitive_type<uint32_t>(), 10);
    the_str.add_member(Member("arr", the_array));
    StructType other_str("other_check");
    other_str.add_member(Member("arr", the_array));
    EXPECT_EQ(TypeConsistency::EQUALS , the_str.is_compatible(other_str));
    EXPECT_EQ(TypeConsistency::EQUALS , other_str.is_compatible(the_str));
}

TEST (Consistency, testing_is_compatible_structure_of_array_large_bound)
{
    StructType the_str("check");
    ArrayType the_array(primitive_type<uint32_t>(), 15000);
    the_str.add_member(Member("arr", the_array));
    StructType other_str("other_check");
    other_str.add_member(Member("arr", the_array));
    EXPECT_EQ(TypeConsistency::EQUALS , the_str.is_compatible(other_str));
    EXPECT_EQ(TypeConsistency::EQUALS , other_str.is_compatible(the_str));
}

TEST (Consistency, testing_is_compatible_structure_of_array_different_bound_and_type)
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

TEST (Consistency, testing_is_compatible_aliases)
{
    AliasType int32_alias(primitive_type<int32_t>(), "int32_t");
    AliasType int32_alias_2(primitive_type<int32_t>(), "int32_t");
    AliasType int32_alias_3(primitive_type<int32_t>(), "another_int32");
    AliasType int32_alias_alias(int32_alias, "int32_t_alias");
    AliasType int32_alias_alias_alias(int32_alias_alias, "int32_t_alias_alias");

    // One way
    EXPECT_EQ(int32_alias.is_compatible(primitive_type<int32_t>()), TypeConsistency::EQUALS);
    EXPECT_EQ(int32_alias.is_compatible(int32_alias_2), TypeConsistency::EQUALS);
    EXPECT_EQ(int32_alias.is_compatible(int32_alias_3), TypeConsistency::EQUALS);
    EXPECT_EQ(int32_alias.is_compatible(int32_alias_alias), TypeConsistency::EQUALS);
    EXPECT_EQ(int32_alias.is_compatible(int32_alias_alias_alias), TypeConsistency::EQUALS);
    EXPECT_EQ(int32_alias_alias.is_compatible(int32_alias), TypeConsistency::EQUALS);
    EXPECT_EQ(int32_alias_alias.is_compatible(int32_alias_2), TypeConsistency::EQUALS);
    EXPECT_EQ(int32_alias_alias.is_compatible(int32_alias_3), TypeConsistency::EQUALS);
    EXPECT_EQ(int32_alias_alias.is_compatible(int32_alias_alias_alias), TypeConsistency::EQUALS);
    EXPECT_EQ(int32_alias_alias_alias.is_compatible(int32_alias), TypeConsistency::EQUALS);
    EXPECT_EQ(int32_alias_alias_alias.is_compatible(int32_alias_2), TypeConsistency::EQUALS);
    EXPECT_EQ(int32_alias_alias_alias.is_compatible(int32_alias_3), TypeConsistency::EQUALS);
    EXPECT_EQ(int32_alias_alias_alias.is_compatible(int32_alias_alias), TypeConsistency::EQUALS);

    // The other way
    EXPECT_EQ(primitive_type<int32_t>().is_compatible(int32_alias), TypeConsistency::EQUALS);
    EXPECT_EQ(int32_alias_2.is_compatible(int32_alias), TypeConsistency::EQUALS);
    EXPECT_EQ(int32_alias_3.is_compatible(int32_alias), TypeConsistency::EQUALS);
    EXPECT_EQ(int32_alias_alias.is_compatible(int32_alias), TypeConsistency::EQUALS);
    EXPECT_EQ(int32_alias_alias_alias.is_compatible(int32_alias), TypeConsistency::EQUALS);
    EXPECT_EQ(int32_alias.is_compatible(int32_alias_alias), TypeConsistency::EQUALS);
    EXPECT_EQ(int32_alias_2.is_compatible(int32_alias_alias), TypeConsistency::EQUALS);
    EXPECT_EQ(int32_alias_3.is_compatible(int32_alias_alias), TypeConsistency::EQUALS);
    EXPECT_EQ(int32_alias_alias_alias.is_compatible(int32_alias_alias), TypeConsistency::EQUALS);
    EXPECT_EQ(int32_alias.is_compatible(int32_alias_alias_alias), TypeConsistency::EQUALS);
    EXPECT_EQ(int32_alias_2.is_compatible(int32_alias_alias_alias), TypeConsistency::EQUALS);
    EXPECT_EQ(int32_alias_3.is_compatible(int32_alias_alias_alias), TypeConsistency::EQUALS);
    EXPECT_EQ(int32_alias_alias.is_compatible(int32_alias_alias_alias), TypeConsistency::EQUALS);
}

TEST (Consistency , wstring_and_string_struct)
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

TEST (Consistency, mixed_types)
{
    StructType a("composition");
    StructType b("composition");
    StructType c("composition");
    StringType a_string(10);
    StringType b_string(11);
    SequenceType a_seq(primitive_type<uint32_t>(), 10);
    SequenceType b_seq(primitive_type<int32_t>(), 11);
    ArrayType a_arr(primitive_type<uint16_t>(), 10);
    ArrayType b_arr(primitive_type<int32_t>(), 11);

    EXPECT_EQ(TypeConsistency::IGNORE_SEQUENCE_BOUNDS |
              TypeConsistency::IGNORE_TYPE_SIGN, a_seq.is_compatible(b_seq));

    EXPECT_EQ(TypeConsistency::IGNORE_ARRAY_BOUNDS |
              TypeConsistency::IGNORE_TYPE_WIDTH |
              TypeConsistency::IGNORE_TYPE_SIGN , a_arr.is_compatible(b_arr));

    a.add_member("a_string", a_string);
    a.add_member("a_seq", a_seq);
    a.add_member("a_arr", a_arr);
    a.add_member("a_primitive", primitive_type<wchar_t>());

    b.add_member("b_string", b_string);
    b.add_member("b_seq", b_seq);
    b.add_member("b_arr", b_arr);

    c.add_member("a_string", b_string);
    c.add_member("a_seq", b_seq);
    c.add_member("a_arr", b_arr);
    c.add_member("a_primitive", primitive_type<wchar_t>());

    EXPECT_EQ(TypeConsistency::IGNORE_MEMBER_NAMES|
              TypeConsistency::IGNORE_TYPE_SIGN |
              TypeConsistency::IGNORE_TYPE_WIDTH |
              TypeConsistency::IGNORE_SEQUENCE_BOUNDS |
              TypeConsistency::IGNORE_ARRAY_BOUNDS |
              TypeConsistency::IGNORE_MEMBERS |
              TypeConsistency::IGNORE_STRING_BOUNDS, a.is_compatible(b));

    EXPECT_EQ(TypeConsistency::IGNORE_TYPE_SIGN |
              TypeConsistency::IGNORE_TYPE_WIDTH |
              TypeConsistency::IGNORE_SEQUENCE_BOUNDS |
              TypeConsistency::IGNORE_ARRAY_BOUNDS |
              TypeConsistency::IGNORE_STRING_BOUNDS, a.is_compatible(c));
}

TEST (Consistency, ignore_member)
{
    StructType a("composition");
    StructType b("composition");

    StringType string(10);
    SequenceType seq(primitive_type<int>(), 10);
    ArrayType arr(primitive_type<float>(), 10.0);

    a.add_member("string", string);
    a.add_member("seq", seq);
    a.add_member("arr", arr);
    a.add_member("flying_element", primitive_type<int>());

    b.add_member("string", string);
    b.add_member("seq", seq);
    b.add_member("arr", arr);

    EXPECT_EQ(TypeConsistency::IGNORE_MEMBERS , a.is_compatible(b));
}

TEST (Consistency, ignore_member_simple_primitive)
{
    StructType a("composition");
    StructType b("composition");
    a.add_member("x", primitive_type<char>());
    a.add_member("y", primitive_type<char>());

    b.add_member("x", primitive_type<char>());

    EXPECT_EQ(TypeConsistency::IGNORE_MEMBERS , a.is_compatible(b));

}
