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
#include <iostream>

using namespace eprosima::xtypes;
using namespace eprosima::xtypes::idl;

void check_result(
        const Module& root)
{
    ASSERT_TRUE(root.has_alias("ByteMultiArray"));
    ASSERT_TRUE(root.has_enum_32("RootEnum"));
    const EnumerationType<uint32_t>& root_enum = root.enum_32("RootEnum");
    ASSERT_EQ(root_enum.value("VALUE_1"), 0);
    ASSERT_EQ(root_enum.value("VALUE_2"), 1);
    ASSERT_EQ(root_enum.value("VALUE_3"), 2);
    ASSERT_EQ(root_enum.value("VALUE_4"), 3);

    ASSERT_TRUE(root.has_constant("ROOT_STRING"));
    std::string root_string = root.constant("ROOT_STRING");
    ASSERT_EQ(root_string, "RootString");

    ASSERT_TRUE(root.has_structure("RootStruct"));
    const StructType& root_struct = root.structure("RootStruct");
    ASSERT_EQ(root_struct.member("my_bool").type().kind(), TypeKind::BOOLEAN_TYPE);
    ASSERT_EQ(root_struct.member("my_int8").type().kind(), TypeKind::INT_8_TYPE);
    ASSERT_EQ(root_struct.member("my_uint8").type().kind(), TypeKind::UINT_8_TYPE);
    ASSERT_EQ(root_struct.member("my_int16").type().kind(), TypeKind::INT_16_TYPE);
    ASSERT_EQ(root_struct.member("my_uint16").type().kind(), TypeKind::UINT_16_TYPE);
    ASSERT_EQ(root_struct.member("my_int32").type().kind(), TypeKind::INT_32_TYPE);
    ASSERT_EQ(root_struct.member("my_uint32").type().kind(), TypeKind::UINT_32_TYPE);
    ASSERT_EQ(root_struct.member("my_int64").type().kind(), TypeKind::INT_64_TYPE);
    ASSERT_EQ(root_struct.member("my_uint64").type().kind(), TypeKind::UINT_64_TYPE);
    ASSERT_EQ(root_struct.member("my_float").type().kind(), TypeKind::FLOAT_32_TYPE);
    ASSERT_EQ(root_struct.member("my_double").type().kind(), TypeKind::FLOAT_64_TYPE);
    ASSERT_EQ(root_struct.member("my_long_double").type().kind(), TypeKind::FLOAT_128_TYPE);
    ASSERT_EQ(root_struct.member("my_char").type().kind(), TypeKind::CHAR_8_TYPE);
    ASSERT_EQ(root_struct.member("my_wchar").type().kind(), TypeKind::WIDE_CHAR_TYPE);
    ASSERT_EQ(root_struct.member("my_string").type().kind(), TypeKind::STRING_TYPE);
    ASSERT_EQ(root_struct.member("my_wstring").type().kind(), TypeKind::WSTRING_TYPE);
    // Complex members
    {
        const Member& member = root_struct.member("my_alias_array");
        ASSERT_EQ(member.type().kind(), TypeKind::ALIAS_TYPE);
        const AliasType& alias = static_cast<const AliasType&>(member.type());
        ASSERT_EQ(alias.get().kind(), TypeKind::ARRAY_TYPE);
        const ArrayType& aliased_array = static_cast<const ArrayType&>(*alias);
        ASSERT_EQ(aliased_array.dimension(), 6);
        ASSERT_EQ(aliased_array.content_type().kind(), TypeKind::ARRAY_TYPE);
        const ArrayType& aliased_array_array = static_cast<const ArrayType&>(aliased_array.content_type());
        ASSERT_EQ(aliased_array_array.dimension(), 7);
        ASSERT_EQ(aliased_array_array.content_type().kind(), TypeKind::ARRAY_TYPE);
        const ArrayType& aliased_array_array_array = static_cast<const ArrayType&>(aliased_array_array.content_type());
        ASSERT_EQ(aliased_array_array_array.dimension(), 8);
        ASSERT_EQ(aliased_array_array_array.content_type().kind(), TypeKind::UINT_8_TYPE);
    }
    {
        const Member& member = root_struct.member("my_unbound_uint32_seq");
        ASSERT_EQ(member.type().kind(), TypeKind::SEQUENCE_TYPE);
        const SequenceType& inner_type = static_cast<const SequenceType&>(member.type());
        ASSERT_EQ(inner_type.content_type().kind(), TypeKind::UINT_32_TYPE);
        ASSERT_EQ(inner_type.bounds(), 0);
    }
    {
        const Member& member = root_struct.member("my_bound10_double_seq");
        ASSERT_EQ(member.type().kind(), TypeKind::SEQUENCE_TYPE);
        const SequenceType& inner_type = static_cast<const SequenceType&>(member.type());
        ASSERT_EQ(inner_type.content_type().kind(), TypeKind::FLOAT_64_TYPE);
        ASSERT_EQ(inner_type.bounds(), 10);
    }
    {
        const Member& member = root_struct.member("my_string_array");
        ASSERT_EQ(member.type().kind(), TypeKind::ARRAY_TYPE);
        const ArrayType& inner_type = static_cast<const ArrayType&>(member.type());
        ASSERT_EQ(inner_type.content_type().kind(), TypeKind::STRING_TYPE);
        ASSERT_EQ(inner_type.dimension(), 3); // VALUE_4 == 3
    }
    {
        const Member& member = root_struct.member("my_nested_bool_seq5_3");
        ASSERT_EQ(member.type().kind(), TypeKind::SEQUENCE_TYPE);
        const SequenceType& inner_type = static_cast<const SequenceType&>(member.type());
        ASSERT_EQ(inner_type.content_type().kind(), TypeKind::SEQUENCE_TYPE);
        ASSERT_EQ(inner_type.bounds(), 3);
        const SequenceType& inner_inner_type = static_cast<const SequenceType&>(inner_type.content_type());
        ASSERT_EQ(inner_inner_type.content_type().kind(), TypeKind::BOOLEAN_TYPE);
        ASSERT_EQ(inner_inner_type.bounds(), 5);
    }
    {
        const Member& member = root_struct.member("my_int64_nested_array");
        ASSERT_EQ(member.type().kind(), TypeKind::ARRAY_TYPE);
        const ArrayType& inner_type = static_cast<const ArrayType&>(member.type());
        ASSERT_EQ(inner_type.content_type().kind(), TypeKind::ARRAY_TYPE);
        ASSERT_EQ(inner_type.dimension(), 5);
        const ArrayType& inner_inner_type = static_cast<const ArrayType&>(inner_type.content_type());
        ASSERT_EQ(inner_inner_type.content_type().kind(), TypeKind::INT_64_TYPE);
        ASSERT_EQ(inner_inner_type.dimension(), 4);
    }
    {
        // map<RootEnum, sequence<int16>> my_map;
        const Member& member = root_struct.member("my_map");
        ASSERT_EQ(member.type().kind(), TypeKind::MAP_TYPE);
        const MapType& inner_type = static_cast<const MapType&>(member.type());
        const PairType& content_type = static_cast<const PairType&>(inner_type.content_type());
        ASSERT_EQ(content_type.first().kind(), TypeKind::ENUMERATION_TYPE);
        ASSERT_EQ(content_type.first().name(), "RootEnum");
        ASSERT_EQ(content_type.second().kind(), TypeKind::SEQUENCE_TYPE);
        const SequenceType& seq_type = static_cast<const SequenceType&>(content_type.second());
        ASSERT_EQ(seq_type.content_type().kind(), TypeKind::INT_16_TYPE);
        ASSERT_EQ(seq_type.bounds(), 0);
    }

    ASSERT_TRUE(root.has_submodule("ModuleA"));
    const Module& mod_A = root["ModuleA"];

    ASSERT_TRUE(mod_A.has_enum_32("ModAEnum"));
    const EnumerationType<uint32_t>& a_enum = mod_A.enum_32("ModAEnum");
    ASSERT_EQ(a_enum.value("A1"), 0);
    ASSERT_EQ(a_enum.value("A2"), 1);

    ASSERT_TRUE(mod_A.has_constant("A_UINT16"));
    uint16_t a_uint16 = mod_A.constant("A_UINT16");
    ASSERT_EQ(a_uint16, 55);

    ASSERT_TRUE(mod_A.has_structure("AStruct"));
    const StructType& a_struct = mod_A.structure("AStruct");
    ASSERT_EQ(a_struct.member("my_root_struct").type().kind(), TypeKind::STRUCTURE_TYPE);
    ASSERT_EQ(a_struct.member("my_root_struct").type().name(), "RootStruct");


    ASSERT_TRUE(mod_A.has_submodule("ModuleB"));
    const Module& mod_B = mod_A["ModuleB"];

    ASSERT_TRUE(mod_B.has_structure("BStruct"));
    const StructType& b_struct = mod_B.structure("BStruct");
    ASSERT_EQ(b_struct.member("my_deep_string").type().kind(), TypeKind::STRING_TYPE);

    ASSERT_TRUE(root.has_union("MyUnion"));
    const UnionType& root_union = root.union_switch("MyUnion");
    ASSERT_EQ(root_union.discriminator().kind(), root_enum.kind());
    ASSERT_EQ(root_union.discriminator().name(), root_enum.name());
    ASSERT_TRUE(root_union.has_member("union_enum"));
    ASSERT_TRUE(root_union.has_member("union_uint32"));
    ASSERT_TRUE(root_union.has_member("union_float"));
    std::vector<int64_t> labels = root_union.get_labels("union_enum");
    ASSERT_EQ(labels.size(), 2);
    ASSERT_EQ(labels[0], root_enum.value("VALUE_1"));
    ASSERT_EQ(labels[1], root_enum.value("VALUE_2"));
    ASSERT_FALSE(root_union.is_default("union_enum"));
    labels = root_union.get_labels("union_uint32");
    ASSERT_EQ(labels.size(), 1);
    ASSERT_EQ(labels[0], root_enum.value("VALUE_3"));
    ASSERT_FALSE(root_union.is_default("union_uint32"));
    labels = root_union.get_labels("union_float");
    ASSERT_EQ(labels.size(), 1);
    ASSERT_EQ(labels[0], root_enum.value("VALUE_4"));
    ASSERT_TRUE(root_union.is_default("union_float"));

    ASSERT_TRUE(root.has_union("MyScopedUnion"));
    const UnionType& root_scoped_union = root.union_switch("MyScopedUnion");
    ASSERT_EQ(root_scoped_union.discriminator().kind(), a_enum.kind());
    ASSERT_EQ(root_scoped_union.discriminator().name(), a_enum.name());
    ASSERT_TRUE(root_scoped_union.has_member("union_struct"));
    ASSERT_TRUE(root_scoped_union.has_member("union_uint8"));
    std::vector<int64_t> scoped_labels = root_scoped_union.get_labels("union_struct");
    ASSERT_EQ(scoped_labels.size(), 1);
    ASSERT_EQ(scoped_labels[0], a_enum.value("A1"));
    ASSERT_FALSE(root_scoped_union.is_default("union_struct"));
    scoped_labels = root_scoped_union.get_labels("union_uint8");
    ASSERT_EQ(scoped_labels.size(), 1);
    ASSERT_EQ(scoped_labels[0], a_enum.value("A2"));
    ASSERT_TRUE(root_scoped_union.is_default("union_uint8"));

    ASSERT_TRUE(root.has_structure("ParentStruct"));
    const StructType& parent_struct = root.structure("ParentStruct");
    ASSERT_EQ(parent_struct.member("parent_str").type().kind(), TypeKind::STRING_TYPE);
    ASSERT_EQ(parent_struct.members().size(), 1);
    ASSERT_TRUE(root.has_structure("ChildStruct"));
    const StructType& child_struct = root.structure("ChildStruct");
    ASSERT_EQ(child_struct.member("parent_str").type().kind(), TypeKind::STRING_TYPE);
    ASSERT_EQ(child_struct.member("child_uint").type().kind(), TypeKind::UINT_32_TYPE);
    ASSERT_EQ(child_struct.members().size(), 2);
    ASSERT_TRUE(child_struct.has_parent());
    ASSERT_EQ(child_struct.parent().name(), "ParentStruct");
    const StructType& gchild_struct = root.structure("GrandChildStruct");
    ASSERT_EQ(gchild_struct.member("parent_str").type().kind(), TypeKind::STRING_TYPE);
    ASSERT_EQ(gchild_struct.member("child_uint").type().kind(), TypeKind::UINT_32_TYPE);
    ASSERT_EQ(gchild_struct.member("gc_float").type().kind(), TypeKind::FLOAT_32_TYPE);
    ASSERT_EQ(gchild_struct.members().size(), 3);
    ASSERT_TRUE(gchild_struct.has_parent());
    ASSERT_EQ(gchild_struct.parent().name(), "ChildStruct");
}

TEST (IDLGenerator, roundtrip)
{
    Context context = parse(
        R"(
        typedef uint8 ByteMultiArray[6][7][8];

        enum RootEnum
        {
            VALUE_1,
            VALUE_2,
            VALUE_3,
            VALUE_4
        };

        const string ROOT_STRING = "RootString";

        struct RootStruct
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
            ByteMultiArray my_alias_array;
            sequence<uint32> my_unbound_uint32_seq;
            sequence<double, 10> my_bound10_double_seq;
            string my_string_array[VALUE_4];
            sequence<sequence<boolean, 5>, 3> my_nested_bool_seq5_3;
            int64 my_int64_nested_array[5][4];
            map<RootEnum, sequence<int16>> my_map;
        };

        module ModuleA
        {
            enum ModAEnum
            {
                A1,
                A2
            };

            const uint16 A_UINT16 = 55;

            struct AStruct
            {
                RootStruct my_root_struct;
            };

            module ModuleB
            {
                struct BStruct
                {
                    string my_deep_string;
                };
            };
        };

        union MyUnion switch (RootEnum)
        {
            case VALUE_1:
            case VALUE_2:
                RootEnum union_enum;
            case VALUE_3:
                uint32 union_uint32;
            case VALUE_4:
            default:
                float union_float;
        };

        union MyScopedUnion switch (ModuleA::ModAEnum)
        {
            case A1:
                RootStruct union_struct;
            case A2:
            default:
                uint8 union_uint8;
        };

        struct ParentStruct
        {
            string parent_str;
        };

        struct ChildStruct : ParentStruct
        {
            uint32 child_uint;
        };

        struct GrandChildStruct : ChildStruct
        {
            float gc_float;
        };
                   )");

    // Check the parser isn't broken
    ASSERT_TRUE(context.success);
    check_result(context.module());

    // Generate another IDL from the parsed root module.
    std::string gen_idl = generator::module(context.module());

    // Parse the generated IDL and check again.
    Context result = parse(gen_idl);
    check_result(result.module());
}

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
