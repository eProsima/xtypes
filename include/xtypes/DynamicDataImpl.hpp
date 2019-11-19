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
            default:
                ss << "Unsupported type: " << type_name;
        }
        ss << std::endl;
    });

    return ss.str();
}

template<typename T, class = Primitive<T>>
inline T DynamicData::cast() const
{
    assert(type_.is_primitive_type());
    switch (type_.kind())
    {
        case TypeKind::BOOLEAN_TYPE:
        {
            bool temp = *this;
            return static_cast<T>(temp);
        }
        case TypeKind::INT_8_TYPE:
        {
            int8_t temp = *this;
            return static_cast<T>(temp);
        }
        case TypeKind::UINT_8_TYPE:
        {
            uint8_t temp = *this;
            return static_cast<T>(temp);
        }
        case TypeKind::INT_16_TYPE:
        {
            int16_t temp = *this;
            return static_cast<T>(temp);
        }
        case TypeKind::UINT_16_TYPE:
        {
            uint16_t temp = *this;
            return static_cast<T>(temp);
        }
        case TypeKind::INT_32_TYPE:
        {
            int32_t temp = *this;
            return static_cast<T>(temp);
        }
        case TypeKind::UINT_32_TYPE:
        {
            uint32_t temp = *this;
            return static_cast<T>(temp);
        }
        case TypeKind::INT_64_TYPE:
        {
            int64_t temp = *this;
            return static_cast<T>(temp);
        }
        case TypeKind::UINT_64_TYPE:
        {
            uint64_t temp = *this;
            return static_cast<T>(temp);
        }
        case TypeKind::FLOAT_32_TYPE:
        {
            float temp = *this;
            return static_cast<T>(temp);
        }
        case TypeKind::FLOAT_64_TYPE:
        {
            double temp = *this;
            return static_cast<T>(temp);
        }
        case TypeKind::FLOAT_128_TYPE:
        {
            long double temp = *this;
            return static_cast<T>(temp);
        }
        case TypeKind::CHAR_8_TYPE:
        {
            char temp = *this;
            return static_cast<T>(temp);
        }
        case TypeKind::CHAR_16_TYPE:
        {
            wchar_t temp = *this;
            return static_cast<T>(temp);
        }
        //case TypeKind::ENUMERATION_TYPE: TODO
    }
    return T();
}

template<typename T = std::string>
inline T DynamicData::cast() const
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
        //case TypeKind::ENUMERATION_TYPE: TODO
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
    return T();
}

inline DynamicData DynamicData::operator - () const
{
    assert(type_.kind() == TypeKind::INT_8_TYPE || type_.kind() == TypeKind::UINT_8_TYPE ||
           type_.kind() == TypeKind::INT_16_TYPE || type_.kind() == TypeKind::UINT_16_TYPE ||
           type_.kind() == TypeKind::INT_32_TYPE || type_.kind() == TypeKind::UINT_32_TYPE ||
           type_.kind() == TypeKind::INT_64_TYPE || type_.kind() == TypeKind::UINT_64_TYPE ||
           type_.kind() == TypeKind::FLOAT_32_TYPE || type_.kind() == TypeKind::FLOAT_64_TYPE ||
           type_.kind() == TypeKind::FLOAT_128_TYPE);

    switch(type_.kind())
    {
        case TypeKind::INT_8_TYPE:
            {
                DynamicData result(primitive_type<int8_t>());
                result = -(*this);
                return result;
            }
        case TypeKind::UINT_8_TYPE:
            {
                DynamicData result(primitive_type<uint8_t>());
                result = -(*this);
                return result;
            }
        case TypeKind::INT_16_TYPE:
            {
                DynamicData result(primitive_type<int16_t>());
                result = -(*this);
                return result;
            }
        case TypeKind::UINT_16_TYPE:
            {
                DynamicData result(primitive_type<uint16_t>());
                result = -(*this);
                return result;
            }
        case TypeKind::INT_32_TYPE:
            {
                DynamicData result(primitive_type<int32_t>());
                result = -(*this);
                return result;
            }
        case TypeKind::UINT_32_TYPE:
            {
                DynamicData result(primitive_type<uint32_t>());
                result = -(*this);
                return result;
            }
        case TypeKind::INT_64_TYPE:
            {
                DynamicData result(primitive_type<int64_t>());
                result = -(*this);
                return result;
            }
        case TypeKind::UINT_64_TYPE:
            {
                DynamicData result(primitive_type<uint64_t>());
                result = -(*this);
                return result;
            }
        case TypeKind::FLOAT_32_TYPE:
            {
                DynamicData result(primitive_type<float>());
                result = -(*this);
                return result;
            }
        case TypeKind::FLOAT_64_TYPE:
            {
                DynamicData result(primitive_type<double>());
                result = -(*this);
                return result;
            }
        case TypeKind::FLOAT_128_TYPE:
            {
                DynamicData result(primitive_type<long double>());
                result = -(*this);
                return result;
            }
        default:
            break;
    }

    return *this;
}

inline DynamicData DynamicData::operator * (const ReadableDynamicDataRef& other) const
{
    switch(type_.kind())
    {
        case TypeKind::INT_8_TYPE:
            {
                int8_t lho = *this;
                int8_t rho = other;
                DynamicData result(primitive_type<int8_t>());
                result = lho * rho;
                return result;
            }
        case TypeKind::UINT_8_TYPE:
            {
                uint8_t lho = *this;
                uint8_t rho = other;
                DynamicData result(primitive_type<uint8_t>());
                result = lho * rho;
                return result;
            }
        case TypeKind::INT_16_TYPE:
            {
                int16_t lho = *this;
                int16_t rho = other;
                DynamicData result(primitive_type<int16_t>());
                result = lho * rho;
                return result;
            }
        case TypeKind::UINT_16_TYPE:
            {
                uint16_t lho = *this;
                uint16_t rho = other;
                DynamicData result(primitive_type<uint16_t>());
                result = lho * rho;
                return result;
            }
        case TypeKind::INT_32_TYPE:
            {
                int32_t lho = *this;
                int32_t rho = other;
                DynamicData result(primitive_type<int32_t>());
                result = lho * rho;
                return result;
            }
        case TypeKind::UINT_32_TYPE:
            {
                uint32_t lho = *this;
                uint32_t rho = other;
                DynamicData result(primitive_type<uint32_t>());
                result = lho * rho;
                return result;
            }
        case TypeKind::INT_64_TYPE:
            {
                int64_t lho = *this;
                int64_t rho = other;
                DynamicData result(primitive_type<int64_t>());
                result = lho * rho;
                return result;
            }
        case TypeKind::UINT_64_TYPE:
            {
                uint64_t lho = *this;
                uint64_t rho = other;
                DynamicData result(primitive_type<uint64_t>());
                result = lho * rho;
                return result;
            }
        case TypeKind::FLOAT_32_TYPE:
            {
                float lho = *this;
                float rho = other;
                DynamicData result(primitive_type<float>());
                result = lho * rho;
                return result;
            }
        case TypeKind::FLOAT_64_TYPE:
            {
                double lho = *this;
                double rho = other;
                DynamicData result(primitive_type<double>());
                result = lho * rho;
                return result;
            }
        case TypeKind::FLOAT_128_TYPE:
            {
                long double lho = *this;
                long double rho = other;
                DynamicData result(primitive_type<long double>());
                result = lho * rho;
                return result;
            }
        default:
            return *this;
    }
}

inline DynamicData DynamicData::operator / (const ReadableDynamicDataRef& other) const
{
    switch(type_.kind())
    {
        case TypeKind::INT_8_TYPE:
            {
                int8_t lho = *this;
                int8_t rho = other;
                DynamicData result(primitive_type<int8_t>());
                result = lho / rho;
                return result;
            }
        case TypeKind::UINT_8_TYPE:
            {
                uint8_t lho = *this;
                uint8_t rho = other;
                DynamicData result(primitive_type<uint8_t>());
                result = lho / rho;
                return result;
            }
        case TypeKind::INT_16_TYPE:
            {
                int16_t lho = *this;
                int16_t rho = other;
                DynamicData result(primitive_type<int16_t>());
                result = lho / rho;
                return result;
            }
        case TypeKind::UINT_16_TYPE:
            {
                uint16_t lho = *this;
                uint16_t rho = other;
                DynamicData result(primitive_type<uint16_t>());
                result = lho / rho;
                return result;
            }
        case TypeKind::INT_32_TYPE:
            {
                int32_t lho = *this;
                int32_t rho = other;
                DynamicData result(primitive_type<int32_t>());
                result = lho / rho;
                return result;
            }
        case TypeKind::UINT_32_TYPE:
            {
                uint32_t lho = *this;
                uint32_t rho = other;
                DynamicData result(primitive_type<uint32_t>());
                result = lho / rho;
                return result;
            }
        case TypeKind::INT_64_TYPE:
            {
                int64_t lho = *this;
                int64_t rho = other;
                DynamicData result(primitive_type<int64_t>());
                result = lho / rho;
                return result;
            }
        case TypeKind::UINT_64_TYPE:
            {
                uint64_t lho = *this;
                uint64_t rho = other;
                DynamicData result(primitive_type<uint64_t>());
                result = lho / rho;
                return result;
            }
        case TypeKind::FLOAT_32_TYPE:
            {
                float lho = *this;
                float rho = other;
                DynamicData result(primitive_type<float>());
                result = lho / rho;
                return result;
            }
        case TypeKind::FLOAT_64_TYPE:
            {
                double lho = *this;
                double rho = other;
                DynamicData result(primitive_type<double>());
                result = lho / rho;
                return result;
            }
        case TypeKind::FLOAT_128_TYPE:
            {
                long double lho = *this;
                long double rho = other;
                DynamicData result(primitive_type<long double>());
                result = lho / rho;
                return result;
            }
        default:
            return *this;
    }

}

inline DynamicData DynamicData::operator % (const ReadableDynamicDataRef& other) const
{
    switch(type_.kind())
    {
        case TypeKind::INT_8_TYPE:
            {
                int8_t lho = *this;
                int8_t rho = other;
                DynamicData result(primitive_type<int8_t>());
                result = lho % rho;
                return result;
            }
        case TypeKind::UINT_8_TYPE:
            {
                uint8_t lho = *this;
                uint8_t rho = other;
                DynamicData result(primitive_type<uint8_t>());
                result = lho % rho;
                return result;
            }
        case TypeKind::INT_16_TYPE:
            {
                int16_t lho = *this;
                int16_t rho = other;
                DynamicData result(primitive_type<int16_t>());
                result = lho % rho;
                return result;
            }
        case TypeKind::UINT_16_TYPE:
            {
                uint16_t lho = *this;
                uint16_t rho = other;
                DynamicData result(primitive_type<uint16_t>());
                result = lho % rho;
                return result;
            }
        case TypeKind::INT_32_TYPE:
            {
                int32_t lho = *this;
                int32_t rho = other;
                DynamicData result(primitive_type<int32_t>());
                result = lho % rho;
                return result;
            }
        case TypeKind::UINT_32_TYPE:
            {
                uint32_t lho = *this;
                uint32_t rho = other;
                DynamicData result(primitive_type<uint32_t>());
                result = lho % rho;
                return result;
            }
        case TypeKind::INT_64_TYPE:
            {
                int64_t lho = *this;
                int64_t rho = other;
                DynamicData result(primitive_type<int64_t>());
                result = lho % rho;
                return result;
            }
        case TypeKind::UINT_64_TYPE:
            {
                uint64_t lho = *this;
                uint64_t rho = other;
                DynamicData result(primitive_type<uint64_t>());
                result = lho % rho;
                return result;
            }
        default:
            return *this;
    }

}

inline DynamicData DynamicData::operator + (const ReadableDynamicDataRef& other) const
{
    switch(type_.kind())
    {
        case TypeKind::INT_8_TYPE:
            {
                int8_t lho = *this;
                int8_t rho = other;
                DynamicData result(primitive_type<int8_t>());
                result = lho + rho;
                return result;
            }
        case TypeKind::UINT_8_TYPE:
            {
                uint8_t lho = *this;
                uint8_t rho = other;
                DynamicData result(primitive_type<uint8_t>());
                result = lho + rho;
                return result;
            }
        case TypeKind::INT_16_TYPE:
            {
                int16_t lho = *this;
                int16_t rho = other;
                DynamicData result(primitive_type<int16_t>());
                result = lho + rho;
                return result;
            }
        case TypeKind::UINT_16_TYPE:
            {
                uint16_t lho = *this;
                uint16_t rho = other;
                DynamicData result(primitive_type<uint16_t>());
                result = lho + rho;
                return result;
            }
        case TypeKind::INT_32_TYPE:
            {
                int32_t lho = *this;
                int32_t rho = other;
                DynamicData result(primitive_type<int32_t>());
                result = lho + rho;
                return result;
            }
        case TypeKind::UINT_32_TYPE:
            {
                uint32_t lho = *this;
                uint32_t rho = other;
                DynamicData result(primitive_type<uint32_t>());
                result = lho + rho;
                return result;
            }
        case TypeKind::INT_64_TYPE:
            {
                int64_t lho = *this;
                int64_t rho = other;
                DynamicData result(primitive_type<int64_t>());
                result = lho + rho;
                return result;
            }
        case TypeKind::UINT_64_TYPE:
            {
                uint64_t lho = *this;
                uint64_t rho = other;
                DynamicData result(primitive_type<uint64_t>());
                result = lho + rho;
                return result;
            }
        case TypeKind::FLOAT_32_TYPE:
            {
                float lho = *this;
                float rho = other;
                DynamicData result(primitive_type<float>());
                result = lho + rho;
                return result;
            }
        case TypeKind::FLOAT_64_TYPE:
            {
                double lho = *this;
                double rho = other;
                DynamicData result(primitive_type<double>());
                result = lho + rho;
                return result;
            }
        case TypeKind::FLOAT_128_TYPE:
            {
                long double lho = *this;
                long double rho = other;
                DynamicData result(primitive_type<long double>());
                result = lho + rho;
                return result;
            }
        default:
            return *this;
    }

}

inline DynamicData DynamicData::operator - (const ReadableDynamicDataRef& other) const
{
    switch(type_.kind())
    {
        case TypeKind::INT_8_TYPE:
            {
                int8_t lho = *this;
                int8_t rho = other;
                DynamicData result(primitive_type<int8_t>());
                result = lho - rho;
                return result;
            }
        case TypeKind::UINT_8_TYPE:
            {
                uint8_t lho = *this;
                uint8_t rho = other;
                DynamicData result(primitive_type<uint8_t>());
                result = lho - rho;
                return result;
            }
        case TypeKind::INT_16_TYPE:
            {
                int16_t lho = *this;
                int16_t rho = other;
                DynamicData result(primitive_type<int16_t>());
                result = lho - rho;
                return result;
            }
        case TypeKind::UINT_16_TYPE:
            {
                uint16_t lho = *this;
                uint16_t rho = other;
                DynamicData result(primitive_type<uint16_t>());
                result = lho - rho;
                return result;
            }
        case TypeKind::INT_32_TYPE:
            {
                int32_t lho = *this;
                int32_t rho = other;
                DynamicData result(primitive_type<int32_t>());
                result = lho - rho;
                return result;
            }
        case TypeKind::UINT_32_TYPE:
            {
                uint32_t lho = *this;
                uint32_t rho = other;
                DynamicData result(primitive_type<uint32_t>());
                result = lho - rho;
                return result;
            }
        case TypeKind::INT_64_TYPE:
            {
                int64_t lho = *this;
                int64_t rho = other;
                DynamicData result(primitive_type<int64_t>());
                result = lho - rho;
                return result;
            }
        case TypeKind::UINT_64_TYPE:
            {
                uint64_t lho = *this;
                uint64_t rho = other;
                DynamicData result(primitive_type<uint64_t>());
                result = lho - rho;
                return result;
            }
        case TypeKind::FLOAT_32_TYPE:
            {
                float lho = *this;
                float rho = other;
                DynamicData result(primitive_type<float>());
                result = lho - rho;
                return result;
            }
        case TypeKind::FLOAT_64_TYPE:
            {
                double lho = *this;
                double rho = other;
                DynamicData result(primitive_type<double>());
                result = lho - rho;
                return result;
            }
        case TypeKind::FLOAT_128_TYPE:
            {
                long double lho = *this;
                long double rho = other;
                DynamicData result(primitive_type<long double>());
                result = lho - rho;
                return result;
            }
        default:
            return *this;
    }

}

inline DynamicData DynamicData::operator << (const ReadableDynamicDataRef& other) const
{
    switch(type_.kind())
    {
        case TypeKind::INT_8_TYPE:
            {
                int8_t lho = *this;
                int8_t rho = other;
                DynamicData result(primitive_type<int8_t>());
                result = lho << rho;
                return result;
            }
        case TypeKind::UINT_8_TYPE:
            {
                uint8_t lho = *this;
                uint8_t rho = other;
                DynamicData result(primitive_type<uint8_t>());
                result = lho << rho;
                return result;
            }
        case TypeKind::INT_16_TYPE:
            {
                int16_t lho = *this;
                int16_t rho = other;
                DynamicData result(primitive_type<int16_t>());
                result = lho << rho;
                return result;
            }
        case TypeKind::UINT_16_TYPE:
            {
                uint16_t lho = *this;
                uint16_t rho = other;
                DynamicData result(primitive_type<uint16_t>());
                result = lho << rho;
                return result;
            }
        case TypeKind::INT_32_TYPE:
            {
                int32_t lho = *this;
                int32_t rho = other;
                DynamicData result(primitive_type<int32_t>());
                result = lho << rho;
                return result;
            }
        case TypeKind::UINT_32_TYPE:
            {
                uint32_t lho = *this;
                uint32_t rho = other;
                DynamicData result(primitive_type<uint32_t>());
                result = lho << rho;
                return result;
            }
        case TypeKind::INT_64_TYPE:
            {
                int64_t lho = *this;
                int64_t rho = other;
                DynamicData result(primitive_type<int64_t>());
                result = lho << rho;
                return result;
            }
        case TypeKind::UINT_64_TYPE:
            {
                uint64_t lho = *this;
                uint64_t rho = other;
                DynamicData result(primitive_type<uint64_t>());
                result = lho << rho;
                return result;
            }
        default:
            return *this;
    }

}

inline DynamicData DynamicData::operator >> (const ReadableDynamicDataRef& other) const
{
    switch(type_.kind())
    {
        case TypeKind::INT_8_TYPE:
            {
                int8_t lho = *this;
                int8_t rho = other;
                DynamicData result(primitive_type<int8_t>());
                result = lho >> rho;
                return result;
            }
        case TypeKind::UINT_8_TYPE:
            {
                uint8_t lho = *this;
                uint8_t rho = other;
                DynamicData result(primitive_type<uint8_t>());
                result = lho >> rho;
                return result;
            }
        case TypeKind::INT_16_TYPE:
            {
                int16_t lho = *this;
                int16_t rho = other;
                DynamicData result(primitive_type<int16_t>());
                result = lho >> rho;
                return result;
            }
        case TypeKind::UINT_16_TYPE:
            {
                uint16_t lho = *this;
                uint16_t rho = other;
                DynamicData result(primitive_type<uint16_t>());
                result = lho >> rho;
                return result;
            }
        case TypeKind::INT_32_TYPE:
            {
                int32_t lho = *this;
                int32_t rho = other;
                DynamicData result(primitive_type<int32_t>());
                result = lho >> rho;
                return result;
            }
        case TypeKind::UINT_32_TYPE:
            {
                uint32_t lho = *this;
                uint32_t rho = other;
                DynamicData result(primitive_type<uint32_t>());
                result = lho >> rho;
                return result;
            }
        case TypeKind::INT_64_TYPE:
            {
                int64_t lho = *this;
                int64_t rho = other;
                DynamicData result(primitive_type<int64_t>());
                result = lho >> rho;
                return result;
            }
        case TypeKind::UINT_64_TYPE:
            {
                uint64_t lho = *this;
                uint64_t rho = other;
                DynamicData result(primitive_type<uint64_t>());
                result = lho >> rho;
                return result;
            }
        default:
            return *this;
    }

}

inline DynamicData DynamicData::operator & (const ReadableDynamicDataRef& other) const
{
    switch(type_.kind())
    {
        case TypeKind::INT_8_TYPE:
            {
                int8_t lho = *this;
                int8_t rho = other;
                DynamicData result(primitive_type<int8_t>());
                result = lho & rho;
                return result;
            }
        case TypeKind::UINT_8_TYPE:
            {
                uint8_t lho = *this;
                uint8_t rho = other;
                DynamicData result(primitive_type<uint8_t>());
                result = lho & rho;
                return result;
            }
        case TypeKind::INT_16_TYPE:
            {
                int16_t lho = *this;
                int16_t rho = other;
                DynamicData result(primitive_type<int16_t>());
                result = lho & rho;
                return result;
            }
        case TypeKind::UINT_16_TYPE:
            {
                uint16_t lho = *this;
                uint16_t rho = other;
                DynamicData result(primitive_type<uint16_t>());
                result = lho & rho;
                return result;
            }
        case TypeKind::INT_32_TYPE:
            {
                int32_t lho = *this;
                int32_t rho = other;
                DynamicData result(primitive_type<int32_t>());
                result = lho & rho;
                return result;
            }
        case TypeKind::UINT_32_TYPE:
            {
                uint32_t lho = *this;
                uint32_t rho = other;
                DynamicData result(primitive_type<uint32_t>());
                result = lho & rho;
                return result;
            }
        case TypeKind::INT_64_TYPE:
            {
                int64_t lho = *this;
                int64_t rho = other;
                DynamicData result(primitive_type<int64_t>());
                result = lho & rho;
                return result;
            }
        case TypeKind::UINT_64_TYPE:
            {
                uint64_t lho = *this;
                uint64_t rho = other;
                DynamicData result(primitive_type<uint64_t>());
                result = lho & rho;
                return result;
            }
        default:
            return *this;
    }

}

inline DynamicData DynamicData::operator ^ (const ReadableDynamicDataRef& other) const
{
    switch(type_.kind())
    {
        case TypeKind::INT_8_TYPE:
            {
                int8_t lho = *this;
                int8_t rho = other;
                DynamicData result(primitive_type<int8_t>());
                result = lho ^ rho;
                return result;
            }
        case TypeKind::UINT_8_TYPE:
            {
                uint8_t lho = *this;
                uint8_t rho = other;
                DynamicData result(primitive_type<uint8_t>());
                result = lho ^ rho;
                return result;
            }
        case TypeKind::INT_16_TYPE:
            {
                int16_t lho = *this;
                int16_t rho = other;
                DynamicData result(primitive_type<int16_t>());
                result = lho ^ rho;
                return result;
            }
        case TypeKind::UINT_16_TYPE:
            {
                uint16_t lho = *this;
                uint16_t rho = other;
                DynamicData result(primitive_type<uint16_t>());
                result = lho ^ rho;
                return result;
            }
        case TypeKind::INT_32_TYPE:
            {
                int32_t lho = *this;
                int32_t rho = other;
                DynamicData result(primitive_type<int32_t>());
                result = lho ^ rho;
                return result;
            }
        case TypeKind::UINT_32_TYPE:
            {
                uint32_t lho = *this;
                uint32_t rho = other;
                DynamicData result(primitive_type<uint32_t>());
                result = lho ^ rho;
                return result;
            }
        case TypeKind::INT_64_TYPE:
            {
                int64_t lho = *this;
                int64_t rho = other;
                DynamicData result(primitive_type<int64_t>());
                result = lho ^ rho;
                return result;
            }
        case TypeKind::UINT_64_TYPE:
            {
                uint64_t lho = *this;
                uint64_t rho = other;
                DynamicData result(primitive_type<uint64_t>());
                result = lho ^ rho;
                return result;
            }
        default:
            return *this;
    }

}

inline DynamicData DynamicData::operator | (const ReadableDynamicDataRef& other) const
{
    switch(type_.kind())
    {
        case TypeKind::INT_8_TYPE:
            {
                //int8_t lho = value<int8_t>();
                int8_t lho = *this;
                int8_t rho = other;
                DynamicData result(primitive_type<int8_t>());
                result = lho | rho;
                return result;
            }
        case TypeKind::UINT_8_TYPE:
            {
                uint8_t lho = *this;
                uint8_t rho = other;
                DynamicData result(primitive_type<uint8_t>());
                result = lho | rho;
                return result;
            }
        case TypeKind::INT_16_TYPE:
            {
                int16_t lho = *this;
                int16_t rho = other;
                DynamicData result(primitive_type<int16_t>());
                result = lho | rho;
                return result;
            }
        case TypeKind::UINT_16_TYPE:
            {
                uint16_t lho = *this;
                uint16_t rho = other;
                DynamicData result(primitive_type<uint16_t>());
                result = lho | rho;
                return result;
            }
        case TypeKind::INT_32_TYPE:
            {
                int32_t lho = *this;
                int32_t rho = other;
                DynamicData result(primitive_type<int32_t>());
                result = lho | rho;
                return result;
            }
        case TypeKind::UINT_32_TYPE:
            {
                uint32_t lho = *this;
                uint32_t rho = other;
                DynamicData result(primitive_type<uint32_t>());
                result = lho | rho;
                return result;
            }
        case TypeKind::INT_64_TYPE:
            {
                int64_t lho = *this;
                int64_t rho = other;
                DynamicData result(primitive_type<int64_t>());
                result = lho | rho;
                return result;
            }
        case TypeKind::UINT_64_TYPE:
            {
                uint64_t lho = *this;
                uint64_t rho = other;
                DynamicData result(primitive_type<uint64_t>());
                result = lho | rho;
                return result;
            }
        default:
            return *this;
    }

}

} //namespace xtypes
} //namespace eprosima

#endif //EPROSIMA_XTYPES_DYNAMIC_DATA_IMPL_HPP_
