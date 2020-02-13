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
#include <xtypes/EnumerationType.hpp>
#include <xtypes/AliasType.hpp>

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

inline std::string map_type_name(const DynamicType& type)
{
    assert(type.kind() == TypeKind::MAP_TYPE);
    const MapType& map_type = static_cast<const MapType&>(type);
    const PairType& map_content = static_cast<const PairType&>(map_type.content_type());
    size_t bounds = map_type.bounds();
    std::stringstream ss;
    ss << "map<";
    ss << type_name(map_content.first()) << ", " << type_name(map_content.second());
    ss << (bounds ? ", " + std::to_string(bounds) : "");
    ss << ">";
    return ss.str();
}

inline std::vector<uint32_t> array_dimensions(const ArrayType& array)
{
    std::vector<uint32_t> dimensions;
    dimensions.push_back(array.dimension());
    DynamicType::Ptr inner(array.content_type());
    while (inner->kind() == TypeKind::ARRAY_TYPE)
    {
        const ArrayType& inner_array = static_cast<const ArrayType&>(*inner);
        dimensions.push_back(inner_array.dimension());
        inner = inner_array.content_type();
    }
    return dimensions;
}

inline std::string array_member(const Member& member)
{
    assert(member.type().kind() == TypeKind::ARRAY_TYPE);
    const DynamicType* type = &member.type();
    std::stringstream dimensions;

    std::vector<uint32_t> array_dims = array_dimensions(static_cast<const ArrayType&>(*type));
    for (uint32_t dimension : array_dims)
    {
        dimensions << "[" << dimension << "]";
    }

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
        { TypeKind::INT_16_TYPE, "int16" },
        { TypeKind::UINT_16_TYPE, "uint16" },
        { TypeKind::INT_32_TYPE, "int32" },
        { TypeKind::UINT_32_TYPE, "uint32" },
        { TypeKind::INT_64_TYPE, "int64" },
        { TypeKind::UINT_64_TYPE, "uint64" },
        { TypeKind::FLOAT_32_TYPE, "float" },
        { TypeKind::FLOAT_64_TYPE, "double" },
        { TypeKind::FLOAT_128_TYPE, "long double" },
    };

    if(type.kind() == TypeKind::ALIAS_TYPE)
    {
        return static_cast<const AliasType&>(type).name();
    }
    if(type.is_primitive_type())
    {
        return mapping.at(type.kind());
    }
    else if(type.is_aggregation_type())
    {
        return type.name();
    }
    else if(type.kind() == TypeKind::ARRAY_TYPE)
    {
        return type_name(static_cast<const ArrayType&>(type).content_type());
    }
    else if(type.kind() == TypeKind::SEQUENCE_TYPE)
    {
        return sequence_type_name(type);
    }
    else if(type.kind() == TypeKind::MAP_TYPE)
    {
        return map_type_name(type);
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
        return type.name();
    }
}

inline std::string label_value(size_t value, const DynamicType& type)
{
    TypeKind kind = type.kind();

    if (kind == TypeKind::ALIAS_TYPE)
    {
        kind = static_cast<const AliasType&>(type).rget().kind();
    }

    switch (kind)
    {
        case TypeKind::BOOLEAN_TYPE:
        {
            return (value == 0) ? "FALSE" : "TRUE";
        }
        case TypeKind::CHAR_8_TYPE:
        {
            char temp = static_cast<char>(value);
            std::stringstream ss;
            ss << "'" << temp << "'";
            return ss.str();
        }
        case TypeKind::CHAR_16_TYPE:
        {
            wchar_t temp = static_cast<wchar_t>(value);
            std::stringstream ss;
            ss << "L'" << temp << "'";
            return ss.str();
        }
        case TypeKind::INT_8_TYPE:
        case TypeKind::UINT_8_TYPE:
        case TypeKind::INT_16_TYPE:
        case TypeKind::UINT_16_TYPE:
        case TypeKind::INT_32_TYPE:
        case TypeKind::UINT_32_TYPE:
        case TypeKind::INT_64_TYPE:
        case TypeKind::UINT_64_TYPE:
        {
            return std::to_string(value);
        }
        case TypeKind::ENUMERATION_TYPE:
        {
            return static_cast<const EnumerationType<uint32_t>&>(type).enumerator(static_cast<uint32_t>(value));
        }
        default:
            xtypes_assert(false, "Unsupported type found while generating label value: " << type.name());
            return std::to_string(value);
    }
}

inline size_t inherit_members(const AggregationType& type)
{
    if (type.has_parent())
    {
        return type.parent().members().size();
    }
    return 0;
}

inline std::string structure(const StructType& type, size_t tabs = 0)
{
    std::stringstream ss;
    ss << std::string(tabs * 4, ' ') << "struct " << type.name();
    if (type.has_parent())
    {
        ss << " : " << type.parent().name();
    }
    ss << std::endl << std::string(tabs * 4, ' ') << "{" << std::endl;

    for(size_t idx = inherit_members(type); idx < type.members().size(); ++idx)
    {
        const Member& member = type.member(idx);
        ss << std::string((tabs + 1) * 4, ' ');
        if(member.type().kind() == TypeKind::ARRAY_TYPE)
        {
            ss << generator::array_member(member); //Spetial member syntax
        }
        else
        {
            ss << generator::type_name(member.type()) << " " << member.name() << ";";
        }
        ss << std::endl;
    }
    ss << std::string(tabs * 4, ' ') << "};" << std::endl;
    return ss.str();
}

inline std::string generate_union(const UnionType& type, size_t tabs = 0)
{
    std::stringstream ss;
    ss << std::string(tabs * 4, ' ') << "union " << type.name()
       << " switch (" << generator::type_name(type.discriminator()) << ")" << std::endl;

    ss << std::string(tabs * 4, ' ') << "{" << std::endl;

    std::vector<std::string> members = type.get_case_members();
    for (const std::string& name : members)
    {
        std::vector<int64_t> labels = type.get_labels(name);
        for (int64_t value : labels)
        {
            ss << std::string((tabs + 1) * 4, ' ');
            ss << "case " << generator::label_value(value, type.discriminator()) << ":" << std::endl;
        }
        if (type.is_default(name)) // Add default "label"
        {
            ss << std::string((tabs + 1) * 4, ' ');
            ss << "default:" << std::endl;
        }
        const Member& member = type.member(name);
        ss << std::string((tabs + 2) * 4, ' ');
        if(member.type().kind() == TypeKind::ARRAY_TYPE)
        {
            ss << generator::array_member(member); //Spetial member syntax
        }
        else
        {
            ss << generator::type_name(member.type()) << " " << member.name() << ";";
        }
        ss << std::endl;
    }
    ss << std::string(tabs * 4, ' ') << "};" << std::endl;
    return ss.str();
}

inline std::string aliase(const DynamicType& type, const std::string& name)
{
    std::stringstream ss;
    ss << "typedef " << generator::type_name(type) << " " << name;
    if (type.kind() == TypeKind::ARRAY_TYPE)
    {
        std::vector<uint32_t> array_dims = array_dimensions(static_cast<const ArrayType&>(type));
        for (uint32_t dimension : array_dims)
        {
            ss << "[" << dimension << "]";
        }
    }
    ss << ";" << std::endl;
    return ss.str();
}

inline std::string enumeration32(const EnumerationType<uint32_t>& enumeration, size_t tabs = 0)
{
    std::stringstream ss;
    // We must add them in order
    using map_pair = std::pair<std::string, uint32_t>;
    std::map<std::string, uint32_t> enumerators = enumeration.enumerators();
    ss << std::string(tabs * 4, ' ') << "enum " << enumeration.name() << std::endl;
    ss << std::string(tabs * 4, ' ') << "{" << std::endl;
    // Copy to a vector
    std::vector<map_pair> ids(enumerators.begin(), enumerators.end());
    // Sort the vector
    std::sort(ids.begin(), ids.end(),
        [](const map_pair& a, const map_pair& b)
              {
                  return a.second < b.second;
              });
    // Print the ordered values
    for (size_t i = 0; i < ids.size(); ++i)
    {
        const auto& value = ids[i];
        ss << std::string((tabs + 1) * 4, ' ') << value.first;
        if (i + 1 < ids.size())
        {
            ss << ",";
        }
        ss << std::endl;
    }
    ss << std::string(tabs * 4, ' ') << "};" << std::endl;
    return ss.str();
}

inline std::string get_const_value(ReadableDynamicDataRef data)
{
    std::stringstream ss;
    std::string prefix = "";
    std::string suffix = "";

    if (data.type().kind() == TypeKind::STRING_TYPE)
    {
        prefix = "\"";
        suffix = "\"";
    }
    else if (data.type().kind() == TypeKind::WSTRING_TYPE)
    {
        prefix = "L\"";
        suffix = "\"";
    }
    else if (data.type().kind() == TypeKind::CHAR_8_TYPE)
    {
        prefix = "'";
        suffix = "'";
    }
    else if (data.type().kind() == TypeKind::CHAR_16_TYPE)
    {
        prefix = "L'";
        suffix = "'";
    }

    ss << prefix << data.cast<std::string>() << suffix;

    return ss.str();
}

// TODO: module_contents (and maybe module) should generate a dependency tree and resolve them in the generated IDL,
// including maybe the need of forward declarations.
inline std::string module_contents(const Module& module_, size_t tabs = 0)
{
    std::stringstream ss;

    // Aliases
    for (const auto& alias : module_.aliases_)
    {
        ss << std::string(tabs * 4, ' ') << aliase(static_cast<const AliasType&>(*alias.second).get(), alias.first);
    }
    // Enums
    for (const auto& pair : module_.enumerations_32_)
    {
        const EnumerationType<uint32_t>& enum_type = static_cast<const EnumerationType<uint32_t>&>(*pair.second);
        ss << enumeration32(enum_type, tabs);
    }
    // Consts
    for (const auto& pair : module_.constants_)
    {
        if (!module_.is_const_from_enum(pair.first)) // Don't add as const the "fake" enumeration consts.
        {
            ss << std::string(tabs * 4, ' ') << "const " << type_name(pair.second.type()) << " " << pair.first
               << " = " << get_const_value(pair.second) << ";" << std::endl;
        }
    }
    // Unions
    for (const auto& pair : module_.unions_)
    {
        const UnionType& union_type = static_cast<const UnionType&>(*pair.second);
        ss << generate_union(union_type, tabs);
    }
    // Structs
    for (const auto& pair : module_.structs_)
    {
        const StructType& struct_type = static_cast<const StructType&>(*pair.second);
        ss << structure(struct_type, tabs);
    }
    // Submodules
    for (const auto& pair : module_.inner_)
    {
        const Module& inner_module = *pair.second;
        ss << std::string(tabs * 4, ' ') << "module " << inner_module.name() << std::endl;
        ss << std::string(tabs * 4, ' ') << "{" << std::endl;
        ss << module_contents(inner_module, tabs + 1);
        ss << std::string(tabs * 4, ' ') << "};" << std::endl;
    }

    return ss.str();
}

inline std::string module(const Module& module, size_t tabs = 0)
{
    std::stringstream ss;

    // Check if it is root
    if (module.name().empty())
    {
        ss << module_contents(module);
    }
    else
    {
        ss << std::string(tabs * 4, ' ') << "module " << module.name() << std::endl;
        ss << std::string(tabs * 4, ' ') << "{" << std::endl;
        ss << module_contents(module, tabs + 1);
        ss << std::string(tabs * 4, ' ') << "};" << std::endl;
    }

    return ss.str();
}

} //namespace generator
} //namespace idl
} //namespace xtypes
} //namespace eprosima

#endif //EPROSIMA_XTYPES_IDL_GENERATOR_HPP_
