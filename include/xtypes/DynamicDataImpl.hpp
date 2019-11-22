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
    assert(type_.is_primitive_type());
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
            // For checking the associated_type, any cast is valid, as long as e_type isn't accessed for anything else.
            const EnumeratedType<uint8_t>& e_type = static_cast<const EnumeratedType<uint8_t>&>(type_);
            if (std::type_index(e_type.get_associated_type()) == std::type_index(typeid(uint8_t)))
            {
                uint8_t temp = *this;
                return std::to_string(temp);
            }
            else if (std::type_index(e_type.get_associated_type()) == std::type_index(typeid(uint16_t)))
            {
                uint16_t temp = *this;
                return std::to_string(temp);
            }
            else if (std::type_index(e_type.get_associated_type()) == std::type_index(typeid(uint32_t)))
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

#define DYNAMIC_DATA_NEGATE(TYPE) \
{\
    DynamicData result(primitive_type<TYPE>());\
    result = -(*this);\
    return result;\
}

inline DynamicData DynamicData::operator - () const
{
    switch(type_.kind())
    {
        case TypeKind::INT_8_TYPE:
            DYNAMIC_DATA_NEGATE(int8_t);
        case TypeKind::UINT_8_TYPE:
            DYNAMIC_DATA_NEGATE(uint8_t);
        case TypeKind::INT_16_TYPE:
            DYNAMIC_DATA_NEGATE(int16_t);
        case TypeKind::UINT_16_TYPE:
            DYNAMIC_DATA_NEGATE(uint16_t);
        case TypeKind::INT_32_TYPE:
            DYNAMIC_DATA_NEGATE(int32_t);
        case TypeKind::UINT_32_TYPE:
            DYNAMIC_DATA_NEGATE(uint32_t);
        case TypeKind::INT_64_TYPE:
            DYNAMIC_DATA_NEGATE(int64_t);
        case TypeKind::UINT_64_TYPE:
            DYNAMIC_DATA_NEGATE(uint64_t);
        case TypeKind::FLOAT_32_TYPE:
            DYNAMIC_DATA_NEGATE(float);
        case TypeKind::FLOAT_64_TYPE:
            DYNAMIC_DATA_NEGATE(double);
        case TypeKind::FLOAT_128_TYPE:
            DYNAMIC_DATA_NEGATE(long double);
        default:
            assert(false);
            break;
    }

    return *this;
}

#define DYNAMIC_DATA_OPERATOR_RESULT(TYPE, OPERATOR) \
{\
    TYPE lho = *this;\
    TYPE rho = other;\
    DynamicData result(primitive_type<TYPE>());\
    result = lho OPERATOR rho;\
    return result;\
}

inline DynamicData DynamicData::operator * (const ReadableDynamicDataRef& other) const
{
    switch(type_.kind())
    {
        case TypeKind::INT_8_TYPE:
            DYNAMIC_DATA_OPERATOR_RESULT(int8_t, *);
        case TypeKind::UINT_8_TYPE:
            DYNAMIC_DATA_OPERATOR_RESULT(uint8_t, *);
        case TypeKind::INT_16_TYPE:
            DYNAMIC_DATA_OPERATOR_RESULT(int16_t, *);
        case TypeKind::UINT_16_TYPE:
            DYNAMIC_DATA_OPERATOR_RESULT(uint16_t, *);
        case TypeKind::INT_32_TYPE:
            DYNAMIC_DATA_OPERATOR_RESULT(int32_t, *);
        case TypeKind::UINT_32_TYPE:
            DYNAMIC_DATA_OPERATOR_RESULT(uint32_t, *);
        case TypeKind::INT_64_TYPE:
            DYNAMIC_DATA_OPERATOR_RESULT(int64_t, *);
        case TypeKind::UINT_64_TYPE:
            DYNAMIC_DATA_OPERATOR_RESULT(uint64_t, *);
        case TypeKind::FLOAT_32_TYPE:
            DYNAMIC_DATA_OPERATOR_RESULT(float, *);
        case TypeKind::FLOAT_64_TYPE:
            DYNAMIC_DATA_OPERATOR_RESULT(double, *);
        case TypeKind::FLOAT_128_TYPE:
            DYNAMIC_DATA_OPERATOR_RESULT(long double, *);
        default:
            assert(false);
            return *this;
    }
}

inline DynamicData DynamicData::operator / (const ReadableDynamicDataRef& other) const
{
    switch(type_.kind())
    {
        case TypeKind::INT_8_TYPE:
            DYNAMIC_DATA_OPERATOR_RESULT(int8_t, /);
        case TypeKind::UINT_8_TYPE:
            DYNAMIC_DATA_OPERATOR_RESULT(uint8_t, /);
        case TypeKind::INT_16_TYPE:
            DYNAMIC_DATA_OPERATOR_RESULT(int16_t, /);
        case TypeKind::UINT_16_TYPE:
            DYNAMIC_DATA_OPERATOR_RESULT(uint16_t, /);
        case TypeKind::INT_32_TYPE:
            DYNAMIC_DATA_OPERATOR_RESULT(int32_t, /);
        case TypeKind::UINT_32_TYPE:
            DYNAMIC_DATA_OPERATOR_RESULT(uint32_t, /);
        case TypeKind::INT_64_TYPE:
            DYNAMIC_DATA_OPERATOR_RESULT(int64_t, /);
        case TypeKind::UINT_64_TYPE:
            DYNAMIC_DATA_OPERATOR_RESULT(uint64_t, /);
        case TypeKind::FLOAT_32_TYPE:
            DYNAMIC_DATA_OPERATOR_RESULT(float, /);
        case TypeKind::FLOAT_64_TYPE:
            DYNAMIC_DATA_OPERATOR_RESULT(double, /);
        case TypeKind::FLOAT_128_TYPE:
            DYNAMIC_DATA_OPERATOR_RESULT(long double, /);
        default:
            assert(false);
            return *this;
    }

}

inline DynamicData DynamicData::operator % (const ReadableDynamicDataRef& other) const
{
    switch(type_.kind())
    {
        case TypeKind::INT_8_TYPE:
            DYNAMIC_DATA_OPERATOR_RESULT(int8_t, %);
        case TypeKind::UINT_8_TYPE:
            DYNAMIC_DATA_OPERATOR_RESULT(uint8_t, %);
        case TypeKind::INT_16_TYPE:
            DYNAMIC_DATA_OPERATOR_RESULT(int16_t, %);
        case TypeKind::UINT_16_TYPE:
            DYNAMIC_DATA_OPERATOR_RESULT(uint16_t, %);
        case TypeKind::INT_32_TYPE:
            DYNAMIC_DATA_OPERATOR_RESULT(int32_t, %);
        case TypeKind::UINT_32_TYPE:
            DYNAMIC_DATA_OPERATOR_RESULT(uint32_t, %);
        case TypeKind::INT_64_TYPE:
            DYNAMIC_DATA_OPERATOR_RESULT(int64_t, %);
        case TypeKind::UINT_64_TYPE:
            DYNAMIC_DATA_OPERATOR_RESULT(uint64_t, %);
        default:
            assert(false);
            return *this;
    }

}

inline DynamicData DynamicData::operator + (const ReadableDynamicDataRef& other) const
{
    switch(type_.kind())
    {
        case TypeKind::INT_8_TYPE:
            DYNAMIC_DATA_OPERATOR_RESULT(int8_t, +);
        case TypeKind::UINT_8_TYPE:
            DYNAMIC_DATA_OPERATOR_RESULT(uint8_t, +);
        case TypeKind::INT_16_TYPE:
            DYNAMIC_DATA_OPERATOR_RESULT(int16_t, +);
        case TypeKind::UINT_16_TYPE:
            DYNAMIC_DATA_OPERATOR_RESULT(uint16_t, +);
        case TypeKind::INT_32_TYPE:
            DYNAMIC_DATA_OPERATOR_RESULT(int32_t, +);
        case TypeKind::UINT_32_TYPE:
            DYNAMIC_DATA_OPERATOR_RESULT(uint32_t, +);
        case TypeKind::INT_64_TYPE:
            DYNAMIC_DATA_OPERATOR_RESULT(int64_t, +);
        case TypeKind::UINT_64_TYPE:
            DYNAMIC_DATA_OPERATOR_RESULT(uint64_t, +);
        case TypeKind::FLOAT_32_TYPE:
            DYNAMIC_DATA_OPERATOR_RESULT(float, +);
        case TypeKind::FLOAT_64_TYPE:
            DYNAMIC_DATA_OPERATOR_RESULT(double, +);
        case TypeKind::FLOAT_128_TYPE:
            DYNAMIC_DATA_OPERATOR_RESULT(long double, +);
        default:
            assert(false);
            return *this;
    }

}

inline DynamicData DynamicData::operator - (const ReadableDynamicDataRef& other) const
{
    switch(type_.kind())
    {
        case TypeKind::INT_8_TYPE:
            DYNAMIC_DATA_OPERATOR_RESULT(int8_t, -);
        case TypeKind::UINT_8_TYPE:
            DYNAMIC_DATA_OPERATOR_RESULT(uint8_t, -);
        case TypeKind::INT_16_TYPE:
            DYNAMIC_DATA_OPERATOR_RESULT(int16_t, -);
        case TypeKind::UINT_16_TYPE:
            DYNAMIC_DATA_OPERATOR_RESULT(uint16_t, -);
        case TypeKind::INT_32_TYPE:
            DYNAMIC_DATA_OPERATOR_RESULT(int32_t, -);
        case TypeKind::UINT_32_TYPE:
            DYNAMIC_DATA_OPERATOR_RESULT(uint32_t, -);
        case TypeKind::INT_64_TYPE:
            DYNAMIC_DATA_OPERATOR_RESULT(int64_t, -);
        case TypeKind::UINT_64_TYPE:
            DYNAMIC_DATA_OPERATOR_RESULT(uint64_t, -);
        case TypeKind::FLOAT_32_TYPE:
            DYNAMIC_DATA_OPERATOR_RESULT(float, -);
        case TypeKind::FLOAT_64_TYPE:
            DYNAMIC_DATA_OPERATOR_RESULT(double, -);
        case TypeKind::FLOAT_128_TYPE:
            DYNAMIC_DATA_OPERATOR_RESULT(long double, -);
        default:
            assert(false);
            return *this;
    }

}

inline DynamicData DynamicData::operator << (const ReadableDynamicDataRef& other) const
{
    switch(type_.kind())
    {
        case TypeKind::INT_8_TYPE:
            DYNAMIC_DATA_OPERATOR_RESULT(int8_t, <<);
        case TypeKind::UINT_8_TYPE:
            DYNAMIC_DATA_OPERATOR_RESULT(uint8_t, <<);
        case TypeKind::INT_16_TYPE:
            DYNAMIC_DATA_OPERATOR_RESULT(int16_t, <<);
        case TypeKind::UINT_16_TYPE:
            DYNAMIC_DATA_OPERATOR_RESULT(uint16_t, <<);
        case TypeKind::INT_32_TYPE:
            DYNAMIC_DATA_OPERATOR_RESULT(int32_t, <<);
        case TypeKind::UINT_32_TYPE:
            DYNAMIC_DATA_OPERATOR_RESULT(uint32_t, <<);
        case TypeKind::INT_64_TYPE:
            DYNAMIC_DATA_OPERATOR_RESULT(int64_t, <<);
        case TypeKind::UINT_64_TYPE:
            DYNAMIC_DATA_OPERATOR_RESULT(uint64_t, <<);
        default:
            assert(false);
            return *this;
    }

}

inline DynamicData DynamicData::operator >> (const ReadableDynamicDataRef& other) const
{
    switch(type_.kind())
    {
        case TypeKind::INT_8_TYPE:
            DYNAMIC_DATA_OPERATOR_RESULT(int8_t, >>);
        case TypeKind::UINT_8_TYPE:
            DYNAMIC_DATA_OPERATOR_RESULT(uint8_t, >>);
        case TypeKind::INT_16_TYPE:
            DYNAMIC_DATA_OPERATOR_RESULT(int16_t, >>);
        case TypeKind::UINT_16_TYPE:
            DYNAMIC_DATA_OPERATOR_RESULT(uint16_t, >>);
        case TypeKind::INT_32_TYPE:
            DYNAMIC_DATA_OPERATOR_RESULT(int32_t, >>);
        case TypeKind::UINT_32_TYPE:
            DYNAMIC_DATA_OPERATOR_RESULT(uint32_t, >>);
        case TypeKind::INT_64_TYPE:
            DYNAMIC_DATA_OPERATOR_RESULT(int64_t, >>);
        case TypeKind::UINT_64_TYPE:
            DYNAMIC_DATA_OPERATOR_RESULT(uint64_t, >>);
        default:
            assert(false);
            return *this;
    }

}

inline DynamicData DynamicData::operator & (const ReadableDynamicDataRef& other) const
{
    switch(type_.kind())
    {
        case TypeKind::INT_8_TYPE:
            DYNAMIC_DATA_OPERATOR_RESULT(int8_t, &);
        case TypeKind::UINT_8_TYPE:
            DYNAMIC_DATA_OPERATOR_RESULT(uint8_t, &);
        case TypeKind::INT_16_TYPE:
            DYNAMIC_DATA_OPERATOR_RESULT(int16_t, &);
        case TypeKind::UINT_16_TYPE:
            DYNAMIC_DATA_OPERATOR_RESULT(uint16_t, &);
        case TypeKind::INT_32_TYPE:
            DYNAMIC_DATA_OPERATOR_RESULT(int32_t, &);
        case TypeKind::UINT_32_TYPE:
            DYNAMIC_DATA_OPERATOR_RESULT(uint32_t, &);
        case TypeKind::INT_64_TYPE:
            DYNAMIC_DATA_OPERATOR_RESULT(int64_t, &);
        case TypeKind::UINT_64_TYPE:
            DYNAMIC_DATA_OPERATOR_RESULT(uint64_t, &);
        default:
            assert(false);
            return *this;
    }

}

inline DynamicData DynamicData::operator ^ (const ReadableDynamicDataRef& other) const
{
    switch(type_.kind())
    {
        case TypeKind::INT_8_TYPE:
            DYNAMIC_DATA_OPERATOR_RESULT(int8_t, ^);
        case TypeKind::UINT_8_TYPE:
            DYNAMIC_DATA_OPERATOR_RESULT(uint8_t, ^);
        case TypeKind::INT_16_TYPE:
            DYNAMIC_DATA_OPERATOR_RESULT(int16_t, ^);
        case TypeKind::UINT_16_TYPE:
            DYNAMIC_DATA_OPERATOR_RESULT(uint16_t, ^);
        case TypeKind::INT_32_TYPE:
            DYNAMIC_DATA_OPERATOR_RESULT(int32_t, ^);
        case TypeKind::UINT_32_TYPE:
            DYNAMIC_DATA_OPERATOR_RESULT(uint32_t, ^);
        case TypeKind::INT_64_TYPE:
            DYNAMIC_DATA_OPERATOR_RESULT(int64_t, ^);
        case TypeKind::UINT_64_TYPE:
            DYNAMIC_DATA_OPERATOR_RESULT(uint64_t, ^);
        default:
            assert(false);
            return *this;
    }

}

inline DynamicData DynamicData::operator | (const ReadableDynamicDataRef& other) const
{
    switch(type_.kind())
    {
        case TypeKind::INT_8_TYPE:
            DYNAMIC_DATA_OPERATOR_RESULT(int8_t, |);
        case TypeKind::UINT_8_TYPE:
            DYNAMIC_DATA_OPERATOR_RESULT(uint8_t, |);
        case TypeKind::INT_16_TYPE:
            DYNAMIC_DATA_OPERATOR_RESULT(int16_t, |);
        case TypeKind::UINT_16_TYPE:
            DYNAMIC_DATA_OPERATOR_RESULT(uint16_t, |);
        case TypeKind::INT_32_TYPE:
            DYNAMIC_DATA_OPERATOR_RESULT(int32_t, |);
        case TypeKind::UINT_32_TYPE:
            DYNAMIC_DATA_OPERATOR_RESULT(uint32_t, |);
        case TypeKind::INT_64_TYPE:
            DYNAMIC_DATA_OPERATOR_RESULT(int64_t, |);
        case TypeKind::UINT_64_TYPE:
            DYNAMIC_DATA_OPERATOR_RESULT(uint64_t, |);
        default:
            assert(false);
            return *this;
    }

}

} //namespace xtypes
} //namespace eprosima

#endif //EPROSIMA_XTYPES_DYNAMIC_DATA_IMPL_HPP_
