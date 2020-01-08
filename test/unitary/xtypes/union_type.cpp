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
#include <iostream>

#include <cmath>
#include <bitset>

#include "../utils.hpp"

using namespace std;
using namespace eprosima::xtypes;

/**********************************
 *        UnionType Tests         *
 **********************************/

TEST (UnionType, primitive_union)
{
    UnionType un("bool_union_name", primitive_type<bool>());
    EXPECT_EQ("bool_union_name", un.name());
    EXPECT_EQ(TypeKind::UNION_TYPE, un.kind());

    size_t mem_size = 0;
    mem_size+=sizeof(bool); // the discriminator is bool

    un.add_case_member<bool>({true}, Member("bool", primitive_type<bool>()));
    mem_size+=sizeof(bool);
    EXPECT_EQ(mem_size, un.memory_size());

    un.add_case_member<bool>({false}, Member("uint8_t", primitive_type<uint8_t>()));
    mem_size+=sizeof(uint8_t);
    EXPECT_EQ(mem_size, un.memory_size());

    DynamicData union_data(un);
    union_data["bool"] = true;
    EXPECT_EQ(union_data["bool"].value<bool>(), true);
    union_data["bool"] = false;
    EXPECT_EQ(union_data["bool"].value<bool>(), false);

    union_data["uint8_t"] = uint8_t(55);
    EXPECT_EQ(union_data["uint8_t"].value<uint8_t>(), 55);

    // Discriminator
    union_data["discriminator"] = true;
    EXPECT_EQ(union_data["discriminator"].value<bool>(), true);
    union_data["discriminator"] = false;
    EXPECT_EQ(union_data["discriminator"].value<bool>(), false);
    union_data.discriminator() = true;
    EXPECT_EQ(union_data["discriminator"].value<bool>(), true);
}
