/*
 * Copyright 2019, Proyectos y Sistemas de Mantenimiento SL (eProsima).
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef EPROSIMA_XTYPES_DYNAMIC_DATA_IMPL_HPP_
#define EPROSIMA_XTYPES_DYNAMIC_DATA_IMPL_HPP_

#include <xtypes/DynamicData.hpp>

#include <sstream>
#include <locale>
#include <codecvt>

namespace eprosima {
namespace xtypes {


inline std::string ReadableDynamicDataRef::to_string() const
{
    std::stringstream ss;
    for_each([&](const DynamicData::ReadableNode& node)
    {
        const std::string& type_name = node.data().type().name();
        ss << std::string(node.deep() * 4, ' ');
        if(node.has_parent())
        {
            ss << "["
                << (node.parent().type().is_aggregation_type()
                    ? node.from_member()->name()
                    : std::to_string(node.from_index()))
                << "] ";
        }
        switch(node.data().type().kind())
        {
            case TypeKind::BOOLEAN_TYPE:
                ss << "<" << type_name << ">  " << node.data().value<bool>();
                break;
            case TypeKind::CHAR_8_TYPE:
                ss << "<" << type_name << ">  " << node.data().value<char>();
                break;
            case TypeKind::CHAR_16_TYPE:
                ss << "<" << type_name << ">  " << node.data().value<char32_t>();
                break;
            case TypeKind::INT_8_TYPE:
                ss << "<" << type_name << ">  " << node.data().value<int8_t>();
                break;
            case TypeKind::UINT_8_TYPE:
                ss << "<" << type_name << ">  " << node.data().value<uint8_t>();
                break;
            case TypeKind::INT_16_TYPE:
                ss << "<" << type_name << ">  " << node.data().value<int16_t>();
                break;
            case TypeKind::UINT_16_TYPE:
                ss << "<" << type_name << ">  " << node.data().value<uint16_t>();
                break;
            case TypeKind::INT_32_TYPE:
                ss << "<" << type_name << ">  " << node.data().value<int32_t>();
                break;
            case TypeKind::UINT_32_TYPE:
                ss << "<" << type_name << ">  " << node.data().value<uint32_t>();
                break;
            case TypeKind::INT_64_TYPE:
                ss << "<" << type_name << ">  " << node.data().value<int64_t>();
                break;
            case TypeKind::UINT_64_TYPE:
                ss << "<" << type_name << ">  " << node.data().value<uint64_t>();
                break;
            case TypeKind::FLOAT_32_TYPE:
                ss << "<" << type_name << ">  " << node.data().value<float>();
                break;
            case TypeKind::FLOAT_64_TYPE:
                ss << "<" << type_name << ">  " << node.data().value<double>();
                break;
            case TypeKind::FLOAT_128_TYPE:
                ss << "<" << type_name << ">  " << node.data().value<long double>();
                break;
            case TypeKind::STRING_TYPE:
                ss << "<" << type_name << ">  " << node.data().value<std::string>();
                break;
            case TypeKind::WSTRING_TYPE:
            {
                std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
                ss << "<" << type_name << ">  " << converter.to_bytes(node.data().value<std::wstring>());
                break;
            }
            case TypeKind::ARRAY_TYPE:
                ss << "<" << type_name << ">";
                break;
            case TypeKind::SEQUENCE_TYPE:
                ss << "<" << type_name << "[" << node.data().size() << "]>";
                break;
            case TypeKind::MAP_TYPE:
                ss << "<" << type_name << "[" << node.data().size() << "]>";
                break;
            case TypeKind::STRUCTURE_TYPE:
                ss << "Structure: <" << type_name << ">";
                break;
            case TypeKind::ENUMERATION_TYPE:
                ss << "Enumeration: <" << type_name << ">";
            default:
                ss << "Unsupported type: " << type_name;
        }
        ss << std::endl;
    });

    return ss.str();
}

template<>
inline std::string ReadableDynamicDataRef::cast<std::string>() const
{
    xtypes_assert(type_.is_primitive_type() ||
           type_.kind() == TypeKind::STRING_TYPE ||
           type_.kind() == TypeKind::WSTRING_TYPE ||
           type_.is_enumerated_type(),
        "Expected a primitive or string type but '" << type_.name() << "' received while casting data to 'std::string'.");
    switch (type_.kind())
    {
        case TypeKind::BOOLEAN_TYPE:
        {
            bool temp = *this;
            return std::to_string(temp);
        }
        case TypeKind::INT_8_TYPE:
        {
            int8_t temp = *this;
            return std::to_string(temp);
        }
        case TypeKind::UINT_8_TYPE:
        {
            uint8_t temp = *this;
            return std::to_string(temp);
        }
        case TypeKind::INT_16_TYPE:
        {
            int16_t temp = *this;
            return std::to_string(temp);
        }
        case TypeKind::UINT_16_TYPE:
        {
            uint16_t temp = *this;
            return std::to_string(temp);
        }
        case TypeKind::INT_32_TYPE:
        {
            int32_t temp = *this;
            return std::to_string(temp);
        }
        case TypeKind::UINT_32_TYPE:
        {
            uint32_t temp = *this;
            return std::to_string(temp);
        }
        case TypeKind::INT_64_TYPE:
        {
            int64_t temp = *this;
            return std::to_string(temp);
        }
        case TypeKind::UINT_64_TYPE:
        {
            uint64_t temp = *this;
            return std::to_string(temp);
        }
        case TypeKind::FLOAT_32_TYPE:
        {
            float temp = *this;
            return std::to_string(temp);
        }
        case TypeKind::FLOAT_64_TYPE:
        {
            double temp = *this;
            return std::to_string(temp);
        }
        case TypeKind::FLOAT_128_TYPE:
        {
            long double temp = *this;
            return std::to_string(temp);
        }
        case TypeKind::CHAR_8_TYPE:
        {
            char temp = *this;
            return std::to_string(temp);
        }
        case TypeKind::CHAR_16_TYPE:
        {
            wchar_t temp = *this;
            return std::to_string(temp);
        }
        case TypeKind::ENUMERATION_TYPE:
        {
            // For checking the associated_type, check for its memory_size
            if (type_.memory_size() == sizeof(uint8_t))
            {
                uint8_t temp = *this;
                return std::to_string(temp);
            }
            else if (type_.memory_size() == sizeof(uint16_t))
            {
                uint16_t temp = *this;
                return std::to_string(temp);
            }
            else if (type_.memory_size() == sizeof(uint32_t))
            {
                uint32_t temp = *this;
                return std::to_string(temp);
            }
        }
        case TypeKind::STRING_TYPE:
        {
            std::string temp = *this;
            return temp;
        }
        /* // TODO
        case TypeKind::WSTRING_TYPE:
        {
            std::wstring temp = *this;
            return reinterpret_cast<T>(temp);
        }
        */
    }
    return std::string();
}

#define DYNAMIC_DATA_UNARY_OPERATOR_RESULT(TYPE, OPERATOR) \
{\
    DynamicData result(primitive_type<TYPE>());\
    result = OPERATOR(this->ReadableDynamicDataRef::value<TYPE>());\
    return result;\
}

inline DynamicData DynamicData::operator - () const
{
    switch(type_.kind())
    {
        case TypeKind::INT_8_TYPE:
            DYNAMIC_DATA_UNARY_OPERATOR_RESULT(int8_t, -);
        case TypeKind::UINT_8_TYPE:
            DYNAMIC_DATA_UNARY_OPERATOR_RESULT(uint8_t, -);
        case TypeKind::INT_16_TYPE:
            DYNAMIC_DATA_UNARY_OPERATOR_RESULT(int16_t, -);
        case TypeKind::UINT_16_TYPE:
            DYNAMIC_DATA_UNARY_OPERATOR_RESULT(uint16_t, -);
        case TypeKind::INT_32_TYPE:
            DYNAMIC_DATA_UNARY_OPERATOR_RESULT(int32_t, -);
        case TypeKind::UINT_32_TYPE:
            DYNAMIC_DATA_UNARY_OPERATOR_RESULT(uint32_t, -);
        case TypeKind::INT_64_TYPE:
            DYNAMIC_DATA_UNARY_OPERATOR_RESULT(int64_t, -);
        case TypeKind::UINT_64_TYPE:
            DYNAMIC_DATA_UNARY_OPERATOR_RESULT(uint64_t, -);
        case TypeKind::FLOAT_32_TYPE:
            DYNAMIC_DATA_UNARY_OPERATOR_RESULT(float, -);
        case TypeKind::FLOAT_64_TYPE:
            DYNAMIC_DATA_UNARY_OPERATOR_RESULT(double, -);
        case TypeKind::FLOAT_128_TYPE:
            DYNAMIC_DATA_UNARY_OPERATOR_RESULT(long double, -);
        default:
            xtypes_assert(false, "operator-() isn't available for type '" << type_.name() << "'.");
            break;
    }

    return *this;
}

inline DynamicData DynamicData::operator ~ () const
{
    switch(type_.kind())
    {
        case TypeKind::INT_8_TYPE:
            DYNAMIC_DATA_UNARY_OPERATOR_RESULT(int8_t, ~);
        case TypeKind::UINT_8_TYPE:
            DYNAMIC_DATA_UNARY_OPERATOR_RESULT(uint8_t, ~);
        case TypeKind::INT_16_TYPE:
            DYNAMIC_DATA_UNARY_OPERATOR_RESULT(int16_t, ~);
        case TypeKind::UINT_16_TYPE:
            DYNAMIC_DATA_UNARY_OPERATOR_RESULT(uint16_t, ~);
        case TypeKind::INT_32_TYPE:
            DYNAMIC_DATA_UNARY_OPERATOR_RESULT(int32_t, ~);
        case TypeKind::UINT_32_TYPE:
            DYNAMIC_DATA_UNARY_OPERATOR_RESULT(uint32_t, ~);
        case TypeKind::INT_64_TYPE:
            DYNAMIC_DATA_UNARY_OPERATOR_RESULT(int64_t, ~);
        case TypeKind::UINT_64_TYPE:
            DYNAMIC_DATA_UNARY_OPERATOR_RESULT(uint64_t, ~);
        default:
            xtypes_assert(false, "operator~() isn't available for type '" << type_.name() << "'.");
            break;
    }

    return *this;
}

#define DYNAMIC_DATA_NOT_OPERATOR_RESULT(TYPE) \
{\
    return !bool(this->ReadableDynamicDataRef::value<TYPE>());\
}

#define DYNAMIC_DATA_SELF_DE_INCREMENT(TYPE, OPERATOR) \
{\
    TYPE pre = *this; \
    this->WritableDynamicDataRef::value<TYPE>(OPERATOR(pre)); \
    return *this; \
}

#define DYNAMIC_DATA_SELF_INCREMENT(TYPE) DYNAMIC_DATA_SELF_DE_INCREMENT(TYPE, ++)

inline DynamicData DynamicData::operator ++ (int)
{
    switch(type_.kind())
    {
        case TypeKind::INT_8_TYPE:
            DYNAMIC_DATA_SELF_INCREMENT(int8_t);
        case TypeKind::UINT_8_TYPE:
            DYNAMIC_DATA_SELF_INCREMENT(uint8_t);
        case TypeKind::INT_16_TYPE:
            DYNAMIC_DATA_SELF_INCREMENT(int16_t);
        case TypeKind::UINT_16_TYPE:
            DYNAMIC_DATA_SELF_INCREMENT(uint16_t);
        case TypeKind::INT_32_TYPE:
            DYNAMIC_DATA_SELF_INCREMENT(int32_t);
        case TypeKind::UINT_32_TYPE:
            DYNAMIC_DATA_SELF_INCREMENT(uint32_t);
        case TypeKind::INT_64_TYPE:
            DYNAMIC_DATA_SELF_INCREMENT(int64_t);
        case TypeKind::UINT_64_TYPE:
            DYNAMIC_DATA_SELF_INCREMENT(uint64_t);
        default:
            xtypes_assert(false, "operator++() isn't available for type '" << type_.name() << "'.");
    }
}

inline DynamicData& DynamicData::operator ++ ()
{
    WritableDynamicDataRef wthis = this->ref();
    wthis = DynamicData::operator++(1);
    return *this;
}

#define DYNAMIC_DATA_SELF_DECREMENT(TYPE) DYNAMIC_DATA_SELF_DE_INCREMENT(TYPE, --)

inline DynamicData DynamicData::operator -- (int)
{
    switch(type_.kind())
    {
        case TypeKind::INT_8_TYPE:
            DYNAMIC_DATA_SELF_DECREMENT(int8_t);
        case TypeKind::UINT_8_TYPE:
            DYNAMIC_DATA_SELF_DECREMENT(uint8_t);
        case TypeKind::INT_16_TYPE:
            DYNAMIC_DATA_SELF_DECREMENT(int16_t);
        case TypeKind::UINT_16_TYPE:
            DYNAMIC_DATA_SELF_DECREMENT(uint16_t);
        case TypeKind::INT_32_TYPE:
            DYNAMIC_DATA_SELF_DECREMENT(int32_t);
        case TypeKind::UINT_32_TYPE:
            DYNAMIC_DATA_SELF_DECREMENT(uint32_t);
        case TypeKind::INT_64_TYPE:
            DYNAMIC_DATA_SELF_DECREMENT(int64_t);
        case TypeKind::UINT_64_TYPE:
            DYNAMIC_DATA_SELF_DECREMENT(uint64_t);
        default:
            xtypes_assert(false, "operator--() isn't available for type '" << type_.name() <<  "'.");
    }
}

inline DynamicData& DynamicData::operator -- ()
{
    WritableDynamicDataRef wthis = this->ref();
    wthis = DynamicData::operator--(1);
    return *this;
}

inline bool DynamicData::operator ! () const
{
    switch(type_.kind())
    {
        case TypeKind::CHAR_8_TYPE:
            DYNAMIC_DATA_NOT_OPERATOR_RESULT(char);
        case TypeKind::CHAR_16_TYPE:
            DYNAMIC_DATA_NOT_OPERATOR_RESULT(char16_t);
        case TypeKind::BOOLEAN_TYPE:
            DYNAMIC_DATA_NOT_OPERATOR_RESULT(bool);
        case TypeKind::INT_8_TYPE:
            DYNAMIC_DATA_NOT_OPERATOR_RESULT(int8_t);
        case TypeKind::UINT_8_TYPE:
            DYNAMIC_DATA_NOT_OPERATOR_RESULT(uint8_t);
        case TypeKind::INT_16_TYPE:
            DYNAMIC_DATA_NOT_OPERATOR_RESULT(int16_t);
        case TypeKind::UINT_16_TYPE:
            DYNAMIC_DATA_NOT_OPERATOR_RESULT(uint16_t);
        case TypeKind::INT_32_TYPE:
            DYNAMIC_DATA_NOT_OPERATOR_RESULT(int32_t);
        case TypeKind::UINT_32_TYPE:
            DYNAMIC_DATA_NOT_OPERATOR_RESULT(uint32_t);
        case TypeKind::INT_64_TYPE:
            DYNAMIC_DATA_NOT_OPERATOR_RESULT(int64_t);
        case TypeKind::UINT_64_TYPE:
            DYNAMIC_DATA_NOT_OPERATOR_RESULT(uint64_t);
        case TypeKind::STRING_TYPE:
            return this->value<std::string>().empty();
        case TypeKind::WSTRING_TYPE:
            return this->value<std::wstring>().empty();
        default:
            xtypes_assert(false, "operator!() isn't available for type '" << "'.");
    }
}

#define DYNAMIC_DATA_LOGIC_OPERATION(TYPE, OPERATOR) \
{\
    switch(other.type().kind())\
    {\
        case TypeKind::CHAR_8_TYPE:\
            return (this->ReadableDynamicDataRef::value<TYPE>() OPERATOR other.ReadableDynamicDataRef::value<char>());\
        case TypeKind::CHAR_16_TYPE:\
            return (this->ReadableDynamicDataRef::value<TYPE>() OPERATOR other.ReadableDynamicDataRef::value<char16_t>());\
        case TypeKind::BOOLEAN_TYPE:\
            return (this->ReadableDynamicDataRef::value<TYPE>() OPERATOR other.ReadableDynamicDataRef::value<bool>());\
        case TypeKind::INT_8_TYPE:\
            return (this->ReadableDynamicDataRef::value<TYPE>() OPERATOR other.ReadableDynamicDataRef::value<int8_t>());\
        case TypeKind::UINT_8_TYPE:\
            return (this->ReadableDynamicDataRef::value<TYPE>() OPERATOR other.ReadableDynamicDataRef::value<uint8_t>());\
        case TypeKind::INT_16_TYPE:\
            return (this->ReadableDynamicDataRef::value<TYPE>() OPERATOR other.ReadableDynamicDataRef::value<int16_t>());\
        case TypeKind::UINT_16_TYPE:\
            return (this->ReadableDynamicDataRef::value<TYPE>() OPERATOR other.ReadableDynamicDataRef::value<uint16_t>());\
        case TypeKind::INT_32_TYPE:\
            return (this->ReadableDynamicDataRef::value<TYPE>() OPERATOR other.ReadableDynamicDataRef::value<int32_t>());\
        case TypeKind::UINT_32_TYPE:\
            return (this->ReadableDynamicDataRef::value<TYPE>() OPERATOR other.ReadableDynamicDataRef::value<uint32_t>());\
        case TypeKind::INT_64_TYPE:\
            return (this->ReadableDynamicDataRef::value<TYPE>() OPERATOR other.ReadableDynamicDataRef::value<int64_t>());\
        case TypeKind::UINT_64_TYPE:\
            return (this->ReadableDynamicDataRef::value<TYPE>() OPERATOR other.ReadableDynamicDataRef::value<uint64_t>());\
        case TypeKind::FLOAT_32_TYPE:\
            return (this->ReadableDynamicDataRef::value<TYPE>() OPERATOR other.ReadableDynamicDataRef::value<float>());\
        case TypeKind::FLOAT_64_TYPE:\
            return (this->ReadableDynamicDataRef::value<TYPE>() OPERATOR other.ReadableDynamicDataRef::value<double>());\
        case TypeKind::FLOAT_128_TYPE:\
            return (this->ReadableDynamicDataRef::value<TYPE>() OPERATOR other.ReadableDynamicDataRef::value<long double>());\
        default:\
            xtypes_assert(false,\
                "operator&&() isn't available for types '" << type_.name() << " and " <<  other.type().name() << "'.");\
    }\
}

inline bool DynamicData::operator && (const ReadableDynamicDataRef& other) const
{
    switch (type_.kind())
    {
        case TypeKind::CHAR_8_TYPE:
            DYNAMIC_DATA_LOGIC_OPERATION(char, &&);
        case TypeKind::CHAR_16_TYPE:
            DYNAMIC_DATA_LOGIC_OPERATION(char16_t, &&);
        case TypeKind::BOOLEAN_TYPE:
            DYNAMIC_DATA_LOGIC_OPERATION(bool, &&);
        case TypeKind::INT_8_TYPE:
            DYNAMIC_DATA_LOGIC_OPERATION(int8_t, &&);
        case TypeKind::UINT_8_TYPE:
            DYNAMIC_DATA_LOGIC_OPERATION(uint8_t, &&);
        case TypeKind::INT_16_TYPE:
            DYNAMIC_DATA_LOGIC_OPERATION(int16_t, &&);
        case TypeKind::UINT_16_TYPE:
            DYNAMIC_DATA_LOGIC_OPERATION(uint16_t, &&);
        case TypeKind::INT_32_TYPE:
            DYNAMIC_DATA_LOGIC_OPERATION(int32_t, &&);
        case TypeKind::UINT_32_TYPE:
            DYNAMIC_DATA_LOGIC_OPERATION(uint32_t, &&);
        case TypeKind::INT_64_TYPE:
            DYNAMIC_DATA_LOGIC_OPERATION(int64_t, &&);
        case TypeKind::UINT_64_TYPE:
            DYNAMIC_DATA_LOGIC_OPERATION(uint64_t, &&);
        case TypeKind::FLOAT_32_TYPE:
            DYNAMIC_DATA_LOGIC_OPERATION(float, &&);
        case TypeKind::FLOAT_64_TYPE:
            DYNAMIC_DATA_LOGIC_OPERATION(double, &&);
        case TypeKind::FLOAT_128_TYPE:
            DYNAMIC_DATA_LOGIC_OPERATION(long double, &&);
        default:
            xtypes_assert(false,
                "operator&&() isn't available for types '" << type_.name() << " and " <<  other.type().name() << "'.");
    }
}

inline bool DynamicData::operator || (const ReadableDynamicDataRef& other) const
{
    switch (type_.kind())
    {
        case TypeKind::CHAR_8_TYPE:
            DYNAMIC_DATA_LOGIC_OPERATION(char, ||);
        case TypeKind::CHAR_16_TYPE:
            DYNAMIC_DATA_LOGIC_OPERATION(char16_t, ||);
        case TypeKind::BOOLEAN_TYPE:
            DYNAMIC_DATA_LOGIC_OPERATION(bool, ||);
        case TypeKind::INT_8_TYPE:
            DYNAMIC_DATA_LOGIC_OPERATION(int8_t, ||);
        case TypeKind::UINT_8_TYPE:
            DYNAMIC_DATA_LOGIC_OPERATION(uint8_t, ||);
        case TypeKind::INT_16_TYPE:
            DYNAMIC_DATA_LOGIC_OPERATION(int16_t, ||);
        case TypeKind::UINT_16_TYPE:
            DYNAMIC_DATA_LOGIC_OPERATION(uint16_t, ||);
        case TypeKind::INT_32_TYPE:
            DYNAMIC_DATA_LOGIC_OPERATION(int32_t, ||);
        case TypeKind::UINT_32_TYPE:
            DYNAMIC_DATA_LOGIC_OPERATION(uint32_t, ||);
        case TypeKind::INT_64_TYPE:
            DYNAMIC_DATA_LOGIC_OPERATION(int64_t, ||);
        case TypeKind::UINT_64_TYPE:
            DYNAMIC_DATA_LOGIC_OPERATION(uint64_t, ||);
        case TypeKind::FLOAT_32_TYPE:
            DYNAMIC_DATA_LOGIC_OPERATION(float, ||);
        case TypeKind::FLOAT_64_TYPE:
            DYNAMIC_DATA_LOGIC_OPERATION(double, ||);
        case TypeKind::FLOAT_128_TYPE:
            DYNAMIC_DATA_LOGIC_OPERATION(long double, ||);
        default:
            xtypes_assert(false,
                "operator||() isn't available for types '" << type_.name() << " and " <<  other.type().name() << "'.");
    }
}

#define DYNAMIC_DATA_OPERATOR_RESULT(TYPE, OPERATOR) \
{\
    TYPE lho = *this;\
    TYPE rho = other;\
    DynamicData result(primitive_type<TYPE>());\
    result = lho OPERATOR rho;\
    return result;\
}

#define DYNAMIC_DATA_NUMERIC_INT_OPERATOR_RESULT(OPERATOR) \
{\
    switch(type_.kind())\
    {\
        case TypeKind::INT_8_TYPE:\
            DYNAMIC_DATA_OPERATOR_RESULT(int8_t, OPERATOR);\
        case TypeKind::UINT_8_TYPE:\
            DYNAMIC_DATA_OPERATOR_RESULT(uint8_t, OPERATOR);\
        case TypeKind::INT_16_TYPE:\
            DYNAMIC_DATA_OPERATOR_RESULT(int16_t, OPERATOR);\
        case TypeKind::UINT_16_TYPE:\
            DYNAMIC_DATA_OPERATOR_RESULT(uint16_t, OPERATOR);\
        case TypeKind::INT_32_TYPE:\
            DYNAMIC_DATA_OPERATOR_RESULT(int32_t, OPERATOR);\
        case TypeKind::UINT_32_TYPE:\
            DYNAMIC_DATA_OPERATOR_RESULT(uint32_t, OPERATOR);\
        case TypeKind::INT_64_TYPE:\
            DYNAMIC_DATA_OPERATOR_RESULT(int64_t, OPERATOR);\
        case TypeKind::UINT_64_TYPE:\
            DYNAMIC_DATA_OPERATOR_RESULT(uint64_t, OPERATOR);\
        default:\
            xtypes_assert(false,\
                "operator" << #OPERATOR << "() isn't available for types '" << type_.name() << "' and '" << other.type().name() << "'.");\
                return *this;\
    }\
}

#define DYNAMIC_DATA_NUMERIC_OPERATOR_RESULT(OPERATOR)\
{\
    switch(type_.kind())\
    {\
        case TypeKind::FLOAT_32_TYPE:\
            DYNAMIC_DATA_OPERATOR_RESULT(float, OPERATOR);\
        case TypeKind::FLOAT_64_TYPE:\
            DYNAMIC_DATA_OPERATOR_RESULT(double, OPERATOR);\
        case TypeKind::FLOAT_128_TYPE:\
            DYNAMIC_DATA_OPERATOR_RESULT(long double, OPERATOR);\
    }\
    DYNAMIC_DATA_NUMERIC_INT_OPERATOR_RESULT(OPERATOR);\
}

inline DynamicData DynamicData::operator * (const ReadableDynamicDataRef& other) const
{
    DYNAMIC_DATA_NUMERIC_OPERATOR_RESULT(*);
}

inline DynamicData DynamicData::operator / (const ReadableDynamicDataRef& other) const
{
    DYNAMIC_DATA_NUMERIC_OPERATOR_RESULT(/);
}

inline DynamicData DynamicData::operator % (const ReadableDynamicDataRef& other) const
{
    DYNAMIC_DATA_NUMERIC_INT_OPERATOR_RESULT(%);
}

inline DynamicData DynamicData::operator + (const ReadableDynamicDataRef& other) const
{
    DYNAMIC_DATA_NUMERIC_OPERATOR_RESULT(+);
}

inline DynamicData DynamicData::operator - (const ReadableDynamicDataRef& other) const
{
    DYNAMIC_DATA_NUMERIC_OPERATOR_RESULT(-);
}

inline DynamicData DynamicData::operator < (const ReadableDynamicDataRef& other) const
{
    DYNAMIC_DATA_NUMERIC_OPERATOR_RESULT(<);
}

inline DynamicData DynamicData::operator > (const ReadableDynamicDataRef& other) const
{
    DYNAMIC_DATA_NUMERIC_OPERATOR_RESULT(>);
}

inline DynamicData DynamicData::operator << (const ReadableDynamicDataRef& other) const
{
    DYNAMIC_DATA_NUMERIC_INT_OPERATOR_RESULT(<<);
}

inline DynamicData DynamicData::operator >> (const ReadableDynamicDataRef& other) const
{
    DYNAMIC_DATA_NUMERIC_INT_OPERATOR_RESULT(>>);
}

inline DynamicData DynamicData::operator & (const ReadableDynamicDataRef& other) const
{
    DYNAMIC_DATA_NUMERIC_INT_OPERATOR_RESULT(&);
}

inline DynamicData DynamicData::operator ^ (const ReadableDynamicDataRef& other) const
{
    DYNAMIC_DATA_NUMERIC_INT_OPERATOR_RESULT(^);
}

inline DynamicData DynamicData::operator | (const ReadableDynamicDataRef& other) const
{
    DYNAMIC_DATA_NUMERIC_INT_OPERATOR_RESULT(|);
}

#define DYNAMIC_DATA_SELF_ASSIGN_OPERATOR_RESULT(OPERATOR) { \
    WritableDynamicDataRef wthis = this->ref();\
    wthis = DynamicData::operator OPERATOR(other);\
    return *this;\
}

#define DYNAMIC_DATA_PRIMITIVE_SELF_ASSIGN_OPERATOR_RESULT(OPERATOR) { \
    xtypes_assert(std::is_arithmetic<T>::value,\
        "Operator" << #OPERATOR << "=() cannot be used with non-arithmetic types");\
    this->WritableDynamicDataRef::value<T>(this->ReadableDynamicDataRef::value<T>() + other);\
    return *this;\
}

#define APPEND(OP1, OP2) OP1##OP2

#define DYNAMIC_DATA_SELF_ASSIGN_OPERATOR_IMPLEMENTATION(OPERATOR) \
inline DynamicData& DynamicData::operator APPEND(OPERATOR,=) (const ReadableDynamicDataRef& other)\
{\
    DYNAMIC_DATA_SELF_ASSIGN_OPERATOR_RESULT(OPERATOR);\
}\
\
template <typename T, typename>\
    inline DynamicData& DynamicData::operator APPEND(OPERATOR,=) (const T& other)\
{\
    DYNAMIC_DATA_PRIMITIVE_SELF_ASSIGN_OPERATOR_RESULT(OPERATOR);\
}

DYNAMIC_DATA_SELF_ASSIGN_OPERATOR_IMPLEMENTATION(*);

DYNAMIC_DATA_SELF_ASSIGN_OPERATOR_IMPLEMENTATION(/);

DYNAMIC_DATA_SELF_ASSIGN_OPERATOR_IMPLEMENTATION(%);

DYNAMIC_DATA_SELF_ASSIGN_OPERATOR_IMPLEMENTATION(+);

DYNAMIC_DATA_SELF_ASSIGN_OPERATOR_IMPLEMENTATION(-);

DYNAMIC_DATA_SELF_ASSIGN_OPERATOR_IMPLEMENTATION(<);

DYNAMIC_DATA_SELF_ASSIGN_OPERATOR_IMPLEMENTATION(>);

DYNAMIC_DATA_SELF_ASSIGN_OPERATOR_IMPLEMENTATION(<<);

DYNAMIC_DATA_SELF_ASSIGN_OPERATOR_IMPLEMENTATION(>>);

DYNAMIC_DATA_SELF_ASSIGN_OPERATOR_IMPLEMENTATION(&);

DYNAMIC_DATA_SELF_ASSIGN_OPERATOR_IMPLEMENTATION(^);

DYNAMIC_DATA_SELF_ASSIGN_OPERATOR_IMPLEMENTATION(|);

} //namespace xtypes
} //namespace eprosima

#endif //EPROSIMA_XTYPES_DYNAMIC_DATA_IMPL_HPP_
