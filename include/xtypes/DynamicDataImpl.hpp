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
#include <xtypes/StringConversion.hpp>

#include <sstream>

namespace eprosima {
namespace xtypes {

#define DYNAMIC_DATA_NUMERIC_SIGNED_INT_SWITCH(MACRO, OPERATOR) \
{\
    switch(type_->kind())\
    {\
        case TypeKind::INT_8_TYPE:\
            MACRO(int8_t, OPERATOR);\
        case TypeKind::INT_16_TYPE:\
            MACRO(int16_t, OPERATOR);\
        case TypeKind::INT_32_TYPE:\
            MACRO(int32_t, OPERATOR);\
        case TypeKind::INT_64_TYPE:\
            MACRO(int64_t, OPERATOR);\
        default:\
            xtypes_assert(false,\
                "Operator" << #OPERATOR << "() isn't available for type '" << type_->name() << "'.");\
            return *this;\
    }\
}

#define DYNAMIC_DATA_NUMERIC_INT_SWITCH(MACRO, OPERATOR) \
{\
    switch(type_->kind())\
    {\
        case TypeKind::UINT_8_TYPE:\
            MACRO(uint8_t, OPERATOR);\
        case TypeKind::UINT_16_TYPE:\
            MACRO(uint16_t, OPERATOR);\
        case TypeKind::UINT_32_TYPE:\
            MACRO(uint32_t, OPERATOR);\
        case TypeKind::UINT_64_TYPE:\
            MACRO(uint64_t, OPERATOR);\
        default:\
            DYNAMIC_DATA_NUMERIC_SIGNED_INT_SWITCH(MACRO, OPERATOR);\
    }\
}

#define DYNAMIC_DATA_NUMERIC_FLT_SWITCH(MACRO, OPERATOR) \
{\
    switch(type_->kind())\
    {\
        case TypeKind::FLOAT_32_TYPE:\
            MACRO(float, OPERATOR);\
        case TypeKind::FLOAT_64_TYPE:\
            MACRO(double, OPERATOR);\
        case TypeKind::FLOAT_128_TYPE:\
            MACRO(long double, OPERATOR);\
        default:;\
    }\
}

#define DYNAMIC_DATA_NUMERIC_SIGNED_SWITCH(MACRO, OPERATOR) \
{\
    DYNAMIC_DATA_NUMERIC_FLT_SWITCH(MACRO, OPERATOR);\
    DYNAMIC_DATA_NUMERIC_SIGNED_INT_SWITCH(MACRO, OPERATOR);\
}

#define DYNAMIC_DATA_NUMERIC_SWITCH(MACRO, OPERATOR) \
{\
    DYNAMIC_DATA_NUMERIC_FLT_SWITCH(MACRO, OPERATOR);\
    DYNAMIC_DATA_NUMERIC_INT_SWITCH(MACRO, OPERATOR);\
}

#define DYNAMIC_DATA_BASICTYPE_SWITCH(MACRO, OPERATOR) \
{\
    switch(type_->kind())\
    {\
        case TypeKind::CHAR_8_TYPE:\
            MACRO(char, OPERATOR);\
        case TypeKind::CHAR_16_TYPE:\
            MACRO(char16_t, OPERATOR);\
        case TypeKind::WIDE_CHAR_TYPE:\
            MACRO(wchar_t, OPERATOR);\
        case TypeKind::BOOLEAN_TYPE:\
            MACRO(bool, OPERATOR);\
        default:\
            DYNAMIC_DATA_NUMERIC_SWITCH(MACRO, OPERATOR);\
    }\
}

#define DYNAMIC_DATA_BASICTYPE_INT_SWITCH(MACRO, OPERATOR) \
{\
    switch(type_->kind())\
    {\
        case TypeKind::CHAR_8_TYPE:\
            MACRO(char, OPERATOR);\
        case TypeKind::CHAR_16_TYPE:\
            MACRO(char16_t, OPERATOR);\
        case TypeKind::WIDE_CHAR_TYPE:\
            MACRO(wchar_t, OPERATOR);\
        case TypeKind::BOOLEAN_TYPE:\
            MACRO(bool, OPERATOR);\
        default:\
            DYNAMIC_DATA_NUMERIC_INT_SWITCH(MACRO, OPERATOR);\
    }\
}

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
            case TypeKind::WIDE_CHAR_TYPE:
                {
                    ss << "<" << type_name << ">  ";
                    auto aux = code_conversion_tool<XTYPES_CHAR>(std::u16string(1, node.data().value<char16_t>()));
                    ss << std::string(aux.begin(), aux.end());
                    break;
                }
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
                    ss << "<" << type_name << ">  ";
                    auto aux = node.data().value<std::wstring>();
                    auto aux2 = code_conversion_tool<XTYPES_CHAR>(std::u16string(aux.begin(), aux.end()));
                    ss << std::string(aux2.begin(), aux2.end());
                    break;
                }
            case TypeKind::STRING16_TYPE:
                {
                    ss << "<" << type_name << ">  ";
                    auto aux = code_conversion_tool<XTYPES_CHAR>(node.data().value<std::u16string>());
                    ss << std::string(aux.begin(), aux.end());
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
                break;
            default:
                ss << "Unsupported type: " << type_name;
        }
        ss << std::endl;
    });

    return ss.str();
}

#define DYNAMIC_DATA_CAST(TYPE, ...) \
{\
    TYPE temp = *this;\
    return std::to_string(temp);\
}

template<>
inline std::string ReadableDynamicDataRef::cast<std::string>() const
{
    xtypes_assert(type_->is_primitive_type() ||
           type_->kind() == TypeKind::STRING_TYPE ||
           type_->kind() == TypeKind::WSTRING_TYPE ||
           type_->kind() == TypeKind::STRING16_TYPE ||
           type_->is_enumerated_type(),
        "Expected a primitive or string type but '" << type_->name() << "' received while casting data to 'std::string'.");
    // Custom switch-case statement for types not contained in the macros
    switch (type_->kind())
    {
        case TypeKind::ENUMERATION_TYPE:
        {
            // For checking the associated_type, check for its memory_size
            if (type_->memory_size() == sizeof(uint8_t))
            {
                uint8_t temp = *this;
                return std::to_string(temp);
            }
            else if (type_->memory_size() == sizeof(uint16_t))
            {
                uint16_t temp = *this;
                return std::to_string(temp);
            }
            else if (type_->memory_size() == sizeof(uint32_t))
            {
                uint32_t temp = *this;
                return std::to_string(temp);
            }
            else
            {
                xtypes_assert(false, "invalid enum size")
                return "";
            }
        }
        case TypeKind::STRING_TYPE:
        {
            std::string temp = *this;
            return temp;
        }
        default:
            DYNAMIC_DATA_BASICTYPE_SWITCH(DYNAMIC_DATA_CAST, );
        /* // TODO
        case TypeKind::WSTRING_TYPE:
        {
            std::wstring temp = *this;
            return reinterpret_cast<T>(temp);
        }
        case TypeKind::STRING16_TYPE:
        {
            std::u16string temp = *this;
            return reinterpret_cast<T>(temp);
        }
        */
    }
    return std::string();
}

#define DYNAMIC_DATA_UNARY_OPERATOR_RESULT(TYPE, OPERATOR) \
{\
    DynamicData result(primitive_type<TYPE>());\
    result = static_cast<TYPE>(OPERATOR(this->value<TYPE>()));\
    return result;\
}

inline DynamicData DynamicData::operator - () const
{
    DYNAMIC_DATA_NUMERIC_SIGNED_SWITCH(DYNAMIC_DATA_UNARY_OPERATOR_RESULT, -);
}

inline DynamicData DynamicData::operator ~ () const
{
    DYNAMIC_DATA_NUMERIC_SIGNED_INT_SWITCH(DYNAMIC_DATA_UNARY_OPERATOR_RESULT, ~);
}

#define DYNAMIC_DATA_NOT_OPERATOR_RESULT(TYPE, ...) \
{\
    return !bool(this->value<TYPE>());\
}

#define DYNAMIC_DATA_SELF_DE_INCREMENT(TYPE, OPERATOR) \
{\
    TYPE pre = *this; \
    this->value<TYPE>(OPERATOR(pre)); \
    return *this; \
}

inline DynamicData DynamicData::operator ++ (int)
{
    DYNAMIC_DATA_NUMERIC_INT_SWITCH(DYNAMIC_DATA_SELF_DE_INCREMENT, ++);
}

inline DynamicData& DynamicData::operator ++ ()
{
    WritableDynamicDataRef wthis = this->ref();
    wthis = DynamicData::operator++(1);
    return *this;
}

inline DynamicData DynamicData::operator -- (int)
{
    DYNAMIC_DATA_NUMERIC_INT_SWITCH(DYNAMIC_DATA_SELF_DE_INCREMENT, --);
}

inline DynamicData& DynamicData::operator -- ()
{
    WritableDynamicDataRef wthis = this->ref();
    wthis = DynamicData::operator--(1);
    return *this;
}

inline bool DynamicData::operator ! () const
{
    switch(type_->kind())
    {
        case TypeKind::STRING_TYPE:
            return this->value<std::string>().empty();
        case TypeKind::WSTRING_TYPE:
            return this->value<std::wstring>().empty();
        case TypeKind::STRING16_TYPE:
            return this->value<std::u16string>().empty();
        default:
            DYNAMIC_DATA_BASICTYPE_INT_SWITCH(DYNAMIC_DATA_NOT_OPERATOR_RESULT, !);
    }
}

#define DYNAMIC_DATA_LOGIC_OPERATION(TYPE, OPERATOR) \
{\
    return (this->value<TYPE>() OPERATOR other.value<TYPE>());\
}

#define DYNAMIC_DATA_LOGIC_OPERATOR_IMPLEMENTATION(OPERATOR)\
inline bool DynamicData::operator OPERATOR (const ReadableDynamicDataRef& other) const\
{\
    DYNAMIC_DATA_BASICTYPE_SWITCH(DYNAMIC_DATA_LOGIC_OPERATION, OPERATOR);\
}

DYNAMIC_DATA_LOGIC_OPERATOR_IMPLEMENTATION(&&);

DYNAMIC_DATA_LOGIC_OPERATOR_IMPLEMENTATION(||);

DYNAMIC_DATA_LOGIC_OPERATOR_IMPLEMENTATION(<);

DYNAMIC_DATA_LOGIC_OPERATOR_IMPLEMENTATION(>);

DYNAMIC_DATA_LOGIC_OPERATOR_IMPLEMENTATION(<=);

DYNAMIC_DATA_LOGIC_OPERATOR_IMPLEMENTATION(>=);

#define DYNAMIC_DATA_OPERATOR_RESULT(TYPE, OPERATOR) \
{\
    TYPE lho = *this;\
    TYPE rho = other;\
    TYPE res = lho OPERATOR rho;\
    DynamicData result(primitive_type<TYPE>());\
    result = res;\
    return result;\
}

#define DYNAMIC_DATA_NUMERIC_INT_OPERATOR_RESULT(OPERATOR) \
{\
    DYNAMIC_DATA_NUMERIC_INT_SWITCH(DYNAMIC_DATA_OPERATOR_RESULT, OPERATOR);\
}

#define DYNAMIC_DATA_NUMERIC_OPERATOR_RESULT(OPERATOR)\
{\
    DYNAMIC_DATA_NUMERIC_FLT_SWITCH(DYNAMIC_DATA_OPERATOR_RESULT, OPERATOR);\
    DYNAMIC_DATA_NUMERIC_INT_OPERATOR_RESULT(OPERATOR);\
}

#define DYNAMIC_DATA_NUMERIC_INT_OPERATOR_IMPLEMENTATION(OPERATOR)\
inline DynamicData DynamicData::operator OPERATOR (const ReadableDynamicDataRef& other) const \
{\
    DYNAMIC_DATA_NUMERIC_INT_OPERATOR_RESULT(OPERATOR);\
}

#define DYNAMIC_DATA_NUMERIC_OPERATOR_IMPLEMENTATION(OPERATOR)\
inline DynamicData DynamicData::operator OPERATOR (const ReadableDynamicDataRef& other) const \
{\
    DYNAMIC_DATA_NUMERIC_OPERATOR_RESULT(OPERATOR);\
}

#define DYNAMIC_DATA_NUMERIC_SAFE_OPERATOR_IMPLEMENTATION(OPERATOR)\
inline DynamicData DynamicData::operator OPERATOR (const ReadableDynamicDataRef& other) const \
{\
    switch(type_->kind())\
    {\
        case TypeKind::CHAR_8_TYPE:\
        case TypeKind::CHAR_16_TYPE:\
        case TypeKind::WIDE_CHAR_TYPE:\
        case TypeKind::BOOLEAN_TYPE:\
        {\
            std::ostringstream os;\
            os << "Operator" << (#OPERATOR[0] == '^' ? "\\^" : #OPERATOR)\
               << " is not supported for type " << type_->name();\
            throw std::runtime_error(os.str());\
        }\
        default:\
            DYNAMIC_DATA_NUMERIC_OPERATOR_RESULT(OPERATOR);\
    }\
}

DYNAMIC_DATA_NUMERIC_OPERATOR_IMPLEMENTATION(*);

DYNAMIC_DATA_NUMERIC_OPERATOR_IMPLEMENTATION(/);

DYNAMIC_DATA_NUMERIC_INT_OPERATOR_IMPLEMENTATION(%);

DYNAMIC_DATA_NUMERIC_SAFE_OPERATOR_IMPLEMENTATION(+);

DYNAMIC_DATA_NUMERIC_OPERATOR_IMPLEMENTATION(-);

DYNAMIC_DATA_NUMERIC_INT_OPERATOR_IMPLEMENTATION(<<);

DYNAMIC_DATA_NUMERIC_INT_OPERATOR_IMPLEMENTATION(>>);

DYNAMIC_DATA_NUMERIC_INT_OPERATOR_IMPLEMENTATION(&);

DYNAMIC_DATA_NUMERIC_INT_OPERATOR_IMPLEMENTATION(^);

DYNAMIC_DATA_NUMERIC_INT_OPERATOR_IMPLEMENTATION(|);

#define DYNAMIC_DATA_SELF_ASSIGN_OPERATOR_RESULT(OPERATOR) { \
    WritableDynamicDataRef wthis = this->ref();\
    wthis = DynamicData::operator OPERATOR(other);\
    return *this;\
}

#define DYNAMIC_DATA_PRIMITIVE_SELF_ASSIGN_OPERATOR_RESULT(OPERATOR) { \
    [[maybe_unused]] bool self_assign_valid = std::is_arithmetic<T>::value && !std::is_same<T, bool>::value &&\
            !std::is_same<T, char>::value && !std::is_same<T, wchar_t>::value && !std::is_same<T, char16_t>::value;\
    xtypes_assert(self_assign_valid,\
        "Operator" << #OPERATOR << "=() cannot be used with non-arithmetic types");\
    this->value<T>(this->value<T>() OPERATOR other);\
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

// Specialize for bool to avoid warning -Wint-in-bool-context
template<>
inline DynamicData& DynamicData::operator *=<bool>(const bool& other)
{
    using T = bool;
    DYNAMIC_DATA_PRIMITIVE_SELF_ASSIGN_OPERATOR_RESULT(&&);
}

DYNAMIC_DATA_SELF_ASSIGN_OPERATOR_IMPLEMENTATION(/);

DYNAMIC_DATA_SELF_ASSIGN_OPERATOR_IMPLEMENTATION(%);

DYNAMIC_DATA_SELF_ASSIGN_OPERATOR_IMPLEMENTATION(+);

DYNAMIC_DATA_SELF_ASSIGN_OPERATOR_IMPLEMENTATION(-);

DYNAMIC_DATA_SELF_ASSIGN_OPERATOR_IMPLEMENTATION(<<);

DYNAMIC_DATA_SELF_ASSIGN_OPERATOR_IMPLEMENTATION(>>);

DYNAMIC_DATA_SELF_ASSIGN_OPERATOR_IMPLEMENTATION(&);

DYNAMIC_DATA_SELF_ASSIGN_OPERATOR_IMPLEMENTATION(^);

DYNAMIC_DATA_SELF_ASSIGN_OPERATOR_IMPLEMENTATION(|);

} //namespace xtypes
} //namespace eprosima

#endif //EPROSIMA_XTYPES_DYNAMIC_DATA_IMPL_HPP_
