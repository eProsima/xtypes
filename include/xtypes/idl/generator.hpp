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

#ifndef EPROSIMA_XTYPES_IDL_GENERATOR_HPP_
#define EPROSIMA_XTYPES_IDL_GENERATOR_HPP_

#include <xtypes/StructType.hpp>
#include <xtypes/ArrayType.hpp>
#include <xtypes/MutableCollectionType.hpp>
#include <xtypes/SequenceType.hpp>

#include <sstream>

namespace eprosima {
namespace xtypes {
namespace idl {
namespace generator {

inline std::string type_name(const DynamicType& type); //implementation below

inline std::string sequence_type_name(const DynamicType& type)
{
    assert(type.kind() == TypeKind::SEQUENCE_TYPE);
    const SequenceType& sequence_type = static_cast<const SequenceType&>(type);
    std::stringstream ss;
    ss << "sequence<";
    ss << type_name(sequence_type.content_type());
    size_t bounds = sequence_type.bounds();
    ss << (bounds ? ", " + std::to_string(bounds) : "");
    ss << ">";
    return ss.str();
}

inline std::string array_member(const Member& member)
{
    assert(member.type().kind() == TypeKind::ARRAY_TYPE);
    const DynamicType* type = &member.type();
    std::stringstream dimensions;
    do
    {
        const ArrayType& array_type = static_cast<const ArrayType&>(*type);
        dimensions << "[" << array_type.dimension() << "]";
        type = &array_type.content_type();
    }
    while(type->kind() == TypeKind::ARRAY_TYPE);

    std::stringstream ss;
    ss << type_name(*type) << " " << member.name() << dimensions.str() << ";";
    return ss.str();
}

inline std::string type_name(const DynamicType& type)
{
    static const std::map<TypeKind, std::string> mapping =
    {
        { TypeKind::BOOLEAN_TYPE, "boolean" },
        { TypeKind::CHAR_8_TYPE, "char" },
        { TypeKind::CHAR_16_TYPE, "wchar" },
        { TypeKind::INT_8_TYPE, "int8" },
        { TypeKind::UINT_8_TYPE, "uint8" },
        { TypeKind::INT_16_TYPE, "short" },
        { TypeKind::UINT_16_TYPE, "unsigned short" },
        { TypeKind::INT_32_TYPE, "long" },
        { TypeKind::UINT_32_TYPE, "unsigned long" },
        { TypeKind::INT_64_TYPE, "long long" },
        { TypeKind::UINT_64_TYPE, "unsigned long long" },
        { TypeKind::FLOAT_32_TYPE, "float" },
        { TypeKind::FLOAT_64_TYPE, "double" },
        { TypeKind::FLOAT_128_TYPE, "long double" },
    };

    if(type.is_primitive_type())
    {
        return mapping.at(type.kind());
    }
    else if(type.is_aggregation_type())
    {
        return type.name();
    }
    else if(type.kind() == TypeKind::SEQUENCE_TYPE)
    {
        return sequence_type_name(type);
    }
    else if(type.kind() == TypeKind::STRING_TYPE || type.kind() == TypeKind::WSTRING_TYPE)
    {
        const MutableCollectionType& collection_type = static_cast<const MutableCollectionType&>(type);
        std::string type_name = type.kind() == TypeKind::STRING_TYPE ? "string" : "wstring";
        size_t bounds = collection_type.bounds();
        return type_name + (bounds > 0 ? "<" + std::to_string(bounds) + ">" : "");
    }
    else
    {
        return "[[unsupported]]";
    }
}

} //namespace generator
} //namespace idl
} //namespace xtypes
} //namespace eprosima

#endif //EPROSIMA_XTYPES_IDL_GENERATOR_HPP_
