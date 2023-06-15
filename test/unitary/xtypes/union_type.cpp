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
#include <xtypes/StringConversion.hpp>
#include <iostream>

#include <cmath>
#include <bitset>

#include "../utils.hpp"

using namespace std;
using namespace eprosima::xtypes;

/**********************************
*        UnionType Tests         *
**********************************/

TEST (UnionType, checks_and_access)
{
    UnionType un("bool_union_name", primitive_type<bool>());
    EXPECT_EQ("bool_union_name", un.name());
    EXPECT_EQ(TypeKind::UNION_TYPE, un.kind());

    size_t mem_size = 0;
    mem_size += sizeof(bool); // the discriminator is bool

    ASSERT_OR_EXCEPTION({un.add_case_member<bool>({true}, Member("discriminator", primitive_type<uint32_t>()));},
            "is reserved");

    ASSERT_OR_EXCEPTION({un.add_case_member<int64_t>({default_union_label(sizeof(bool))}, Member("default_label",
                         primitive_type<uint32_t>()));},
            "is reserved");

    ASSERT_OR_EXCEPTION({un.add_case_member<size_t>({}, Member("no_labels", primitive_type<uint32_t>()));},
            "without labels");

    un.add_case_member<bool>({true}, Member("bool", primitive_type<bool>()));
    mem_size += sizeof(bool);
    EXPECT_EQ(mem_size, un.memory_size());

    un.add_case_member<bool>({false}, Member("uint8_t", primitive_type<uint8_t>()));
    // sizeof(bool) == sizeof(uint8_t), so memory_size doesn't change.
    EXPECT_EQ(mem_size, un.memory_size());

    ASSERT_OR_EXCEPTION({un.add_case_member<bool>({true}, Member("label_taken", primitive_type<uint32_t>()));},
            "already in use");

    DynamicData union_data(un);
    ASSERT_OR_EXCEPTION({bool data = union_data.get_member("bool"); (void)data;}, "member selected");
    union_data["bool"] = true;
    ASSERT_OR_EXCEPTION({bool data = union_data.get_member("uint8_t"); (void)data;}, "non-selected case member");
    {
        union_data.get_member("bool"); // Once selected, it doesn't fails.
    }
    EXPECT_EQ(union_data.d().value<bool>(), true); // The discriminator must be updated
    EXPECT_EQ(union_data["bool"].value<bool>(), true);
    union_data["bool"] = false;
    EXPECT_EQ(union_data["bool"].value<bool>(), false);

    union_data["uint8_t"] = uint8_t(55);
    EXPECT_EQ(union_data.d().value<bool>(), false); // The discriminator must be updated
    EXPECT_EQ(union_data["uint8_t"].value<uint8_t>(), 55);

    // Discriminator
    ASSERT_OR_EXCEPTION({union_data["discriminator"] = true;}, "Union discriminator");
    ASSERT_OR_EXCEPTION({union_data.d(static_cast<size_t>(true));}, "Cannot switch member");
    union_data.d(static_cast<size_t>(false));
    union_data["bool"] = true;
    EXPECT_EQ(union_data.d().value<bool>(), true); // The discriminator must be updated

    DynamicData disc(primitive_type<bool>());
    disc = true;
    union_data.d(disc);
    disc = false;
    ASSERT_OR_EXCEPTION({union_data.d(disc);}, "Cannot switch member");

    // -1 must be allowed
    un.add_case_member<int64_t>({-1}, Member("_1", primitive_type<int32_t>()));

    // This line doesn't compiles
    // un.add_case_member<float>({55.5}, Member("dont_compiles", primitive_type<bool>()));
}

TEST (UnionType, discriminator_types)
{
    UnionType un_bool("bool_union", primitive_type<bool>());
    UnionType un_int8("int8_union", primitive_type<int8_t>());
    UnionType un_uint8("uint8_union", primitive_type<uint8_t>());
    UnionType un_int16("int16_union", primitive_type<int16_t>());
    UnionType un_uint16("uint16_union", primitive_type<uint16_t>());
    UnionType un_int32("int32_union", primitive_type<int32_t>());
    UnionType un_uint32("uint32_union", primitive_type<uint32_t>());
    UnionType un_int64("int64_union", primitive_type<int64_t>());
    UnionType un_uint64("uint64_union", primitive_type<uint64_t>());
    UnionType un_char8("char8_union", primitive_type<char>());
    UnionType un_char16("char16_union", primitive_type<wchar_t>());

    EnumerationType<uint32_t> my_enum("MyEnum");
    UnionType un_enum("enum_union", my_enum);

    AliasType my_alias(my_enum, "MyEnumAlias");
    UnionType un_alias("alias_union", my_alias);

    AliasType my_alias2(my_alias, "MyAliasAlias");
    UnionType un_alias2("alias_alias_union", my_alias2);

    ASSERT_OR_EXCEPTION({UnionType un_invalid("invalid", primitive_type<float>());}, "isn't allowed");
    ASSERT_OR_EXCEPTION({UnionType un_invalid("invalid", primitive_type<double>());}, "isn't allowed");
    ASSERT_OR_EXCEPTION({UnionType un_invalid("invalid", primitive_type<long double>());}, "isn't allowed");
    ASSERT_OR_EXCEPTION({UnionType un_invalid("invalid", un_bool);}, "isn't allowed");
    StringType str;
    ASSERT_OR_EXCEPTION({UnionType un_invalid("invalid", str);}, "isn't allowed");
    StructType struct_t("MyStruct");
    ASSERT_OR_EXCEPTION({UnionType un_invalid("invalid", struct_t);}, "isn't allowed");
    ArrayType array(primitive_type<bool>(), 5);
    ASSERT_OR_EXCEPTION({UnionType un_invalid("invalid", array);}, "isn't allowed");
    SequenceType seq(primitive_type<bool>());
    ASSERT_OR_EXCEPTION({UnionType un_invalid("invalid", seq);}, "isn't allowed");
}

TEST (UnionType, complex_union_and_default)
{
    AliasType first_alias(primitive_type<char>(), "Alias1");
    AliasType disc_type(first_alias, "DiscType");
    UnionType complex("complex", disc_type);

    size_t disc_size = sizeof(char); // the discriminator is char
    EXPECT_EQ(disc_size, complex.memory_size());

    std::vector<char> bool_labels{'a', 'c', 'd'};
    complex.add_case_member(bool_labels, Member("bool", primitive_type<bool>()));
    size_t mem_size = disc_size + sizeof(bool);
    EXPECT_EQ(mem_size, complex.memory_size());

    StructType st("MyStruct");
    st.add_member(Member("uint64", primitive_type<uint64_t>()));
    st.add_member(Member("int64", primitive_type<int64_t>()));
    st.add_member(Member("float", primitive_type<float>()));

    std::vector<char> st_labels{'b', 'e'};
    complex.add_case_member(st_labels, Member("st", st));
    mem_size = disc_size + st.memory_size(); // st is bigger than bool
    EXPECT_EQ(mem_size, complex.memory_size());

    ArrayType array(primitive_type<uint64_t>(), 50);
    std::vector<char> array_labels{'f'};
    complex.add_case_member(array_labels, Member("array", array), true); // DEFAULT!

    // DEFAULT CHECK
    ASSERT_OR_EXCEPTION(
        {complex.add_case_member<char>({'z'}, Member("invalid", array), true);},
        "default");

    mem_size = disc_size + array.memory_size(); // array is bigger than st
    EXPECT_EQ(mem_size, complex.memory_size());

    StringType string;
    std::vector<char> string_labels{'g'};
    complex.add_case_member(string_labels, Member("string", string));
    // This string isn't bigger than the array, so the size shouldn't change.
    EXPECT_EQ(mem_size, complex.memory_size());

    DynamicData data(complex);

    // By default, 'f' is selected. The array.
    EXPECT_EQ('f', data.d().value<char>());
    for (uint64_t i = 0; i < data.get_member("array").size(); ++i)
    {
        data["array"][i] = (i + 1) * 5000;
        EXPECT_EQ((i + 1) * 5000, data.get_member("array")[i].value<uint64_t>());
    }

    // Change to bool
    data["bool"] = true;
    EXPECT_EQ('a', data.d().value<char>());
    EXPECT_TRUE(data["bool"].value<bool>());

    // Back to array
    data["array"]; // Enough to select it.
    EXPECT_EQ('f', data.d().value<char>());
    EXPECT_NE(5000, data.get_member("array")[0].value<uint64_t>()); // Modified when accessing as bool.

    // Change to st
    data["st"]["uint64"] = UINT64_C(5000);
    data["st"]["int64"] = INT64_C(-756757623);
    data["st"]["float"] = 445.322f;
    EXPECT_EQ('b', data.d().value<char>());
    data.d('e');
    EXPECT_EQ('e', data.d().value<char>());
    EXPECT_EQ(data.get_member("st")["uint64"].value<uint64_t>(), 5000ul);
    EXPECT_EQ(data.get_member("st")["int64"].value<int64_t>(), -756757623l);
    EXPECT_EQ(data.get_member("st")["float"].value<float>(), 445.322f);

    // Change to string
    data["string"] = "This is an string";
    EXPECT_EQ('g', data.d().value<char>());
    std::string str = data.get_member("string");
    EXPECT_EQ(str, "This is an string");

    // Change to st again and back to string
    data["st"];
    str = data["string"].value<std::string>();
    EXPECT_NE(str, "This is an string"); // Data lost
}

TEST (UnionType, default_behavior)
{
    EnumerationType<uint32_t> my_enum("MyEnum");
    my_enum.add_enumerator("A", 5);
    my_enum.add_enumerator("B", 6);
    my_enum.add_enumerator("C", 9);

    UnionType union_type("MyUnion", my_enum);
    ASSERT_OR_EXCEPTION(
        {union_type.add_case_member<uint32_t>({8}, Member("invalid", primitive_type<bool>()));},
        "isn't allowed by the discriminator");
    union_type.add_case_member<uint32_t>({6}, Member("valid", primitive_type<bool>()));
    union_type.add_case_member<uint32_t>({}, Member("default", my_enum), true);
    std::vector<uint32_t> labels{my_enum.value("A")};
    union_type.add_case_member(labels, Member("a", primitive_type<uint32_t>()));

    DynamicData data(union_type);
    EXPECT_EQ(data.d().value<uint32_t>(), static_cast<uint32_t>(default_union_label(sizeof(uint32_t))));
    data["default"] = my_enum.value("C");
    EXPECT_EQ(data.get_member("default").value<uint32_t>(), 9);
}

TEST (UnionType, labels_as_string)
{
    EnumerationType<uint32_t> my_enum("MyEnum");
    my_enum.add_enumerator("A", 55);
    my_enum.add_enumerator("B", 100);

    UnionType union_type("MyUnion", my_enum);
    std::vector<std::string> labels_1{"A"};
    std::vector<std::string> labels_2{"100", "default"};

    union_type.add_case_member(labels_1, Member("a", primitive_type<bool>()));
    union_type.add_case_member(labels_2, Member("b_default", primitive_type<bool>()));

    DynamicData data(union_type);
    EXPECT_EQ(data.d().value<uint32_t>(), static_cast<uint32_t>(100));
}

// Regression test for #8366
TEST (UnionType, changing_several_discriminators)
{
    UnionType union_type("union_for_disc", primitive_type<uint32_t>());
    union_type.add_case_member<uint32_t>({6}, Member("field_1", primitive_type<uint32_t>()));
    union_type.add_case_member<uint32_t>({1}, Member("field_2", primitive_type<uint8_t>()));

    DynamicData data1(union_type);
    data1["field_2"] = static_cast<uint8_t>(3);
    EXPECT_EQ(data1.get_member("field_2").value<uint8_t>(), static_cast<uint8_t>(3));

    DynamicData data2(union_type);
    data2["field_1"] = 1024u;
    EXPECT_EQ(data2.get_member("field_1").value<uint32_t>(), 1024u);

    EXPECT_EQ(data1.get_member("field_2").value<uint8_t>(), static_cast<uint8_t>(3));
}

// Regression test for #8361 - Negative signed values converted into int64_t positive values by shrink_label
TEST (UnionType, signed_int)
{
    StructType struct_type("MyStruct");
    UnionType union_type("MyUnion", primitive_type<int32_t>());
    StructType member_a_type("StructA");
    StructType member_b_type("StructB");

    member_a_type.add_member("aa", primitive_type<int32_t>());

    union_type.add_case_member({"3112858602"}, Member("a", member_a_type));

    struct_type.add_member("union", union_type);

    DynamicData st_data(struct_type);
    DynamicData member_a_data(member_a_type);
    member_a_data["aa"] = 55;

    st_data["union"]["a"] = member_a_data;

    EXPECT_EQ(st_data["union"]["a"]["aa"].value<int32_t>(), member_a_data["aa"].value<int32_t>());

    st_data["union"]["a"]["aa"] = 66;

    member_a_data = st_data["union"]["a"];

    EXPECT_EQ(st_data["union"]["a"]["aa"].value<int32_t>(), member_a_data["aa"].value<int32_t>());
    EXPECT_EQ(66, member_a_data["aa"].value<int32_t>());
}
