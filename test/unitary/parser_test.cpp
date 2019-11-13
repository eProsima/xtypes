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
#include <xtypes/idl/IDLParser.hpp>
#include <iostream>

using namespace eprosima::xtypes;
using namespace eprosima::xtypes::idl;


TEST (IDLParser, simple_struct_test)
{
    std::map<std::string, std::shared_ptr<DynamicType>> result;
    result = parse(R"(
        struct SimpleStruct
        {
            string my_string;
            float my_float;
        };
                   )");
    EXPECT_EQ(1, result.size());
}

/*
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
*/

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
