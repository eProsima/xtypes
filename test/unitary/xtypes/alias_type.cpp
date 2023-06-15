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
#include <xtypes/idl/idl.hpp>
#include "../utils.hpp"

#define PI 3.14159f

using namespace eprosima::xtypes;

/***********************************************
 *         DynamicType AliasType Tests         *
 **********************************************/
TEST (AliasType, alias_name)
{
    AliasType at(WStringType(50), "wstr50");

    EXPECT_EQ("wstr50", at.name());
    EXPECT_EQ(TypeKind::ALIAS_TYPE, at.kind());
    EXPECT_EQ(TypeKind::WSTRING_TYPE, at->kind());
}

TEST (AliasType, alias_casting)
{
    AliasType at(StringType(100), "str100");

    ASSERT_OR_EXCEPTION({ (void)static_cast<const StructType&>(at); },
            "cannot be cast to the specified type");
}

TEST (AliasType, dynamic_alias_data)
{
    AliasType at(primitive_type<float>(), "flt32");
    StructType stdata("StructData");
    stdata.add_member("st0", at);
    stdata.add_member("st1", StringType());
    DynamicData data(stdata);

    data["st0"] = PI;
    const DynamicType& dt = data["st0"].type();
    ASSERT_EQ(dt.kind(), TypeKind::FLOAT_32_TYPE);
    ASSERT_EQ(static_cast<float>(data["st0"]), PI);
    ASSERT_OR_EXCEPTION({ data["st0"] = "This will fail"; }, "Expected type 'float'");
}

TEST (AliasType, recursive_dynamic_alias_data)
{
    AliasType at(primitive_type<uint32_t>(), "seconds");
    AliasType at1(at, "secs");
    AliasType at2(at1, "s");
    ASSERT_EQ(at.kind(), TypeKind::ALIAS_TYPE);
    ASSERT_EQ(at1.kind(), TypeKind::ALIAS_TYPE);
    ASSERT_EQ(at2.kind(), TypeKind::ALIAS_TYPE);
    ASSERT_EQ(at->kind(), TypeKind::UINT_32_TYPE);
    ASSERT_EQ(at1->kind(), TypeKind::ALIAS_TYPE);
    ASSERT_EQ(at2->kind(), TypeKind::ALIAS_TYPE);
    ASSERT_EQ(at1.rget().kind(), TypeKind::UINT_32_TYPE);
    ASSERT_EQ(at2.rget().kind(), TypeKind::UINT_32_TYPE);

    StructType stdata("StructData");
    stdata.add_member("st0", at);
    stdata.add_member("st1", at1);
    stdata.add_member("st2", at2);

    DynamicData data(stdata);
    data["st0"] = 20u;
    data["st1"] = 30u;
    data["st2"] = 40u;

    const DynamicType& dt0 = data["st0"].type();
    const DynamicType& dt1 = data["st1"].type();
    const DynamicType& dt2 = data["st2"].type();
    ASSERT_EQ(dt0.kind(), TypeKind::UINT_32_TYPE);
    ASSERT_EQ(dt1.kind(), TypeKind::UINT_32_TYPE);
    ASSERT_EQ(dt2.kind(), TypeKind::UINT_32_TYPE);
}

TEST (AliasType, alias_idl_generate)
{
    AliasType at(WStringType(), "utf8string");
    AliasType at1(at, "u8str");
    StructType stdata("StructData");
    stdata.add_member("st0", at);
    stdata.add_member("st1", primitive_type<int>());
    stdata.add_member("st2", at1);

    idl::Module m;
    m.add_alias(at);
    m.add_alias(at1);
    m.structure(stdata);

    std::string gen_idl = idl::generate(m);
    std::stringstream expected_idl;
    expected_idl << "typedef wstring utf8string;" << std::endl << std::endl;
    expected_idl << "typedef utf8string u8str;" << std::endl << std::endl;
    expected_idl << "struct StructData" << std::endl;
    expected_idl << "{" << std::endl;
    expected_idl << "    utf8string st0;" << std::endl;
    expected_idl << "    int32 st1;" << std::endl;
    expected_idl << "    u8str st2;" << std::endl;
    expected_idl << "};" << std::endl;
    ASSERT_EQ(gen_idl, expected_idl.str());
}
