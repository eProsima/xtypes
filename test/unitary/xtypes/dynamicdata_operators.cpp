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
#include "../utils.hpp"

using namespace eprosima::xtypes;

#define PI 3.14159f

/***********************************************
 *         DynamicData Operators Tests         *
 **********************************************/
#define ASSERT_EQ_DYNAMICDATA(DYNAMICDATA, TYPE, VAL) ASSERT_EQ(DYNAMICDATA.value<TYPE>(), VAL)

#define ASSERT_EQ_DYNAMICDATA_INT32(DYNAMICDATA, VAL) ASSERT_EQ_DYNAMICDATA(DYNAMICDATA, int32_t, VAL)

TEST (DynamicDataOperators, increment_decrement_operators)
{
    DynamicData data(primitive_type<int32_t>());
    data.value(4);
    DynamicData& dataref = ++data;
    ASSERT_EQ_DYNAMICDATA_INT32(dataref, 5);
    ASSERT_EQ_DYNAMICDATA_INT32(data--, 4);
}

#define ASSERT_EQ_DYNAMICDATA_ARITHMETIC_OPERATION(OPERATOR, VALUE) \
{\
    res = data OPERATOR _data;\
    ASSERT_EQ(res.value<int32_t>(), VALUE);\
}

TEST (DynamicDataOperators, arithmetic_operators)
{
    DynamicData data(primitive_type<int32_t>());
    DynamicData _data(primitive_type<int32_t>());
    DynamicData res(primitive_type<int32_t>());
    data.value(4);
    _data.value(-2);
    ASSERT_EQ_DYNAMICDATA_INT32(-data, -4);
    ASSERT_EQ_DYNAMICDATA_INT32(~data, -5);
    ASSERT_EQ_DYNAMICDATA_INT32(!data, false);
    ASSERT_EQ_DYNAMICDATA_ARITHMETIC_OPERATION(+, 2);
    ASSERT_EQ_DYNAMICDATA_ARITHMETIC_OPERATION(-, 6);
    ASSERT_EQ_DYNAMICDATA_ARITHMETIC_OPERATION(*, -8);
    ASSERT_EQ_DYNAMICDATA_ARITHMETIC_OPERATION(/, -2);
    _data.value(3);
    ASSERT_EQ_DYNAMICDATA_ARITHMETIC_OPERATION(%, 1);
    _data.value(1);
    ASSERT_EQ_DYNAMICDATA_ARITHMETIC_OPERATION(<<, 8);
    ASSERT_EQ_DYNAMICDATA_ARITHMETIC_OPERATION(>>, 2);
    data.value(6);
    _data.value(5);
    ASSERT_EQ_DYNAMICDATA_ARITHMETIC_OPERATION(&, 4);
    ASSERT_EQ_DYNAMICDATA_ARITHMETIC_OPERATION(|, 7);
}

TEST (DynamicDataOperators, logical_operators)
{
    DynamicData A(primitive_type<bool>());
    DynamicData B(primitive_type<bool>());
    A = true;
    B = true;
    ASSERT_EQ(A && B, true);
    A = false;
    ASSERT_EQ(A && B, false);
    ASSERT_EQ(A || B, true);
    StringType str(20);
    DynamicData fail(str);
    fail = "This will fail";
    DynamicData _fail(str);
    _fail = "This will fail too";
    ASSERT_OR_EXCEPTION({ fail && _fail; }, "Operator&&()");
}

TEST (DynamicDataOperators, comparison_operators)
{
    DynamicData data(primitive_type<uint32_t>());
    DynamicData _data(primitive_type<uint32_t>());
    data = 23u;
    _data = 23u;
    ASSERT_EQ(data <= _data, true);
    ASSERT_EQ(data < _data, false);
    _data--;
    ASSERT_EQ(data <= _data, false);
    ASSERT_EQ(data > _data, true);
    ASSERT_EQ(data >= data, true);
}

#define ASSERT_EQ_DYNAMICDATA_FLOAT32(DYNAMICDATA, VAL) ASSERT_EQ_DYNAMICDATA(DYNAMICDATA, float, VAL)

TEST (DynamicDataOperators, assignment_operators)
{
    DynamicData data(primitive_type<float>());
    DynamicData _data(primitive_type<float>());
    _data = PI;
    data = _data;
    data += static_cast<float>(2); 
    ASSERT_EQ_DYNAMICDATA_FLOAT32(data, PI + 2);
    _data -= 0.14159f;
    ASSERT_EQ_DYNAMICDATA_FLOAT32(_data, static_cast<float>(3));
    data *= _data;
    ASSERT_EQ_DYNAMICDATA_FLOAT32(data, 3*(PI + 2));
    ASSERT_OR_EXCEPTION({ data <<= _data; }, "Operator<<()");
}
