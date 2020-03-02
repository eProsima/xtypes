// Copyright 2020 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

void generation_roundtrip_test(
        const std::vector<std::pair<std::string, std::string>>& module_elements)
{
    Module root;
    Module& mod_A = root.create_submodule("A");
    Module& mod_B = root.create_submodule("B");
    Module& mod_AB = mod_A.create_submodule("B");
    Module& mod_AC = mod_A.create_submodule("C");
    Module& mod_ABA = mod_AB.create_submodule("A");

    const std::map<const std::string, Module&> modules = {
        {"A",       mod_A  },
        {"B",       mod_B  },
        {"A::B",    mod_AB },
        {"A::C",    mod_AC },
        {"A::B::A", mod_ABA}
    };

    EnumerationType<uint32_t> my_enum("MyEnum");
    my_enum.add_enumerator("A", 0);
    my_enum.add_enumerator("B", 1);
    my_enum.add_enumerator("C", 2);

    DynamicData my_enum_data(my_enum);
    my_enum_data = my_enum.value("B");

    EnumerationType<uint32_t> my_union_enum("MyDiscriminatorEnum");
    my_union_enum.add_enumerator("Disc_1", 1);
    my_union_enum.add_enumerator("Disc_2", 2);
    my_union_enum.add_enumerator("Disc_3", 5);
    my_union_enum.add_enumerator("Disc_4", 7);

    AliasType my_bool_alias(primitive_type<bool>(), "MyBoolAlias");
    AliasType my_bool_alias_alias(my_bool_alias, "MyBoolAliasAlias");

    StructType my_struct("MyStruct");
    my_struct.add_member(Member("ms0", primitive_type<uint16_t>()));
    my_struct.add_member("ms1", my_enum);

    StructType my_child_struct("MyChildStruct", &my_struct);
    my_child_struct.add_member("mcs0", StringType(20));

    UnionType my_union("MyUnion", my_union_enum);
    std::vector<std::string> labels = {"Disc_1", "Disc_2"};
    my_union.add_case_member(labels, Member("my_union_alias", my_bool_alias_alias));
    std::vector<uint32_t> test(my_union_enum.value("Disc_3"));
    my_union.add_case_member(my_union_enum.value("Disc_3"), "my_union_float", primitive_type<float>());
    my_union.add_case_member(my_union_enum.value("Disc_4"), "my_union_struct", my_struct, true);

    for (const auto& pair : module_elements)
    {
        const std::string& submodule = pair.first;

        Module* module = &root;
        if (!submodule.empty())
        {
            ASSERT_TRUE(modules.count(submodule));
            module = &modules.at(submodule);
        }

        const std::string& type_name = pair.second;
        if (type_name == "MyEnum")
        {
            module->enum_32(std::move(my_enum));
        }
        else if (type_name == "MyDiscriminatorEnum")
        {
            module->enum_32(std::move(my_union_enum));
        }
        else if (type_name == "MyEnumConst")
        {
            module->create_constant("MyEnumConst", my_enum_data);
        }
        else if (type_name == "MyBoolAlias")
        {
            module->add_alias(my_bool_alias);
        }
        else if (type_name == "MyBoolAliasAlias")
        {
            module->add_alias(my_bool_alias_alias);
        }
        else if (type_name == "MyStruct")
        {
            module->structure(my_struct);
        }
        else if (type_name == "MyChildStruct")
        {
            module->structure(my_child_struct);
        }
        else if (type_name == "MyUnion")
        {
            module->union_switch(my_union);
        }
        else
        {
            xtypes_assert(false, "Undeclared type: '" << type_name << "'.");
        }
    }

    std::string gen_idl = idl::generate(root);
    // Debug
    // std::cout << "===========================================" << std::endl \
              << gen_idl \
              << "===========================================" << std::endl;
    //Parse again and check if it went as expected
    idl::Context context = idl::parse(gen_idl);
    ASSERT_TRUE(context.success);

    Module& root_gen = context.module();
    ASSERT_TRUE(root_gen.has_submodule("A"));
    Module& modA_gen = root_gen["A"];
    ASSERT_TRUE(modA_gen.has_submodule("B"));
    Module& modAB_gen = modA_gen["B"];
    ASSERT_TRUE(modA_gen.has_submodule("C"));
    Module& modAC_gen = modA_gen["C"];
    ASSERT_TRUE(modAB_gen.has_submodule("A"));
    Module& modABA_gen = modAB_gen["A"];
    ASSERT_TRUE(root_gen.has_submodule("B"));
    Module& modB_gen = root_gen["B"];

    for (const auto& pair : module_elements)
    {
        const std::string& submodule = pair.first;

        if (submodule == "")
        {
            ASSERT_TRUE(root_gen.has_symbol(pair.second));
        } else if (submodule == "A")
        {
            ASSERT_TRUE(modA_gen.has_symbol(pair.second));
        } else if (submodule == "B")
        {
            ASSERT_TRUE(modB_gen.has_symbol(pair.second));
        } else if (submodule == "A::B")
        {
            ASSERT_TRUE(modAB_gen.has_symbol(pair.second));
        } else if (submodule == "A::C")
        {
            ASSERT_TRUE(modAC_gen.has_symbol(pair.second));
        } else if (submodule == "A::B::A")
        {
            ASSERT_TRUE(modABA_gen.has_symbol(pair.second));
        } else
        {
            xtypes_assert(false, "Unknown module: '" << submodule << "'.");
        }
    }
}

TEST (IDLGenerator, dependencies)
{
    const std::vector<std::pair<std::string, std::string>> test1 =
    {
        {"",        "MyUnion"            },
        {"",        "MyChildStruct"      },
        {"",        "MyBoolAliasAlias"   },
        {"A::B",    "MyEnumConst"        },
        {"A::C",    "MyBoolAlias"        },
        {"A::C",    "MyDiscriminatorEnum"},
        {"A::B::A", "MyEnum"             },
        {"B",       "MyStruct"           },
    };
    generation_roundtrip_test(test1);

    const std::vector<std::pair<std::string, std::string>> test2 =
    {
        {"",        "MyEnum"             },
        {"",        "MyUnion"            },
        {"",        "MyEnumConst"        },
        {"",        "MyBoolAliasAlias"   },
        {"A",       "MyChildStruct"      },
        {"A::C",    "MyBoolAlias"        },
        {"A::B::A", "MyStruct"           },
        {"B",       "MyDiscriminatorEnum"},
    };
    generation_roundtrip_test(test2);

    const std::vector<std::pair<std::string, std::string>> test3 =
    {
        {"",        "MyStruct"           },
        {"",        "MyEnumConst"        },
        {"",        "MyBoolAliasAlias"   },
        {"",        "MyDiscriminatorEnum"},
        {"A::C",    "MyEnum"             },
        {"A::B::A", "MyBoolAlias"        },
        {"B",       "MyUnion"            },
        {"B",       "MyChildStruct"      },
    };
    generation_roundtrip_test(test3);

    // This test contains circular dependencies, hence it will fail until forward declarations are supported.
    // TODO: uncomment once FW_DECL feature is implemented.
    /*const std::vector<std::pair<std::string, std::string>> test4 =
    {
        {"B",       "MyStruct"           },
        {"",        "MyEnumConst"        },
        {"A::B::A", "MyBoolAliasAlias"   },
        {"A",       "MyDiscriminatorEnum"},
        {"A::C",    "MyEnum"             },
        {"",        "MyBoolAlias"        },
        {"B",       "MyUnion"            },
        {"A",       "MyChildStruct"      },
    };
    generation_roundtrip_test(test4);*/
}

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
