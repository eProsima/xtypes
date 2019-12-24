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

#define PI 3.14159f
#define XTYPES_ASSERT_ERRMSG(MSG) std::string("[XTYPES].*Assertion failed with message: ") + MSG

using namespace eprosima::xtypes;

/***********************************************
 *         DynamicType AliasType Tests         *
 **********************************************/
TEST (AliasType, alias_name)
{
    AliasType at(WStringType(50), "wstr50");

    EXPECT_EQ("wstr50", at.name());
    EXPECT_EQ(TypeKind::ALIAS_TYPE, at.kind());
    EXPECT_EQ(TypeKind::WSTRING_TYPE, at.get().kind());
}

TEST (AliasType, alias_casting)
{
    AliasType at(StringType(100), "str100");

    static_cast<const StringType&>(at); // This should work OK
    ASSERT_DEATH({ static_cast<const StructType&>(at); },
        XTYPES_ASSERT_ERRMSG("Alias \\[" + at.name() + "\\] cannot be cast to the specified type"));        
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
    ASSERT_DEATH({ data["st0"] = "This will fail"; },
        XTYPES_ASSERT_ERRMSG("Expected type 'float'"));
}

TEST (AliasType, alias_idl_generate)
{
    AliasType at(WStringType(), "utf8string");
    StructType stdata("StructData");
    stdata.add_member("st0", at);
    stdata.add_member("st1", primitive_type<int>());

    Module m;
    m.create_alias(at);
    m.structure(stdata);

    std::string gen_idl = idl::generate(m);
    std::stringstream expected_idl;
    expected_idl << "typedef wstring utf8string;" << std::endl;
    expected_idl << "struct StructData" << std::endl;
    expected_idl << "{" << std::endl;
    expected_idl << "    utf8string st0;" << std::endl;
    expected_idl << "    int32 st1;" << std::endl;
    expected_idl << "};" << std::endl;
    ASSERT_EQ(gen_idl, expected_idl.str());
}

TEST (AliasType, alias_idl_parse)
{
    std::string idl_spec = R"(
        typedef uint32 u32;
        typedef double longfloat;
        struct StructData
        {
            u32 st0;
            longfloat st1;
        };
    )";

    idl::Context context = idl::parse(idl_spec);
    Module& m_context = context.module();
    EXPECT_TRUE(m_context.has_alias("u32"));
    EXPECT_TRUE(m_context.has_alias("longfloat"));

    const StructType& st = m_context.structure("StructData");
    const DynamicType& dst0 = st.member("st0").type();
    const DynamicType& dst1 = st.member("st1").type();
    EXPECT_EQ(dst0.kind(), TypeKind::ALIAS_TYPE);
    EXPECT_EQ(dst1.kind(), TypeKind::ALIAS_TYPE);
    EXPECT_EQ(static_cast<const AliasType&>(dst0).get().kind(), TypeKind::UINT_32_TYPE);
    EXPECT_EQ(static_cast<const AliasType&>(dst1).get().kind(), TypeKind::FLOAT_64_TYPE);
}
