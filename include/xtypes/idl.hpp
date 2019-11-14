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
 *
*/

#ifndef EPROSIMA_XTYPES_IDL_HPP_
#define EPROSIMA_XTYPES_IDL_HPP_

#include <xtypes/StructType.hpp>

#include <sstream>

namespace eprosima {
namespace xtypes {
namespace idl {

inline std::map<std::string, DynamicType::Ptr> parse(const std::string& /*idl*/)
{
    return std::map<std::string, DynamicType::Ptr>();
}


inline std::string from(const StructType& type)
{
    std::stringstream ss;
    for(const Member& member: type.members())
    {
        switch(member.type().kind())
        {
            case TypeKind::BOOLEAN_TYPE:
                ss << "boolean " << member.name() << ";";
                break;
            case TypeKind::CHAR_8_TYPE:
                ss << "char " << member.name() << ";";
                break;
            case TypeKind::CHAR_16_TYPE:
                ss << "wchar " << member.name() << ";";
                break;
            case TypeKind::INT_8_TYPE:
                ss << "int8 " << member.name() << ";";
                break;
            case TypeKind::UINT_8_TYPE:
                ss << "uint8 " << member.name() << ";";
                break;
            case TypeKind::INT_16_TYPE:
                ss << "short " << member.name() << ";";
                break;
            case TypeKind::UINT_16_TYPE:
                ss << "unsigned short " << member.name() << ";";
                break;
            case TypeKind::INT_32_TYPE:
                ss << "long " << member.name() << ";";
                break;
            case TypeKind::UINT_32_TYPE:
                ss << "unsigned long " << member.name() << ";";
                break;
            case TypeKind::INT_64_TYPE:
                ss << "long long " << member.name() << ";";
                break;
            case TypeKind::UINT_64_TYPE:
                ss << "unsigned long long " << member.name() << ";";
                break;
            case TypeKind::FLOAT_32_TYPE:
                ss << "float " << member.name() << ";";
                break;
            case TypeKind::FLOAT_64_TYPE:
                ss << "double " << member.name() << ";";
                break;
            case TypeKind::FLOAT_128_TYPE:
                ss << "long double " << member.name() << ";";
                break;
            case TypeKind::STRING_TYPE:
                ss << "string " << member.name() << ";"; //TODO
                break;
            case TypeKind::WSTRING_TYPE:
                ss << "wstring " << member.name() << ";"; //TODO
                break;
            case TypeKind::ARRAY_TYPE:
                //TODO
                break;
            case TypeKind::SEQUENCE_TYPE:
                ss << "sequence " << member.name() << ";"; //TODO
                break;
            case TypeKind::STRUCTURE_TYPE:
                ss << member.type().name() << " " << member.name() << ";";
                break;
            default:
                ss << "<<Unsupported type: " << member.type().name() << ">>";
        }
        ss << std::endl;
    }
    return ss.str();
}

} //namespace idl
} //namespace xtypes
} //namespace eprosima

#endif //EPROSIMA_XTYPES_IDL_HPP_
