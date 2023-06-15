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
#include <xtypes/DynamicData.hpp>
#include <iostream>

#include "../utils.hpp"

using namespace eprosima::xtypes;
using namespace eprosima::xtypes::idl;

TEST (IDLParser, check_grammar)
{
    peg::parser parser;

    parser.set_logger([](size_t line, size_t col, const std::string& msg, const std::string &) {
            std::cerr << line << ":" << col << ": " << msg << std::endl;
            });

    ASSERT_TRUE(parser.load_grammar(eprosima::xtypes::idl::idl_grammar())) << "grammar cannot be parsed";
}

TEST (IDLParser, simple_struct_test)
{
    Context context = parse(
        R"(
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
    std::map<std::string, DynamicType::Ptr> result = context.module().get_all_types();
    EXPECT_EQ(1, result.size());

    const StructType& my_struct = context.module().structure("SimpleStruct");
    DynamicData data(my_struct);

    data["my_bool"] = true;
    //data["my_int8"] = 'c';
    data["my_int8"] = static_cast<int8_t>(-55);
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
    data["my_string"] = "It works!";
    data["my_wstring"] = L"It works!";
    EXPECT_TRUE(data["my_bool"].value<bool>());
    EXPECT_EQ(-55, data["my_int8"].value<int8_t>());
    EXPECT_EQ(55, data["my_uint8"].value<uint8_t>());
    EXPECT_EQ(-5, data["my_int16"].value<int16_t>());
    EXPECT_EQ(6, data["my_uint16"].value<uint16_t>());
    EXPECT_EQ(-5, data["my_int32"].value<int32_t>());
    EXPECT_EQ(6, data["my_uint32"].value<uint32_t>());
    EXPECT_EQ(-5, data["my_int64"].value<int64_t>());
    EXPECT_EQ(6, data["my_uint64"].value<uint64_t>());
    EXPECT_EQ(5.55f, data["my_float"].value<float>());
    EXPECT_EQ(5.55, data["my_double"].value<double>());
    EXPECT_EQ(5.55l, data["my_long_double"].value<long double>());
    EXPECT_EQ('e', data["my_char"].value<char>());
    EXPECT_EQ(L'e', data["my_wchar"].value<wchar_t>());
    EXPECT_EQ("It works!", data["my_string"].value<std::string>());
    EXPECT_EQ(L"It works!", data["my_wstring"].value<std::wstring>());

    // Verify members order
    size_t idx = 0;
    EXPECT_EQ(data[idx++].type().name(), "bool");
    EXPECT_EQ(data[idx++].type().name(), "int8_t");
    EXPECT_EQ(data[idx++].type().name(), "uint8_t");
    EXPECT_EQ(data[idx++].type().name(), "int16_t");
    EXPECT_EQ(data[idx++].type().name(), "uint16_t");
    EXPECT_EQ(data[idx++].type().name(), "int32_t");
    EXPECT_EQ(data[idx++].type().name(), "uint32_t");
    EXPECT_EQ(data[idx++].type().name(), "int64_t");
    EXPECT_EQ(data[idx++].type().name(), "uint64_t");
    EXPECT_EQ(data[idx++].type().name(), "float");
    EXPECT_EQ(data[idx++].type().name(), "double");
    EXPECT_EQ(data[idx++].type().name(), "long double");
    EXPECT_EQ(data[idx++].type().name(), "char");
    EXPECT_EQ(data[idx++].type().name(), "wchar_t");
    EXPECT_EQ(data[idx++].type().name(), "std::string");
    EXPECT_EQ(data[idx++].type().name(), "std::wstring");
}

TEST (IDLParser, char16_t_test)
{
    Context context;
    context.wchar_type = Context::CHAR16_T;
    parse(
        R"(
        struct SimpleStruct
        {
            wchar my_wchar;
            wstring my_wstring;
        };
                   )",
        context);

    std::map<std::string, DynamicType::Ptr> result = context.module().get_all_types();
    EXPECT_EQ(1, result.size());

    const StructType& my_struct = context.module().structure("SimpleStruct");
    DynamicData data(my_struct);

    data["my_wchar"] = u'e';
    data["my_wstring"] = u"It works!";
    EXPECT_EQ(u'e', data["my_wchar"].value<char16_t>());
    EXPECT_EQ(u"It works!", data["my_wstring"].value<std::u16string>());
}

TEST (IDLParser, array_sequence_struct_test)
{
    Context context = parse(
        R"(
        struct SimpleStruct
        {
            boolean my_bool_5[5];
            int8 my_int8_3_2[3][2];
            string<16> my_string16;
            wstring<32> my_wstring32;
            sequence<int32> my_int_seq;
            sequence<char, 6> my_char6_seq;
            sequence<sequence<string, 2>, 3> my_seq_seq_str;
        };
                   )");
    std::map<std::string, DynamicType::Ptr> result = context.module().get_all_types();
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

    data["my_int8_3_2"][0][0] = static_cast<int8_t>('a');
    data["my_int8_3_2"][0][1] = static_cast<int8_t>('b');
    data["my_int8_3_2"][1][0] = static_cast<int8_t>('c');
    data["my_int8_3_2"][1][1] = static_cast<int8_t>('d');
    data["my_int8_3_2"][2][0] = static_cast<int8_t>('e');
    data["my_int8_3_2"][2][1] = static_cast<int8_t>('f');
    EXPECT_EQ(data["my_int8_3_2"][0][0].value<int8_t>(), 'a');
    EXPECT_EQ(data["my_int8_3_2"][0][1].value<int8_t>(), 'b');
    EXPECT_EQ(data["my_int8_3_2"][1][0].value<int8_t>(), 'c');
    EXPECT_EQ(data["my_int8_3_2"][1][1].value<int8_t>(), 'd');
    EXPECT_EQ(data["my_int8_3_2"][2][0].value<int8_t>(), 'e');
    EXPECT_EQ(data["my_int8_3_2"][2][1].value<int8_t>(), 'f');

    EXPECT_EQ(data["my_string16"].bounds(), 16);
    // data["my_string16"] = "0123456789abcdefghijklmnopqrstuvwxyz" ;
    // EXPECT_EQ(data["my_string16"].size(), 16);
    data["my_string16"] = "0123456789";
    EXPECT_EQ(data["my_string16"].size(), 10);
    EXPECT_EQ(data["my_string16"].bounds(), 16);
    EXPECT_EQ(data["my_string16"].value<std::string>(), "0123456789");

    EXPECT_EQ(data["my_wstring32"].bounds(), 32);
    // data["my_wstring32"] = L"0123456789abcdefghijklmnñopqrstuvwxyz" ;
    // EXPECT_EQ(data["my_wstring32"].size(), 32);
    data["my_wstring32"] = L"0123456789";
    EXPECT_EQ(data["my_wstring32"].size(), 10);
    EXPECT_EQ(data["my_wstring32"].bounds(), 32);
    EXPECT_EQ(data["my_wstring32"].value<std::wstring>(), L"0123456789");

    for (int32_t i = 0; i < 300; ++i)
    {
        data["my_int_seq"].push(i);
    }
    for (int32_t i = 0; i < 300; ++i)
    {
        EXPECT_EQ(data["my_int_seq"][i].value<int32_t>(), i);
    }
    EXPECT_EQ(data["my_int_seq"].size(), 300);

    EXPECT_EQ(data["my_char6_seq"].bounds(), 6);
    for (int32_t i = 0; i < 7; ++i)
    {
        if (i < 6)
        {
            data["my_char6_seq"].push(static_cast<char>(i));
        }
        else
        {
            ASSERT_OR_EXCEPTION(data["my_char6_seq"][i].push(static_cast<char>(i)); , "out of bounds");
        }
    }
    for (int32_t i = 0; i < 7; ++i)
    {
        if (i < 6)
        {
            EXPECT_EQ(data["my_char6_seq"][i].value<char>(), static_cast<char>(i));
        }
        else
        {
            ASSERT_OR_EXCEPTION(data["my_char6_seq"][i]; , "out of bounds");
        }
    }
    EXPECT_EQ(data["my_char6_seq"].size(), 6);
    EXPECT_EQ(data["my_char6_seq"].bounds(), 6);

    for (int32_t i = 0; i < 3; ++i)
    {
        DynamicData seq(static_cast<const SequenceType&>(data["my_seq_seq_str"].type()).content_type());
        for (int32_t j = 0; j < 2; ++j)
        {
            seq.push(std::to_string(j + (i * 2)));
        }
        data["my_seq_seq_str"].push(seq);
    }

    for (int32_t i = 0; i < 3; ++i)
    {
        for (int32_t j = 0; j < 2; ++j)
        {
            EXPECT_EQ(data["my_seq_seq_str"][i][j].value<std::string>(), std::to_string(j + (i * 2)));
        }
    }
}

TEST (IDLParser, inner_struct_test)
{
    Context context = parse(
        R"(
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
    std::map<std::string, DynamicType::Ptr> result = context.module().get_all_types();
    EXPECT_EQ(3, result.size());

    const DynamicType* my_struct = result["SuperStruct"].get();
    DynamicData data(*my_struct);
    DynamicData rec_data(*result["RecursiveStruct"].get());

    data["inner"]["message"] = "It works!";
    EXPECT_EQ("It works!", data["inner"]["message"].value<std::string>());
}

TEST (IDLParser, multiple_declarator_members_test)
{
    Context context = parse(
        R"(
        struct SimpleStruct
        {
            boolean my_bool_5[5], other[55], another, multi_array[2][3];
        };
                   )");
    std::map<std::string, DynamicType::Ptr> result = context.module().get_all_types();
    EXPECT_EQ(1, result.size());
}

TEST (IDLParser, name_collision)
{
    {
        // Test that the parser throws an exception when using a keyword (ignoring case) as identifier.
        try
        {
            Context context;
            context.ignore_case = true;

            context = parse(R"(
                struct MyStruct
                {
                    string STRUCT;
                };)", context);

            FAIL() << " Exception wasn't thrown!" << std::endl;
        }
        catch (const Parser::exception& e)
        {
            if (std::string(e.what()).find("reserved word") == std::string::npos)
            {
                FAIL() << " Another Parser::exception was thrown." << std::endl;
            }
        }
        catch (...)
        {
            FAIL() << " Unexpected exception catch" << std::endl;
        }
    }

    {
        // Test that the parser accepts an uppercase keyword when case isn't ignored.
        Context context = parse(
            R"(
            struct Struct
            {
                string STRUCT;
            };)");

        std::map<std::string, DynamicType::Ptr> result = context.module().get_all_types();
        EXPECT_EQ(1, result.size());
        const DynamicType* my_struct = result["Struct"].get();
        EXPECT_EQ(my_struct->name(), "Struct");
    }

    {
        // Test that the parser accepts a keyword when the usage of keywords as identifiers is allowed.
        Context context;
        context.allow_keyword_identifiers = true;
        parse(R"(
            struct struct
            {
                string string;
            };)", context);

        std::map<std::string, DynamicType::Ptr> result = context.module().get_all_types();
        EXPECT_EQ(1, result.size());
    }

    {
        // Test that the parser accepts a keyword prefixed by an underscore even ignoring case, and
        // the resulting identifier doesn't have the prefixed underscore.
        Context context;
        parse(R"(
            struct MyStruct
            {
                string _struct;
            };)", context);

        std::map<std::string, DynamicType::Ptr> result = context.module().get_all_types();
        EXPECT_EQ(1, result.size());

        const DynamicType* my_struct = result["MyStruct"].get();
        DynamicData data(*my_struct);
        data["struct"] = "It works!";
        EXPECT_EQ("It works!", data["struct"].value<std::string>());
    }

    {
        // Test that the parser throws an exception when using an already defined symbol as identifier.
        try
        {
            Context context = parse(R"(
                struct MyStruct
                {
                    uint32 MyStruct;
                };)");

            FAIL() << " Exception wasn't thrown!" << std::endl;
        }
        catch (const Parser::exception& e)
        {
            if (std::string(e.what()).find("already") == std::string::npos)
            {
                FAIL() << " Another Parser::exception was thrown." << std::endl;
            }
        }
        catch (...)
        {
            FAIL() << " Unexpected exception catch" << std::endl;
        }
    }

    {
        // Test that the parser throws an exception when using an already defined symbol as identifier (II).
        try
        {
            Context context = parse(R"(
                struct MyStruct
                {
                    uint32 a;
                    string a;
                };)");

            FAIL() << " Exception wasn't thrown!" << std::endl;
        }
        catch (const Parser::exception& e)
        {
            if (std::string(e.what()).find("already") == std::string::npos)
            {
                FAIL() << " Another Parser::exception was thrown." << std::endl;
            }
        }
        catch (...)
        {
            FAIL() << " Unexpected exception catch" << std::endl;
        }
    }

    {
        // Test that the parser throws an exception when using an already defined symbol as identifier (III).
        try
        {
            Context context = parse(R"(
                struct MyStruct
                {
                    uint32 a, a;
                };)");

            FAIL() << " Exception wasn't thrown!" << std::endl;
        }
        catch (const Parser::exception& e)
        {
            if (std::string(e.what()).find("already") == std::string::npos)
            {
                FAIL() << " Another Parser::exception was thrown." << std::endl;
            }
        }
        catch (...)
        {
            FAIL() << " Unexpected exception catch" << std::endl;
        }
    }
}

TEST (IDLParser, module_scope_test)
{
    Context context = parse(
        R"(
        module A
        {
            struct StA;
        };

        module B
        {
            module C
            {
                struct StBC
                {
                    A::StA st_a;
                };
            };

            struct StB
            {
                C::StBC st_bc;
            };
        };

        module A
        {
            struct StA
            {
                string my_string;
            };

            struct StD
            {
                ::B::C::StBC st_bc;
            };
        };

        struct CompleteStruct
        {
            A::StA a;
            B::StB b;
            B::C::StBC bc;
            ::A::StD d;
        };
                   )");

    std::map<std::string, DynamicType::Ptr> result = context.get_all_scoped_types();
    EXPECT_EQ(5, result.size());

    DynamicType::Ptr StA = result.at("A::StA");
    EXPECT_TRUE(StA.get() != nullptr);

    DynamicType::Ptr StBC = result.at("B::C::StBC");
    EXPECT_TRUE(StBC.get() != nullptr);

    EXPECT_TRUE(result.count("StBC") == 0);

    DynamicType::Ptr mainSt = result.at("CompleteStruct");
    EXPECT_TRUE(mainSt.get() != nullptr);
}

TEST (IDLParser, constants)
{
    try
    {
        Context context = parse(
            R"(
            const uint32 MAX_SIZE = 32 / 2;
            const uint32 SUPER_MAX = MAX_SIZE * 1000 << 5;
                       )");
    }
    catch (const Parser::exception& exc)
    {
        FAIL() << exc.what() << std::endl;
    }

    try
    {
        Context context = parse(R"(
            const string C_STRING = "Hola";
                       )");
    }
    catch (const Parser::exception& exc)
    {
        FAIL() << exc.what() << std::endl;
    }

    ASSERT_OR_EXCEPTION(
    {
        Context context = parse(R"(
                const string C_STRING = "Hola" + 55;
            )");
    },
        "Assertion failed");

    try
    {
        Context context = parse(
            R"(
            const string C_STRING = "Hola";
            const string C_STRING_2 = C_STRING;
            const string C_STRING_3 = "Hey, " "Adios!!"
                " Esto debe estar conca"   "tenado";
                       )");
    }
    catch (const Parser::exception& exc)
    {
        FAIL() << exc.what() << std::endl;
    }

    try
    {
        Context context = parse(R"(
            const float BAD_TYPE = "Hola";
                       )");
        FAIL() << "Exception not thown.";
    }
    catch (const Parser::exception& exc)
    {
        FAIL() << exc.what() << std::endl;
    }
    catch (const std::exception& exc)
    {
        std::string msg = exc.what();
        if (msg.find("stof") == std::string::npos)
        {
            FAIL() << "Unexpected exception";
        }
    }

    try
    {
        Context context = parse(R"(
            const uint64 BAD_TYPE = 55.8;
                       )");
    }
    catch (const Parser::exception& exc)
    {
        FAIL() << exc.what() << std::endl; // TODO?
    }

    {
        Context context = parse(
            R"(
            const uint32 SIZE = 50;

            struct MyStruct
            {
                string my_str_array[SIZE];
                sequence<long, SIZE> my_seq;
                string<SIZE> my_bounded_str;
            };
                       )");

        std::map<std::string, DynamicType::Ptr> result = context.module().get_all_types();
        EXPECT_EQ(1, result.size());

        const DynamicType* my_struct = result["MyStruct"].get();
        DynamicData data(*my_struct);
        ASSERT_EQ(data["my_str_array"].bounds(), 50);
        ASSERT_EQ(data["my_seq"].bounds(), 50);
        ASSERT_EQ(data["my_bounded_str"].bounds(), 50);
    }
}

TEST (IDLParser, not_yet_supported)
//TEST (IDLParser, DISABLED_not_yet_supported)
{
    try
    {
        Context context = parse(
            R"(
            const uint32 MAX_SIZE = 32 / 2;

            module A
            {
                struct MyStruct
                {
                    uint64 my_uint64;
                };
            };

            struct ForwardStruct;

            struct FutureStruct
            {
                map<int32, string, 5> my_map;
                A::MyStruct scoped_struct;
                ForwardStruct fwd_struct;
            };

            struct ForwardStruct
            {
                string<MAX_SIZE> my_string;
            };

            enum MyEnum
            {
                AAA,
                BBB,
                CCC
            };

            union ForwardUnion;

            union MyUnion switch (int32)
            {
                case AAA: string str_a;
                case BBB: wstring wstr_b;
                case CCC: ForwardUnion union_c;
            };

            union ForwardUnion switch (octet)
            {
                case 0: int32 my_int32;
                case 1: uint64 my_uint64;
                default: string my_string;
            };

            bitset MyBitset
            {
                bitfield<3> a;
                bitfield<1> b;
                bitfield<4>;
                bitfield<10, long> c;
            };

            bitmask MyBitmask
            {
                flag0,
                flag1,
                @position(5) flag5,
                flag6
            };

            typedef uint8 MyArray[8];
            typedef string MyString[2][3][4];
            typedef FutureStruct future_is_now;
                       )");
    }
    catch (const Parser::exception& exc)
    {
        std::cout << exc.what() << std::endl;
    }
    /*
       EXPECT_EQ(1, result.size());

       const DynamicType* my_struct = result["FutureStruct"].get();
       DynamicData data(*my_struct);
     */
}

TEST (IDLParser, const_value_parser)
{
    {
        uint32_t value = (998 + 8) * 8;
        Context context = parse(
            R"(
            const uint32 SIZE = (998 + 8) * 8;

            struct MyStruct
            {
                string my_str_array[SIZE];
                sequence<long, SIZE> my_seq;
                string<SIZE> my_bounded_str;
            };
                       )");

        std::map<std::string, DynamicType::Ptr> result = context.module().get_all_types();
        EXPECT_EQ(1, result.size());

        const DynamicType* my_struct = result["MyStruct"].get();
        DynamicData data(*my_struct);
        ASSERT_EQ(data["my_str_array"].bounds(), value);
        ASSERT_EQ(data["my_seq"].bounds(), value);
        ASSERT_EQ(data["my_bounded_str"].bounds(), value);
    }
}

TEST (IDLParser, parse_file)
{
    Context context = parse_file("idl/test01.idl");
    ASSERT_TRUE(context.success);
    std::map<std::string, DynamicType::Ptr> result = context.module().get_all_types();
    EXPECT_EQ(1, result.size());
    const DynamicType* my_struct = result["Test01"].get();
    DynamicData data(*my_struct);
    ASSERT_EQ(data["my_long"].type().name(), "int32_t");
    ASSERT_EQ(data["my_short"].type().name(), "int16_t");
    ASSERT_EQ(data["my_long_long"].type().name(), "int64_t");
}

TEST (IDLParser, include_from_string)
{
    Context context = parse(
        R"(
        #include "idl/include/test_include.idl"

        module include
        {
            struct Test00
            {
                TestInclude incl;
            };
        };
        )");
    std::map<std::string, DynamicType::Ptr> result = context.module().get_all_types();
    ASSERT_EQ(2, result.size());
    const DynamicType* my_struct = result["Test00"].get();
    DynamicData data(*my_struct);
    ASSERT_EQ(data["incl"]["my_string"].type().name(), "std::string");
}

TEST (IDLParser, include_from_file_02_local)
{
    Context context;
    context.include_paths.push_back("idl");
    parse_file("idl/test02.idl", context);
    std::map<std::string, DynamicType::Ptr> result = context.module().get_all_types();
    EXPECT_EQ(2, result.size());
    const DynamicType* my_struct = result["Test02"].get();
    DynamicData data(*my_struct);
    ASSERT_EQ(data["my_include"]["my_string"].type().name(), "std::string");
}

TEST (IDLParser, include_from_file_03_global)
{
    Context context;
    context.include_paths.push_back("idl/include");
    parse_file("idl/test03.idl", context);
    std::map<std::string, DynamicType::Ptr> result = context.module().get_all_types();
    EXPECT_EQ(2, result.size());
    const DynamicType* my_struct = result["Test03"].get();
    DynamicData data(*my_struct);
    ASSERT_EQ(data["my_include"]["my_string"].type().name(), "std::string");
}

TEST (IDLParser, include_from_file_04_multi)
{
    Context context;
    context.include_paths.push_back("idl");
    context.include_paths.push_back("idl/include");
    parse_file("idl/test04.idl", context);
    std::map<std::string, DynamicType::Ptr> result = context.module().get_all_types();
    EXPECT_EQ(3, result.size());
    const DynamicType* my_struct = result["Test04"].get();
    DynamicData data(*my_struct);
    ASSERT_EQ(data["my_include"]["my_string"].type().name(), "std::string");
    ASSERT_EQ(data["my_test03"]["my_include"]["my_string"].type().name(), "std::string");
}

TEST (IDLParser, real_world_parsing)
{
    std::string idl_content =
            R"(
        module geometry_msgs {
            module msg {
                struct Point {
                    double x;
                    double y;
                    double z;
                };
            };
        };
        module geometry_msgs {
            module msg {
                struct Quaternion {
                    double x;
                    double y;
                    double z;
                    double w;
                };
            };
        };
        module geometry_msgs {
            module msg {
                struct Pose {
                    geometry_msgs::msg::Point position;
                    geometry_msgs::msg::Quaternion orientation;
                };
            };
        };
        module builtin_interfaces {
            module msg {
                struct Time {
                    int32 sec;
                    uint32 nanosec;
                };
            };
        };
        module std_msgs {
            module msg {
                struct Header {
                    builtin_interfaces::msg::Time stamp;
                    string frame_id;
                };
            };
        };
        module geometry_msgs {
            module msg {
                struct PoseStamped {
                    std_msgs::msg::Header header;
                    geometry_msgs::msg::Pose pose;
                };
            };
        };
        module geometry_msgs {
            module msg {
                struct Point {
                    double x;
                    double y;
                    double z;
                };
            };
        };
        module geometry_msgs {
            module msg {
                struct Quaternion {
                    double x;
                    double y;
                    double z;
                    double w;
                };
            };
        };
        module geometry_msgs {
            module msg {
                struct Pose {
                    geometry_msgs::msg::Point position;
                    geometry_msgs::msg::Quaternion orientation;
                };
            };
        };
        module builtin_interfaces {
            module msg {
                struct Time {
                    int32 sec;
                    uint32 nanosec;
                };
            };
        };
        module std_msgs {
            module msg {
                struct Header {
                    builtin_interfaces::msg::Time stamp;
                    string frame_id;
                };
            };
        };
        module geometry_msgs {
            module msg {
                struct PoseStamped {
                    std_msgs::msg::Header header;
                    geometry_msgs::msg::Pose pose;
                };
            };
        };
        module builtin_interfaces {
            module msg {
                struct Time {
                    int32 sec;
                    uint32 nanosec;
                };
            };
        };
        module std_msgs {
            module msg {
                struct Header {
                    builtin_interfaces::msg::Time stamp;
                    string frame_id;
                };
            };
        };
        module nav_msgs {
            module msg {
                struct Path {
                    std_msgs::msg::Header header;
                    sequence<geometry_msgs::msg::PoseStamped> poses;
                };
            };
        };
        module nav_msgs {
            module srv {
                struct GetPlan_Request {
                    geometry_msgs::msg::PoseStamped start;
                    geometry_msgs::msg::PoseStamped goal;
                    float tolerance;
                };
                struct GetPlan_Response {
                    nav_msgs::msg::Path plan;
                };
            };
        };
    )";

    Context context;
    context.allow_keyword_identifiers = true;
    context.ignore_redefinition = true;
    parse(idl_content, context);
    ASSERT_TRUE(context.success);
}

TEST (IDLParser, enumerations_test)
{
    {
        Context context = parse(
            R"(
            enum MyEnum
            {
                A,
                B,
                C
            };

            const uint32 D = B + C;

            struct MyStruct
            {
                string my_str_array[B];
                sequence<long, C> my_seq;
                string<D> my_bounded_str;
            };
                       )");

        std::map<std::string, DynamicType::Ptr> result = context.get_all_types();
        EXPECT_EQ(2, result.size());

        const DynamicType* my_struct = result["MyStruct"].get();
        DynamicData data(*my_struct);
        ASSERT_EQ(data["my_str_array"].bounds(), 1);
        ASSERT_EQ(data["my_seq"].bounds(), 2);
        ASSERT_EQ(data["my_bounded_str"].bounds(), 3);

        EnumerationType<uint32_t>& my_enum = context.module().enum_32("MyEnum");
        ASSERT_EQ(my_enum.value("A"), 0);
        ASSERT_EQ(my_enum.value("B"), 1);
        ASSERT_EQ(my_enum.value("C"), 2);
    }
}

TEST (IDLParser, bad_idl_logging)
{
    Context context;
    context.log_level(log::LogLevel::xDEBUG);
    // context.print_log(true); // Useful for debbuging
    context.preprocess = false;
    parse(R"~~(
        // This is not a well formed IDL file
        struct Str
        {
            323241 std_string;
        };
        )~~",
            context);

    std::vector<log::LogEntry> log = context.log();
    ASSERT_GT(log.size(), 0);

    uint32_t log_peglib_error = 0;
    uint32_t log_result = 0;

    for (const log::LogEntry& entry : log)
    {
        if (entry.category == "PEGLIB_PARSER")
        {
            if (entry.message.find("syntax error") != std::string::npos)
            {
                ++log_peglib_error;
            }
            else
            {
                FAIL() << "Unexpected log entry: " << entry.to_string();
            }
        }
        else if (entry.category == "RESULT")
        {
            if (entry.message.find("found errors") != std::string::npos)
            {
                ++log_result;
            }
            else
            {
                FAIL() << "Unexpected log entry: " << entry.to_string();
            }
        }
        else
        {
            FAIL() << "Unexpected log entry: " << entry.to_string();
        }
    }

    ASSERT_EQ(log_peglib_error, 1);
    ASSERT_EQ(log_result, 1);
}

TEST (IDLParser, logging)
{
    Context context;
    context.log_level(log::LogLevel::xINFO);
    // context.print_log(true); // Useful for debbuging
    context.preprocess = false;
    context.allow_keyword_identifiers = true;
    context.ignore_redefinition = true;
    context.ignore_case = true;
    parse(R"~~(
        struct Struct
        {
            int32 int32;
        };
        struct Struct
        {
            int64 int64;
        };
        )~~",
            context);

    std::vector<log::LogEntry> log = context.log();
    ASSERT_GT(log.size(), 0);

    // Expected logs are:
    // INFO - 'Struct' reserved word x 2
    // INFO - 'int32' reserved word
    // INFO - 'Struct' already used
    // INFO - 'int64' reserved word
    // INFO - 'Struct' redefinition
    uint32_t log_struct_reserved = 0;
    uint32_t log_int32_reserved = 0;
    uint32_t log_int64_reserved = 0;
    uint32_t log_struct_already_used = 0;
    uint32_t log_struct_redefinition = 0;

    for (const log::LogEntry& entry : log)
    {
        if (entry.category == "RESERVED_WORD")
        {
            if (entry.message.find("Struct") != std::string::npos)
            {
                ++log_struct_reserved;
            }
            else if (entry.message.find("int32") != std::string::npos)
            {
                ++log_int32_reserved;
            }
            else if (entry.message.find("int64") != std::string::npos)
            {
                ++log_int64_reserved;
            }
            else
            {
                FAIL() << "Unexpected log entry: " << entry.to_string();
            }
        }
        else if (entry.category == "ALREADY_USED")
        {
            if (entry.message.find("Struct") != std::string::npos)
            {
                ++log_struct_already_used;
            }
            else
            {
                FAIL() << "Unexpected log entry: " << entry.to_string();
            }
        }
        else if (entry.category == "REDEFINITION")
        {
            if (entry.message.find("Struct") != std::string::npos)
            {
                ++log_struct_redefinition;
            }
            else
            {
                FAIL() << "Unexpected log entry: " << entry.to_string();
            }
        }
        else
        {
            FAIL() << "Unexpected log entry: " << entry.to_string();
        }
    }

    ASSERT_EQ(log_struct_reserved, 2);
    ASSERT_EQ(log_int32_reserved, 1);
    ASSERT_EQ(log_int64_reserved, 1);
    ASSERT_EQ(log_struct_already_used, 1);
    ASSERT_EQ(log_struct_redefinition, 1);
}

#define EXPECTED_LOG_RESULTS(LOG_LEVEL, N_ENTRIES, ASSERT, PRINT)                                                   \
    {                                                                                                                   \
        Context context;                                                                                                \
        context.log_level(log::LogLevel::LOG_LEVEL);                                                                    \
        if (PRINT)                                                                                                       \
        {                                                                                                               \
            context.print_log(true);                                                                                    \
        }                                                                                                               \
        context.preprocess = false;                                                                                     \
        context.allow_keyword_identifiers = true;                                                                       \
        parse(idl_str, context);                                                                                        \
                                                                                                                    \
        std::vector<log::LogEntry> log = context.log();                                                                 \
        ASSERT(log.size(), N_ENTRIES);                                                                                  \
    }

#define EXPECTED_LOG_RESULTS_FILTERED(LOG_LEVEL, N_ENTRIES, ASSERT, PRINT)                                          \
    {                                                                                                                   \
        Context context;                                                                                                \
        context.log_level(log::LogLevel::xDEBUG);                                                                        \
        if (PRINT)                                                                                                       \
        {                                                                                                               \
            context.print_log(true);                                                                                    \
        }                                                                                                               \
        context.preprocess = false;                                                                                     \
        context.allow_keyword_identifiers = true;                                                                       \
        parse(idl_str, context);                                                                                        \
                                                                                                                    \
        std::vector<log::LogEntry> log = context.log(log::LogLevel::LOG_LEVEL, true);                                   \
        ASSERT(log.size(), N_ENTRIES);                                                                                  \
    }

TEST (IDLParser, severity_logging)
{
    const std::string idl_str =
            R"~~(
        struct MyStruct // DEBUG
        {
            int32 int32; // DEBUG + INFO
        }; // DEBUG + DEBUG

        const MyStruct NOT_VALID = 666; // DEBUG + ERROR

        const boolean BAD_LITERAL = 55; // DEBUG + WARNING + WARNING + DEBUG (result)
        )~~";

    EXPECTED_LOG_RESULTS(xERROR, 1, ASSERT_EQ, false);
    EXPECTED_LOG_RESULTS(xWARNING, 3, ASSERT_EQ, false);
    EXPECTED_LOG_RESULTS(xINFO, 4, ASSERT_EQ, false);
    EXPECTED_LOG_RESULTS(xDEBUG, 11, ASSERT_EQ, false);
    EXPECTED_LOG_RESULTS_FILTERED(xERROR, 1, ASSERT_EQ, false);
    EXPECTED_LOG_RESULTS_FILTERED(xWARNING, 2, ASSERT_EQ, false);
    EXPECTED_LOG_RESULTS_FILTERED(xINFO, 1, ASSERT_EQ, false);
    EXPECTED_LOG_RESULTS_FILTERED(xDEBUG, 7, ASSERT_EQ, false);
}

TEST(IDLParser, alias_test)
{
    std::string idl_spec =
            R"(
        typedef uint32 u32;
        typedef double longfloat;
        typedef longfloat lfloat;
        typedef lfloat lflt;
        typedef uint8 ByteMultiArray[2][3];
        struct StructData
        {
            u32 st0;
            longfloat st1;
            lfloat st2;
            lflt st3;
            ByteMultiArray st4;
        };
    )";

    Context context = parse(idl_spec);
    ASSERT_TRUE(context.success);

    Module& m_context = context.module();
    EXPECT_TRUE(m_context.has_alias("u32"));
    EXPECT_TRUE(m_context.has_alias("longfloat"));
    EXPECT_TRUE(m_context.has_alias("lfloat"));
    EXPECT_TRUE(m_context.has_alias("lflt"));
    EXPECT_TRUE(m_context.has_alias("ByteMultiArray"));

    const StructType& st = m_context.structure("StructData");
    const DynamicType& dst0 = st.member("st0").type();
    const DynamicType& dst1 = st.member("st1").type();
    const DynamicType& dst2 = st.member("st2").type();
    const DynamicType& dst3 = st.member("st3").type();
    const DynamicType& dst4 = st.member("st4").type();
    EXPECT_EQ(dst0.kind(), TypeKind::ALIAS_TYPE);
    EXPECT_EQ(dst1.kind(), TypeKind::ALIAS_TYPE);
    EXPECT_EQ(dst2.kind(), TypeKind::ALIAS_TYPE);
    EXPECT_EQ(dst3.kind(), TypeKind::ALIAS_TYPE);
    EXPECT_EQ(dst4.kind(), TypeKind::ALIAS_TYPE);
    EXPECT_EQ(static_cast<const AliasType&>(dst0).get().kind(), TypeKind::UINT_32_TYPE);
    EXPECT_EQ(static_cast<const AliasType&>(dst1).get().kind(), TypeKind::FLOAT_64_TYPE);
    EXPECT_EQ(static_cast<const AliasType&>(dst2).get().kind(), TypeKind::ALIAS_TYPE);
    EXPECT_EQ(static_cast<const AliasType&>(dst2).rget().kind(), TypeKind::FLOAT_64_TYPE);
    EXPECT_EQ(static_cast<const AliasType&>(dst3).get().kind(), TypeKind::ALIAS_TYPE);
    EXPECT_EQ(static_cast<const AliasType&>(dst3).rget().kind(), TypeKind::FLOAT_64_TYPE);
    EXPECT_EQ(static_cast<const AliasType&>(dst4).get().kind(), TypeKind::ARRAY_TYPE);
}

TEST (IDLParser, alias_redefinition)
{
    std::string idl_spec =
            R"(
        typedef uint32 u32;
        typedef double longfloat;
        typedef longfloat lfloat;
        typedef uint32 u32;
    )";

    Context context;
    context.ignore_redefinition = true;
    parse(idl_spec, context);
    ASSERT_TRUE(context.success);
}

TEST (IDLParser, union_tests)
{
    Context context = parse(
        R"(
        enum MyEnum
        {
            AAA,
            BBB,
            CCC
        };

        typedef char MyChar;
        const MyChar C_CHAR = 'L';
        const MyEnum C_ENUM = CCC;

        union ForwardUnion switch (uint64)
        {
            case 0: int32 my_int32;
            case 1: uint64 my_uint64;
            case C_ENUM:
            case 3: float my_float;
            default: string my_string;
        };

        union MyUnion switch (MyEnum)
        {
            case AAA: string str_a;
            case BBB: wstring wstr_b;
            case CCC: ForwardUnion union_c;
        };

        union TestUnion switch (MyChar)
        {
            case 'a': string a;
            case 'b': string b;
            case C_CHAR: string c;
        };

                   )");

    std::map<std::string, DynamicType::Ptr> result = context.module().get_all_types();
    EXPECT_EQ(5, result.size());

    const UnionType& my_union = context.module().union_switch("MyUnion");
    DynamicData data(my_union);

    data["str_a"] = "Testing";
    EXPECT_EQ(data.d().value<uint32_t>(), 0);

    data["wstr_b"] = L"Testing Wstring";
    EXPECT_EQ(data.d().value<uint32_t>(), 1);

    EXPECT_EQ(data["union_c"].d().value<uint64_t>(), static_cast<uint64_t>(default_union_label(sizeof(uint64_t))));
    data["union_c"]["my_string"] = "Correct";

    data["union_c"]["my_float"] = 3.14f;
    EXPECT_EQ(data["union_c"].d().value<uint64_t>(), 2);

    data["union_c"].d(3);
    EXPECT_EQ(data["union_c"].d().value<uint64_t>(), 3);

    data["union_c"]["my_uint64"] = UINT64_C(314);
    EXPECT_EQ(data["union_c"].d().value<uint64_t>(), 1);

    data["union_c"]["my_int32"] = INT32_C(314);
    EXPECT_EQ(data["union_c"].d().value<uint64_t>(), 0);

    const UnionType& test_union = context.module().union_switch("TestUnion");
    DynamicData data_2(test_union);
    data_2["b"] = "Testing!";
    EXPECT_EQ(data_2.d().value<char>(), 'b');
}

TEST (IDLParser, map_tests)
{
    Context context = parse(
        R"(
        enum MyEnum
        {
            AAA,
            BBB,
            CCC
        };

        typedef char MyChar;

        struct KeyStruct
        {
            string my_string;
        };

        struct MyStruct
        {
            map<uint32, string> map_1;
            map<KeyStruct, MyEnum> map_2;
            map<string, MyChar> map_3;
            map<MyChar, float> map_4;
            map<MyEnum, KeyStruct> map_5;
            map<string, map<uint32, string>> map_6;
        };

                   )");

    std::map<std::string, DynamicType::Ptr> result = context.module().get_all_types();
    EXPECT_EQ(4, result.size());

    const StructType& my_struct = context.module().structure("MyStruct");
    const StructType& my_key = context.module().structure("KeyStruct");
    const AliasType& my_alias = context.module().alias("MyChar");
    const EnumerationType<uint32_t>& my_enum = context.module().enum_32("MyEnum");
    DynamicData data(my_struct);

    StringType str_type;
    DynamicData map_1_key(primitive_type<uint32_t>());
    DynamicData map_2_key(my_key);
    DynamicData map_3_key(str_type);
    DynamicData map_4_key(my_alias);
    DynamicData map_5_key(my_enum);
    DynamicData map_6_key(str_type);
    DynamicData map_6_inner_key(primitive_type<uint32_t>());

    DynamicData key_data(my_key);
    DynamicData enum_data(my_enum);
    // Set values
    map_1_key = uint32_t(55);
    data["map_1"][map_1_key] = "This is a simple map";
    map_1_key = uint32_t(99);
    data["map_1"][map_1_key] = "This is the same simple map";

    map_2_key["my_string"] = "String A";
    data["map_2"][map_2_key] = my_enum.value("AAA");
    map_2_key["my_string"] = "String B";
    data["map_2"][map_2_key] = my_enum.value("BBB");

    map_3_key = "KeyA";
    data["map_3"][map_3_key] = 'A';
    map_3_key = "KeyB";
    data["map_3"][map_3_key] = 'B';

    map_4_key = 'A';
    data["map_4"][map_4_key] = 1.1f;
    map_4_key = 'B';
    data["map_4"][map_4_key] = 2.2f;

    key_data["my_string"] = "AAA String";
    map_5_key = my_enum.value("AAA");
    data["map_5"][map_5_key] = key_data;
    key_data["my_string"] = "BBB String";
    map_5_key = my_enum.value("BBB");
    data["map_5"][map_5_key] = key_data;

    map_6_key = "OuterMapKey_1";
    map_6_inner_key = 550u;
    data["map_6"][map_6_key][map_6_inner_key] = "I'm a map of maps!";
    map_6_key = "OuterMapKey_2";
    map_6_inner_key = 666u;
    data["map_6"][map_6_key][map_6_inner_key] = "I'm a map of maps, but infernal!";

    // Check everything worked as expected.
    map_1_key = uint32_t(55);
    EXPECT_EQ(data["map_1"][map_1_key].value<std::string>(), "This is a simple map");
    map_1_key = uint32_t(99);
    EXPECT_EQ(data["map_1"][map_1_key].value<std::string>(), "This is the same simple map");

    map_2_key["my_string"] = "String A";
    EXPECT_EQ(data["map_2"][map_2_key].value<uint32_t>(), my_enum.value("AAA"));
    map_2_key["my_string"] = "String B";
    EXPECT_EQ(data["map_2"][map_2_key].value<uint32_t>(), my_enum.value("BBB"));

    map_3_key = "KeyA";
    EXPECT_EQ(data["map_3"][map_3_key].value<char>(), 'A');
    map_3_key = "KeyB";
    EXPECT_EQ(data["map_3"][map_3_key].value<char>(), 'B');

    map_4_key = 'A';
    EXPECT_EQ(data["map_4"][map_4_key].value<float>(), 1.1f);
    map_4_key = 'B';
    EXPECT_EQ(data["map_4"][map_4_key].value<float>(), 2.2f);

    key_data["my_string"] = "AAA String";
    map_5_key = my_enum.value("AAA");
    EXPECT_EQ(data["map_5"][map_5_key]["my_string"].value<std::string>(), "AAA String");
    key_data["my_string"] = "BBB String";
    map_5_key = my_enum.value("BBB");
    EXPECT_EQ(data["map_5"][map_5_key]["my_string"].value<std::string>(), "BBB String");

    map_6_key = "OuterMapKey_1";
    map_6_inner_key = 550u;
    EXPECT_EQ(data["map_6"][map_6_key][map_6_inner_key].value<std::string>(), "I'm a map of maps!");
    map_6_key = "OuterMapKey_2";
    map_6_inner_key = 666u;
    EXPECT_EQ(data["map_6"][map_6_key][map_6_inner_key].value<std::string>(), "I'm a map of maps, but infernal!");
}

TEST (IDLParser, struct_inheritance)
{
    Context context = parse(
        R"(
        struct ParentStruct
        {
            string my_str;
            uint32 my_uint;
        };

        struct ChildStruct : ParentStruct
        {
            string my_child_str;
        };
                   )");

    std::map<std::string, DynamicType::Ptr> result = context.get_all_types();
    EXPECT_EQ(2, result.size());

    const StructType* my_struct = static_cast<const StructType*>(result["ChildStruct"].get());

    ASSERT_TRUE(my_struct->has_parent());
    ASSERT_EQ(my_struct->parent().name(), "ParentStruct");

    DynamicData data(*my_struct);

    data["my_str"] = "I'm parent's string.";
    data["my_uint"] = 765u;
    data["my_child_str"] = "I'm child's string.";
    ASSERT_EQ(data["my_str"].value<std::string>(), "I'm parent's string.");
    ASSERT_EQ(data["my_uint"].value<uint32_t>(), 765);
    ASSERT_EQ(data["my_child_str"].value<std::string>(), "I'm child's string.");
}

TEST (IDLParser, empty_struct)
{
    Context context = parse(R"(
        struct EmptyStruct
        {
        };
                   )");

    std::map<std::string, DynamicType::Ptr> result = context.get_all_types();
    EXPECT_EQ(1, result.size());

    const StructType* my_struct = static_cast<const StructType*>(result["EmptyStruct"].get());
    ASSERT_NE(my_struct, nullptr);
    ASSERT_EQ(my_struct->members().size(), 0);
}

TEST (IDLParser, scoped_empty_struct)
{
    Context context = parse(
        R"(
        module a
        {
            module b
            {
                struct EmptyStruct
                {
                };
            };
        };
                   )");

    std::map<std::string, DynamicType::Ptr> result = context.get_all_scoped_types();
    EXPECT_EQ(1, result.size());

    const StructType* my_struct = static_cast<const StructType*>(result["a::b::EmptyStruct"].get());
    ASSERT_NE(my_struct, nullptr);
    ASSERT_EQ(my_struct->members().size(), 0);
}

TEST (IDLParser, same_struct_id_in_different_modules)
{
    DynamicType::Ptr first_struct;

    {
        Context context;
        parse(R"(
            module a
            {
                module b
                {
                    module c
                    {
                        struct MyStruct
                        {
                            long id;
                        };
                    };
                };
            };
                       )",
                context
                );
        std::map<std::string, DynamicType::Ptr> result = context.module().get_all_types();
        EXPECT_EQ(1, result.size());
        const DynamicType* my_struct = result["MyStruct"].get();
        EXPECT_EQ(my_struct->name(), "a::b::c::MyStruct");
        first_struct = result["MyStruct"];
        EXPECT_EQ(first_struct.get()->name(), "a::b::c::MyStruct");
    }

    {
        Context context;
        parse(R"(
            module x
            {
                module y
                {
                    struct MyStruct
                    {
                        long id;
                    };
                };
            };
                       )",
                context
                );
        std::map<std::string, DynamicType::Ptr> result = context.module().get_all_types();
        EXPECT_EQ(1, result.size());
        const DynamicType* my_struct = result["MyStruct"].get();
        EXPECT_EQ(my_struct->name(), "x::y::MyStruct");
    }

    EXPECT_EQ(first_struct.get()->name(), "a::b::c::MyStruct");
}

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
