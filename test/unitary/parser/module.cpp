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
#include <xtypes/idl/Module.hpp>

using namespace eprosima::xtypes;
using namespace eprosima::xtypes::idl;

// This test checks the correct behavior of Modules, by checking visibility of the different structs from
// each possible scope. The expected results are the same that would apply to the C++ namespaces.
TEST (Modules, scope)
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
    ASSERT_TRUE(submod_A.has_structure("InnerType"));
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
    ASSERT_TRUE(submod_AA.has_structure("InnerType"));
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
    ASSERT_TRUE(submod_B.has_structure("InnerType"));
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
