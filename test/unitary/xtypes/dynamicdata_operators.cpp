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
#include <type_traits>

using namespace eprosima::xtypes;

#define PI 3.14159f
#define PI_D 3.14159
#define PI_L 3.14159L

/***********************************************
 *         DynamicData Operators Tests         *
 **********************************************/
#define ASSERT_EQ_DYNAMICDATA(DYNAMICDATA, VAL) ASSERT_EQ((DYNAMICDATA).value<T>(), VAL)

template <typename T>
inline void check_de_increment_operators(
        const T& value,
        bool assert_fail=false)
{
    const DynamicType& type(primitive_type<T>());
    DynamicData data(type);
    data = static_cast<T>(value);

    if (assert_fail)
    {
        ASSERT_OR_EXCEPTION({ ASSERT_EQ_DYNAMICDATA(data++, value + 1); }, R"xtypes(Operator\+\+\(\))xtypes");
        ASSERT_OR_EXCEPTION({ ASSERT_EQ_DYNAMICDATA(data--, value + 1); }, R"xtypes(Operator--\(\))xtypes");
    }
    else
    {
        ASSERT_EQ_DYNAMICDATA(data++, static_cast<T>(value + 1));
        ASSERT_EQ_DYNAMICDATA(data--, value);
        DynamicData other(type);
        other = ++data;
        ASSERT_EQ_DYNAMICDATA(other, static_cast<T>(value + 1));
        other = --data;
        ASSERT_EQ_DYNAMICDATA(other, value);
    }
}

TEST (DynamicDataOperators, increment_decrement_operators)
{
    // Operators ++/-- available
    check_de_increment_operators<int8_t>(3);
    check_de_increment_operators<uint8_t>(7);
    check_de_increment_operators<int16_t>(-10);
    check_de_increment_operators<uint16_t>(10u);
    check_de_increment_operators<int32_t>(5);
    check_de_increment_operators<uint32_t>(20u);
    check_de_increment_operators<int64_t>(-6);
    check_de_increment_operators<uint64_t>(60u);
    // Operators ++/-- unavailable
    check_de_increment_operators<bool>(true, true);
    check_de_increment_operators<char>('a', true);
    check_de_increment_operators<wchar_t>('\n', true);
    check_de_increment_operators<char16_t>(u'\u00f1', true);
    check_de_increment_operators<float>(PI, true);
    check_de_increment_operators<double>(PI_D, true);
    check_de_increment_operators<long double>(PI_L, true);
}

#define GET_MACRO4(_1, _2, _3, _4, NAME, ...) NAME
#define CHECK_ARITHMETIC_UNARY_OPERATOR(...) \
    GET_MACRO4(__VA_ARGS__, CHECK_ARITHMETIC_UNARY_OPERATOR_COMPARE, \
    CHECK_ARITHMETIC_UNARY_OPERATOR_EXECUTE_OP)(__VA_ARGS__)

#define CHECK_ARITHMETIC_UNARY_OPERATOR_EXECUTE_OP(value, assert_fail, OP) \
{\
    const DynamicType& type(primitive_type<T>());\
    DynamicData data(type);\
    data = static_cast<T>(value);\
\
    if (assert_fail)\
    {\
        ASSERT_OR_EXCEPTION({ ASSERT_EQ_DYNAMICDATA(OP(data), OP(value)); },\
            std::string("Operator") + #OP + "\\(\\)");\
    }\
    else\
    {\
        ASSERT_EQ_DYNAMICDATA(OP(data), OP(value));\
    }\
}

#define CHECK_ARITHMETIC_UNARY_OPERATOR_COMPARE(value, assert_fail, OP, result) \
{\
    const DynamicType& type(primitive_type<T>());\
    DynamicData data(type);\
    data = static_cast<T>(value);\
\
    if (assert_fail)\
    {\
        ASSERT_OR_EXCEPTION({ ASSERT_EQ_DYNAMICDATA(OP(data), (result)); },\
            std::string("Operator") + #OP + "\\(\\)");\
    }\
    else\
    {\
        ASSERT_EQ_DYNAMICDATA(OP(data), (result));\
    }\
}

template <typename T>
inline void check_bitwise_complement_operator(
        const T& value,
        bool assert_fail=false)
{
    CHECK_ARITHMETIC_UNARY_OPERATOR(value, assert_fail, ~);
}

// in order to avoid warning C4804: ~value == !value
template<>
inline void check_bitwise_complement_operator(
        const bool& value,
        bool assert_fail)
{
    using T = bool;
    CHECK_ARITHMETIC_UNARY_OPERATOR(value, assert_fail, ~, !value);
}

template<typename T>
inline void check_negate_operator(
        const T& value,
        bool assert_fail=false)
{
    CHECK_ARITHMETIC_UNARY_OPERATOR(value, assert_fail, -);
}

// in order to avoid warning C4804: -value == !value
template<>
inline void check_negate_operator(
        const bool& value,
        bool assert_fail)
{
    using T = bool;
    CHECK_ARITHMETIC_UNARY_OPERATOR(value, assert_fail, -, !value);
}

TEST (DynamicDataOperators, arithmetic_unary_operators)
{
    // Operator ~ available
    check_bitwise_complement_operator<int8_t>(3);
    check_bitwise_complement_operator<int16_t>(-10);
    check_bitwise_complement_operator<int32_t>(5);
    check_bitwise_complement_operator<int64_t>(-6);
    // Operator ~ unavailable
    check_bitwise_complement_operator<bool>(true, true);
    check_bitwise_complement_operator<char>('a', true);
    check_bitwise_complement_operator<wchar_t>('\n', true);
    check_bitwise_complement_operator<char16_t>(u'\u00f1', true);
    check_bitwise_complement_operator<uint8_t>(7u, true);
    check_bitwise_complement_operator<uint16_t>(10u, true);
    check_bitwise_complement_operator<uint32_t>(20u, true);
    check_bitwise_complement_operator<uint64_t>(60u, true);

    // Operator - available
    check_negate_operator<int8_t>(3);
    check_negate_operator<int16_t>(-10);
    check_negate_operator<int32_t>(5);
    check_negate_operator<int64_t>(-6);
    check_negate_operator<float>(PI);
    check_negate_operator<double>(PI_D);
    check_negate_operator<long double>(PI_L);
    // Operator - unavailable
    check_negate_operator<bool>(true, true);
    check_negate_operator<char>('a', true);
    check_negate_operator<wchar_t>('\n', true);
    check_negate_operator<char16_t>(u'\u00f1', true);
    check_negate_operator<uint8_t>(7u, true);
    check_negate_operator<uint16_t>(10u, true);
    check_negate_operator<uint32_t>(20u, true);
    check_negate_operator<uint64_t>(60u, true);
}

#define ASSERT_EQ_DYNAMICDATA_ARITHMETIC_OP(OPERAND1, OPERATOR, OPERAND2, RES) \
{\
    res = OPERAND1 OPERATOR OPERAND2;\
    ASSERT_EQ_DYNAMICDATA(res, RES);\
}

template <typename T>
inline void check_arithmetic_flt_binary_operators(
        const T& A,
        const T& B)
{
    const DynamicType& type(primitive_type<T>());
    DynamicData data(type), _data(type), res(type);
    data = A;
    _data = B;
    ASSERT_EQ_DYNAMICDATA_ARITHMETIC_OP(data, +, _data, A + B);
    ASSERT_EQ_DYNAMICDATA_ARITHMETIC_OP(data, -, _data, A - B);
    ASSERT_EQ_DYNAMICDATA_ARITHMETIC_OP(data, *, _data, A * B);
    ASSERT_EQ_DYNAMICDATA_ARITHMETIC_OP(data, /, _data, A / B);
}

// in order to avoid warning C4804: handle arithmetica as boolean algebra
template<>
inline void check_arithmetic_flt_binary_operators(
        const bool& A,
        const bool& B)
{
    const DynamicType& type(primitive_type<bool>());
    DynamicData data(type), _data(type), res(type);
    data = A;
    _data = B;

    using T = bool;
    ASSERT_EQ_DYNAMICDATA_ARITHMETIC_OP(data, +, _data, A || B);
    ASSERT_EQ_DYNAMICDATA_ARITHMETIC_OP(data, *, _data, A && B);
}

#define ASSERT_EQ_DYNAMICDATA_ARITHMETIC_OP_EXCEPT(OPERAND1, OPERATOR, OPERAND2, RES) \
{\
    std::stringstream errmsg;\
    char op = #OPERATOR[0];\
    errmsg << "Operator";\
    if(op == '^' || op == '|')\
    {\
        errmsg << "\\";\
    }\
    errmsg << op;\
    ASSERT_OR_EXCEPTION(\
        { ASSERT_EQ_DYNAMICDATA_ARITHMETIC_OP(OPERAND1, OPERATOR, OPERAND2, RES);},\
        errmsg.str());\
}

template <typename T>
inline void check_arithmetic_int_binary_operators(
        const T& A,
        const T& B,
        bool assert_fail=false)
{
    const DynamicType& type(primitive_type<T>());
    DynamicData data(type), _data(type), res(type);
    data = A;
    _data = B;
    if (assert_fail)
    {
        ASSERT_EQ_DYNAMICDATA_ARITHMETIC_OP_EXCEPT(data, +,  _data, A +  B);
        ASSERT_EQ_DYNAMICDATA_ARITHMETIC_OP_EXCEPT(data, -,  _data, A -  B);
        ASSERT_EQ_DYNAMICDATA_ARITHMETIC_OP_EXCEPT(data, *,  _data, A *  B);
        ASSERT_EQ_DYNAMICDATA_ARITHMETIC_OP_EXCEPT(data, /,  _data, A /  B);
        ASSERT_EQ_DYNAMICDATA_ARITHMETIC_OP_EXCEPT(data, %,  _data, A %  B);
        ASSERT_EQ_DYNAMICDATA_ARITHMETIC_OP_EXCEPT(data, <<, _data, A << B);
        ASSERT_EQ_DYNAMICDATA_ARITHMETIC_OP_EXCEPT(data, >>, _data, A >> B);
        ASSERT_EQ_DYNAMICDATA_ARITHMETIC_OP_EXCEPT(data, &,  _data, A &  B);
        ASSERT_EQ_DYNAMICDATA_ARITHMETIC_OP_EXCEPT(data, ^,  _data, A ^  B);
        ASSERT_EQ_DYNAMICDATA_ARITHMETIC_OP_EXCEPT(data, |,  _data, A |  B);
    }
    else
    {
        ASSERT_EQ_DYNAMICDATA_ARITHMETIC_OP(data, +,  _data, A +  B);
        ASSERT_EQ_DYNAMICDATA_ARITHMETIC_OP(data, -,  _data, A -  B);
        ASSERT_EQ_DYNAMICDATA_ARITHMETIC_OP(data, *,  _data, A *  B);
        ASSERT_EQ_DYNAMICDATA_ARITHMETIC_OP(data, /,  _data, A /  B);
        ASSERT_EQ_DYNAMICDATA_ARITHMETIC_OP(data, %,  _data, A %  B);
        ASSERT_EQ_DYNAMICDATA_ARITHMETIC_OP(data, <<, _data, A << B);
        ASSERT_EQ_DYNAMICDATA_ARITHMETIC_OP(data, >>, _data, A >> B);
        ASSERT_EQ_DYNAMICDATA_ARITHMETIC_OP(data, &,  _data, A &  B);
        ASSERT_EQ_DYNAMICDATA_ARITHMETIC_OP(data, ^,  _data, A ^  B);
        ASSERT_EQ_DYNAMICDATA_ARITHMETIC_OP(data, |,  _data, A |  B);
    }
}

// in order to avoid warning C4804: handle arithmetica as boolean algebra
template <>
inline void check_arithmetic_int_binary_operators(
        const bool& A,
        const bool& B,
        bool assert_fail)
{
    const DynamicType& type(primitive_type<bool>());
    DynamicData data(type), _data(type), res(type);
    data = A;
    _data = B;

    using T = bool;
    if (assert_fail)
    {
        ASSERT_EQ_DYNAMICDATA_ARITHMETIC_OP_EXCEPT(data, +,  _data, A || B);
        ASSERT_EQ_DYNAMICDATA_ARITHMETIC_OP_EXCEPT(data, *,  _data, A && B);
        ASSERT_EQ_DYNAMICDATA_ARITHMETIC_OP_EXCEPT(data, <<, _data, A);
        ASSERT_EQ_DYNAMICDATA_ARITHMETIC_OP_EXCEPT(data, >>, _data, A && !B);
        ASSERT_EQ_DYNAMICDATA_ARITHMETIC_OP_EXCEPT(data, &,  _data, A & B);
        ASSERT_EQ_DYNAMICDATA_ARITHMETIC_OP_EXCEPT(data, ^,  _data, A ^ B);
        ASSERT_EQ_DYNAMICDATA_ARITHMETIC_OP_EXCEPT(data, |,  _data, A | B);
    }
    else
    {
        ASSERT_EQ_DYNAMICDATA_ARITHMETIC_OP(data, +,  _data, A || B);
        ASSERT_EQ_DYNAMICDATA_ARITHMETIC_OP(data, *,  _data, A && B);
        ASSERT_EQ_DYNAMICDATA_ARITHMETIC_OP(data, <<, _data, A);
        ASSERT_EQ_DYNAMICDATA_ARITHMETIC_OP(data, >>, _data, A && !B);
        ASSERT_EQ_DYNAMICDATA_ARITHMETIC_OP(data, &,  _data, A & B);
        ASSERT_EQ_DYNAMICDATA_ARITHMETIC_OP(data, ^,  _data, A ^ B);
        ASSERT_EQ_DYNAMICDATA_ARITHMETIC_OP(data, |,  _data, A | B);
    }
}

TEST (DynamicDataOperators, arithmetic_binary_operators)
{
    // Operators available (all)
    check_arithmetic_int_binary_operators<int8_t>(3, 2);
    check_arithmetic_int_binary_operators<uint8_t>(7u, 2u);
    check_arithmetic_int_binary_operators<int16_t>(-10, 2);
    check_arithmetic_int_binary_operators<uint16_t>(10u, 2u);
    check_arithmetic_int_binary_operators<int32_t>(5, 2);
    check_arithmetic_int_binary_operators<uint32_t>(20u, 2u);
    check_arithmetic_int_binary_operators<int64_t>(-6, -2);
    check_arithmetic_int_binary_operators<uint64_t>(60u, 2u);

#ifdef XTYPES_EXCEPTIONS
    // Operators not available
    check_arithmetic_int_binary_operators<bool>(true, false, true);
    check_arithmetic_int_binary_operators<char>('a', 'b', true);
    check_arithmetic_int_binary_operators<wchar_t>('\n', 'b', true);
    check_arithmetic_int_binary_operators<char16_t>(u'\u00f1', 'b', true);
#endif

    // Operators available (only floating-point)
    check_arithmetic_flt_binary_operators<float>(PI, 1.5f);
    check_arithmetic_flt_binary_operators<double>(PI_D, 1.5);
    check_arithmetic_flt_binary_operators<long double>(PI_L, 1.5L);
}

template <typename T>
inline void check_logical_not_operator(
        const T& value,
        bool assert_fail=false)
{
    DynamicData data(primitive_type<T>());
    data = value;
    if (assert_fail)
    {
        ASSERT_OR_EXCEPTION({ ASSERT_EQ(!data, !value); }, R"xtypes(Operator!\(\))xtypes");
    }
    else
    {
        ASSERT_EQ(!data, !value);
    }
}

template <typename T>
inline void check_logical_binary_operator(
        const T& A,
        const T& B)
{
    const DynamicType& type(primitive_type<T>());
    DynamicData data(type), _data(type);
    data = A;
    _data = B;

    ASSERT_EQ(data && _data, A && B);
    ASSERT_EQ(data || _data, A || B);
}

TEST (DynamicDataOperators, logical_operators)
{
    // Unary operator !() available
    check_logical_not_operator<bool>(true);
    check_logical_not_operator<char>('a');
    check_logical_not_operator<wchar_t>('\0');
    check_logical_not_operator<char16_t>(u'\u00f1');
    check_logical_not_operator<int8_t>(1);
    check_logical_not_operator<uint8_t>(1u);
    check_logical_not_operator<int16_t>(0);
    check_logical_not_operator<uint16_t>(0u);
    check_logical_not_operator<int32_t>(-6);
    check_logical_not_operator<uint32_t>(3u);
    check_logical_not_operator<int64_t>(0);
    check_logical_not_operator<uint64_t>(3000u);
    // Special operator !() consideration for (W)StringType
    {
        StringType str(20);
        DynamicData data(str);
        data = "some text";
        ASSERT_EQ(!data, false);
        WStringType wstr(20);
        DynamicData wdata(wstr);
        wdata = L"";
        ASSERT_EQ(!wdata, true);
    }
    // Unary operator (!) not available
    check_logical_not_operator<float>(PI, true);
    check_logical_not_operator<double>(PI_D, true);
    check_logical_not_operator<long double>(PI_L, true);
    // Binary operators: &&, ||
    check_logical_binary_operator<bool>(true, false);
    check_logical_binary_operator<char>('a', 'b');
    check_logical_binary_operator<wchar_t>('\n', '\0');
    check_logical_binary_operator<char16_t>(u'\u00f1', '\0');
    check_logical_binary_operator<int8_t>(1, -2);
    check_logical_binary_operator<uint8_t>(1u, 2u);
    check_logical_binary_operator<int16_t>(4, 23);
    check_logical_binary_operator<uint16_t>(0u, 1u);
    check_logical_binary_operator<int32_t>(-6, -7);
    check_logical_binary_operator<uint32_t>(2u, 3u);
    check_logical_binary_operator<int64_t>(0, 0);
    check_logical_binary_operator<uint64_t>(3000u, 239u);
    check_logical_binary_operator<float>(PI, 0.1f);
    check_logical_binary_operator<double>(PI_D, 0.1);
    check_logical_binary_operator<long double>(PI_L, 0.1L);
}

template <typename T>
inline void check_comparison_binary_operator(
        const T& A,
        const T& B)
{
    const DynamicType& type(primitive_type<T>());
    DynamicData data(type), _data(type);
    data = A;
    _data = B;

    ASSERT_EQ(data == _data, A == B);
    ASSERT_EQ(data != _data, A != B);
    ASSERT_EQ(data <  _data, A <  B);
    ASSERT_EQ(data >  _data, A >  B);
    ASSERT_EQ(data <= _data, A <= B);
    ASSERT_EQ(data >= _data, A >= B);
}

TEST (DynamicDataOperators, comparison_operators)
{
    check_comparison_binary_operator<bool>(true, false);
    check_comparison_binary_operator<char>('a', 'b');
    check_comparison_binary_operator<wchar_t>('\n', '\0');
    check_comparison_binary_operator<char16_t>(u'\u00f1', '\0');
    check_comparison_binary_operator<int8_t>(1, -2);
    check_comparison_binary_operator<uint8_t>(1u, 2u);
    check_comparison_binary_operator<int16_t>(4, 23);
    check_comparison_binary_operator<uint16_t>(0, 1);
    check_comparison_binary_operator<int32_t>(-6, -7);
    check_comparison_binary_operator<uint32_t>(2, 3);
    check_comparison_binary_operator<int64_t>(0, 0);
    check_comparison_binary_operator<uint64_t>(3000, 239);
    check_comparison_binary_operator<float>(PI, 0.1f);
    check_comparison_binary_operator<double>(PI_D, 0.1);
    check_comparison_binary_operator<long double>(PI_L, 0.1L);
}

#define ASSERT_EQ_DYNAMICDATA_SELFASSIGN_OP(OPERAND1, OPERATOR, OPERAND2, RES) \
{\
    OPERAND1 = A;\
    OPERAND1 OPERATOR OPERAND2;\
    ASSERT_EQ_DYNAMICDATA(OPERAND1, RES);\
}

template <typename T>
inline void check_assignment_flt_operators(
        const T& A,
        const T& B)
{
    const DynamicType& type(primitive_type<T>());
    DynamicData data(type), _data(type);

    data = A;
    _data = B;

    ASSERT_EQ_DYNAMICDATA_SELFASSIGN_OP(data, +=, _data, A + B);
    ASSERT_EQ_DYNAMICDATA_SELFASSIGN_OP(data, -=, _data, A - B);
    ASSERT_EQ_DYNAMICDATA_SELFASSIGN_OP(data, *=, _data, A * B);
    ASSERT_EQ_DYNAMICDATA_SELFASSIGN_OP(data, /=, _data, A / B);

    ASSERT_EQ_DYNAMICDATA_SELFASSIGN_OP(data, +=, B, A + B);
    ASSERT_EQ_DYNAMICDATA_SELFASSIGN_OP(data, -=, B, A - B);
    ASSERT_EQ_DYNAMICDATA_SELFASSIGN_OP(data, *=, B, A * B);
    ASSERT_EQ_DYNAMICDATA_SELFASSIGN_OP(data, /=, B, A / B);
}

#define ASSERT_EQ_DYNAMICDATA_SELFASSIGN_OP_EXCEPT(OPERAND1, OPERATOR, OPERAND2, RES) \
{\
    ASSERT_OR_EXCEPTION(\
        { ASSERT_EQ_DYNAMICDATA_SELFASSIGN_OP(OPERAND1, OPERATOR, OPERAND2, RES);},\
        R"xtypes(Operator.*\(\) cannot be used with non-arithmetic types)xtypes");\
}

template <typename T>
inline void check_assignment_operators(
        const T& A,
        const T& B,
        bool assert_fail=false)
{
    const DynamicType& type(primitive_type<T>());
    DynamicData data(type), _data(type), res(type);
    data = A;
    _data = B;
    if (assert_fail)
    {
        ASSERT_EQ_DYNAMICDATA_SELFASSIGN_OP_EXCEPT(data, +=,  B, A +  B);
        ASSERT_EQ_DYNAMICDATA_SELFASSIGN_OP_EXCEPT(data, -=,  B, A -  B);
        ASSERT_EQ_DYNAMICDATA_SELFASSIGN_OP_EXCEPT(data, *=,  B, A *  B);
        ASSERT_EQ_DYNAMICDATA_SELFASSIGN_OP_EXCEPT(data, /=,  B, A /  B);
        ASSERT_EQ_DYNAMICDATA_SELFASSIGN_OP_EXCEPT(data, %=,  B, A %  B);
        ASSERT_EQ_DYNAMICDATA_SELFASSIGN_OP_EXCEPT(data, <<=, B, A << B);
        ASSERT_EQ_DYNAMICDATA_SELFASSIGN_OP_EXCEPT(data, >>=, B, A >> B);
        ASSERT_EQ_DYNAMICDATA_SELFASSIGN_OP_EXCEPT(data, &=,  B, A &  B);
        ASSERT_EQ_DYNAMICDATA_SELFASSIGN_OP_EXCEPT(data, ^=,  B, A ^  B);
        ASSERT_EQ_DYNAMICDATA_SELFASSIGN_OP_EXCEPT(data, |=,  B, A |  B);
    }
    else
    {
        ASSERT_EQ_DYNAMICDATA_SELFASSIGN_OP(data, +=,  _data, A +  B);
        ASSERT_EQ_DYNAMICDATA_SELFASSIGN_OP(data, -=,  _data, A -  B);
        ASSERT_EQ_DYNAMICDATA_SELFASSIGN_OP(data, *=,  _data, A *  B);
        ASSERT_EQ_DYNAMICDATA_SELFASSIGN_OP(data, /=,  _data, A /  B);
        ASSERT_EQ_DYNAMICDATA_SELFASSIGN_OP(data, %=,  _data, A %  B);
        ASSERT_EQ_DYNAMICDATA_SELFASSIGN_OP(data, <<=, _data, A << B);
        ASSERT_EQ_DYNAMICDATA_SELFASSIGN_OP(data, >>=, _data, A >> B);
        ASSERT_EQ_DYNAMICDATA_SELFASSIGN_OP(data, &=,  _data, A &  B);
        ASSERT_EQ_DYNAMICDATA_SELFASSIGN_OP(data, ^=,  _data, A ^  B);
        ASSERT_EQ_DYNAMICDATA_SELFASSIGN_OP(data, |=,  _data, A |  B);

        ASSERT_EQ_DYNAMICDATA_SELFASSIGN_OP(data, +=,  B, A +  B);
        ASSERT_EQ_DYNAMICDATA_SELFASSIGN_OP(data, -=,  B, A -  B);
        ASSERT_EQ_DYNAMICDATA_SELFASSIGN_OP(data, *=,  B, A *  B);
        ASSERT_EQ_DYNAMICDATA_SELFASSIGN_OP(data, /=,  B, A /  B);
        ASSERT_EQ_DYNAMICDATA_SELFASSIGN_OP(data, %=,  B, A %  B);
        ASSERT_EQ_DYNAMICDATA_SELFASSIGN_OP(data, <<=, B, A << B);
        ASSERT_EQ_DYNAMICDATA_SELFASSIGN_OP(data, >>=, B, A >> B);
        ASSERT_EQ_DYNAMICDATA_SELFASSIGN_OP(data, &=,  B, A &  B);
        ASSERT_EQ_DYNAMICDATA_SELFASSIGN_OP(data, ^=,  B, A ^  B);
        ASSERT_EQ_DYNAMICDATA_SELFASSIGN_OP(data, |=,  B, A |  B);
    }
}

// in order to avoid warning C4804: handle arithmetica as boolean algebra
template <>
inline void check_assignment_operators(
        const bool& A,
        const bool& B,
        bool assert_fail)
{
    const DynamicType& type(primitive_type<bool>());
    DynamicData data(type), _data(type), res(type);
    data = A;
    _data = B;

    using T = bool;
    if (assert_fail)
    {
        ASSERT_EQ_DYNAMICDATA_SELFASSIGN_OP_EXCEPT(data, +=,  B, A || B);
        ASSERT_EQ_DYNAMICDATA_SELFASSIGN_OP_EXCEPT(data, *=,  B, A && B);
        ASSERT_EQ_DYNAMICDATA_SELFASSIGN_OP_EXCEPT(data, &=,  B, A &  B);
        ASSERT_EQ_DYNAMICDATA_SELFASSIGN_OP_EXCEPT(data, ^=,  B, A ^  B);
        ASSERT_EQ_DYNAMICDATA_SELFASSIGN_OP_EXCEPT(data, |=,  B, A |  B);
    }
    else
    {
        ASSERT_EQ_DYNAMICDATA_SELFASSIGN_OP(data, +=,  _data, A || B);
        ASSERT_EQ_DYNAMICDATA_SELFASSIGN_OP(data, *=,  _data, A && B);
        ASSERT_EQ_DYNAMICDATA_SELFASSIGN_OP(data, &=,  _data, A &  B);
        ASSERT_EQ_DYNAMICDATA_SELFASSIGN_OP(data, ^=,  _data, A ^  B);
        ASSERT_EQ_DYNAMICDATA_SELFASSIGN_OP(data, |=,  _data, A |  B);

        ASSERT_EQ_DYNAMICDATA_SELFASSIGN_OP(data, +=,  B, A || B);
        ASSERT_EQ_DYNAMICDATA_SELFASSIGN_OP(data, *=,  B, A && B);
        ASSERT_EQ_DYNAMICDATA_SELFASSIGN_OP(data, &=,  B, A &  B);
        ASSERT_EQ_DYNAMICDATA_SELFASSIGN_OP(data, ^=,  B, A ^  B);
        ASSERT_EQ_DYNAMICDATA_SELFASSIGN_OP(data, |=,  B, A |  B);
    }
}

TEST (DynamicDataOperators, assignment_operators)
{
    // Operators available (all)
    check_assignment_operators<int8_t>(3, 2);
    check_assignment_operators<uint8_t>(7u, 2u);
    check_assignment_operators<int16_t>(-10, 2);
    check_assignment_operators<uint16_t>(10u, 2u);
    check_assignment_operators<int32_t>(5, 2);
    check_assignment_operators<uint32_t>(20u, 2u);
    check_assignment_operators<int64_t>(-6, -2);
    check_assignment_operators<uint64_t>(60u, 2u);
    // Operators not available
    check_assignment_operators<bool>(true, false, true);
    check_assignment_operators<char>('a', 'b', true);
    check_assignment_operators<wchar_t>('\n', 'b', true);
    check_assignment_operators<char16_t>(u'\u00f1', u'b', true);
    /*
    DynamicData a(primitive_type<char16_t>());
    DynamicData b(primitive_type<char16_t>());
    DynamicData c(primitive_type<char16_t>());
    a = u'A';
    b = u'B';
    c = a + b;
    a += b;
    ASSERT_EQ(a.value<char16_t>(), c.value<char16_t>());
    */

    // Operators available (only floating-point)
    check_assignment_flt_operators<float>(PI, 1.5f);
    check_assignment_flt_operators<double>(PI_D, 1.5);
    check_assignment_flt_operators<long double>(PI_L, 1.5L);
}
