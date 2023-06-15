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

#include <xtypes/idl/generator_deptree.hpp>
#include <xtypes/idl/Module.hpp>

#include <sstream>

#define dependencynode_assert(NODEPTR, KIND) \
{\
    xtypes_assert(NODEPTR->kind() == dependencytree::ModuleElementKind::KIND,\
        "Trying to generate an IDL " << #KIND << " definition from a non-" << #KIND\
        << " node (" << NODEPTR->name() << ").");\
}

namespace eprosima {
namespace xtypes {
namespace idl {
namespace generator {

inline std::string sequence_type_name(
        dependencytree::DependencyNode* node,
        const DynamicType& type)
{
    assert(type.kind() == TypeKind::SEQUENCE_TYPE);
    const SequenceType& sequence_type = static_cast<const SequenceType&>(type);
    std::stringstream ss;
    ss << "sequence<";
    ss << type_name(node, sequence_type.content_type());
    size_t bounds = sequence_type.bounds();
    ss << (bounds ? ", " + std::to_string(bounds) : "");
    ss << ">";
    return ss.str();
}

inline std::string map_type_name(
        dependencytree::DependencyNode* node,
        const DynamicType& type)
{
    assert(type.kind() == TypeKind::MAP_TYPE);
    const MapType& map_type = static_cast<const MapType&>(type);
    const PairType& map_content = static_cast<const PairType&>(map_type.content_type());
    size_t bounds = map_type.bounds();
    std::stringstream ss;
    ss << "map<";
    ss << type_name(node, map_content.first()) << ", " << type_name(node, map_content.second());
    ss << (bounds ? ", " + std::to_string(bounds) : "");
    ss << ">";
    return ss.str();
}

inline std::vector<uint32_t> array_dimensions(
        const ArrayType& array)
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

inline std::string array_member(
        dependencytree::DependencyNode* node,
        const Member& member)
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
    ss << type_name(node, *type) << " " << member.name() << dimensions.str() << ";";
    return ss.str();
}

inline std::string type_scope(
        dependencytree::DependencyNode* node,
        const DynamicType& type)
{
    if (type.name().find("::") != std::string::npos) // Type name is already scoped
    {
        return std::string();
    }
    if (node == nullptr)
    {
        return std::string();
    }
    using namespace dependencytree;
    DependencyModule* from = node->from();

    if (from->opts_for_dependency_setting(type) && !from->module().has_symbol(type.name(), false)
        && from->module().symbol_count(type.name()) != 1)
    {
        const DependencyModule* dep_mod = from->search_module_with_node(type.name());
        return from->relative_scope(dep_mod);
    }
    return std::string();
}

inline std::string type_name(
        dependencytree::DependencyNode* node,
        const DynamicType& type,
        bool scoped)
{
    static const std::map<TypeKind, std::string> mapping =
    {
        { TypeKind::BOOLEAN_TYPE, "boolean" },
        { TypeKind::CHAR_8_TYPE, "char" },
        { TypeKind::CHAR_16_TYPE, "wchar" },
        { TypeKind::WIDE_CHAR_TYPE, "wchar" },
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

    if(type.is_primitive_type())
    {
        return mapping.at(type.kind());
    }
    else if (type.kind() == TypeKind::ARRAY_TYPE)
    {
        return type_name(node, static_cast<const ArrayType&>(type).content_type());
    }
    else if (type.kind() == TypeKind::SEQUENCE_TYPE)
    {
        return sequence_type_name(node, type);
    }
    else if (type.kind() == TypeKind::MAP_TYPE)
    {
        return map_type_name(node, type);
    }
    else if (
            type.kind() == TypeKind::STRING_TYPE ||
            type.kind() == TypeKind::WSTRING_TYPE ||
            type.kind() == TypeKind::STRING16_TYPE)
    {
        const MutableCollectionType& collection_type = static_cast<const MutableCollectionType&>(type);
        std::string type_name = type.kind() == TypeKind::STRING_TYPE ? "string" : "wstring";
        size_t bounds = collection_type.bounds();
        return type_name + (bounds > 0 ? "<" + std::to_string(bounds) + ">" : "");
    }
    else
    {
        std::stringstream ss;
        ss << generator::type_scope(node, type);

        if (type.kind() == TypeKind::ALIAS_TYPE)
        {
            ss << static_cast<const AliasType&>(type).name();
        }
        else
        {
            const std::string& type_name = type.name();
            if (type_name.find("::") != std::string::npos)
            {
                size_t scope_end = type_name.rfind("::");
                std::string scope = type_name.substr(0, scope_end);
                if (node && (scope == node->module().scope()) && !scoped) // Redundant scope: get rid of it
                {
                    ss << type_name.substr(scope_end + 2);
                }
                else
                {
                    ss << type_name;
                }
            }
            else
            {
                ss << type_name;
            }
        }
        return ss.str();
    }
}

inline std::string label_value(
        size_t value,
        const DynamicType& type)
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
        case TypeKind::WIDE_CHAR_TYPE:
        {
            char16_t temp = static_cast<char16_t>(value);
            std::stringstream ss;
            auto aux = code_conversion_tool<XTYPES_CHAR>(std::u16string(1, temp));
            ss << "L'" << std::string(aux.begin(), aux.end()) << "'";
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

inline size_t inherit_members(
        const AggregationType& type)
{
    if (type.has_parent())
    {
        return type.parent().members().size();
    }
    return 0;
}

inline std::string structure(
        const std::string& name,
        const StructType& type,
        dependencytree::DependencyNode* struct_node,
        size_t tabs,
        std::map<std::string, std::string>* struct_idl)
{
    if (struct_node != nullptr)
    {
        dependencynode_assert(struct_node, xSTRUCT);
    }

    std::stringstream ss;
    ss << std::string(tabs * 4, ' ') << "struct " << name;
    if (type.has_parent())
    {
        ss << " : " << generator::type_scope(struct_node, type.parent()) << type.parent().name();
    }
    ss << std::endl << std::string(tabs * 4, ' ') << "{" << std::endl;

    for (size_t idx = inherit_members(type); idx < type.members().size(); ++idx)
    {
        const Member& member = type.member(idx);
        ss << std::string((tabs + 1) * 4, ' ');
        if (member.type().kind() == TypeKind::ARRAY_TYPE)
        {
            ss << generator::array_member(struct_node, member); //Special member syntax
        }
        else
        {
            if (member.type().kind() == TypeKind::STRUCTURE_TYPE && struct_idl != nullptr)
            {
                if (struct_idl->find(struct_node->type().name() + ":dependencies") != struct_idl->end())
                {
                    (*struct_idl)[struct_node->type().name() + ":dependencies"] += ",";
                }
                (*struct_idl)[struct_node->type().name() + ":dependencies"] += member.type().name();
            }
            ss << generator::type_name(struct_node, member.type(), true) << " " << member.name() << ";";
        }
        ss << std::endl;
    }
    ss << std::string(tabs * 4, ' ') << "};" << std::endl;
    return ss.str();
}

inline std::string generate_union(
        const std::string& name,
        const UnionType& type,
        dependencytree::DependencyNode* union_node,
        size_t tabs)
{
    dependencynode_assert(union_node, xUNION);

    std::stringstream ss;
    ss << std::string(tabs * 4, ' ') << "union " << name
       << " switch (" << generator::type_name(union_node, type.discriminator()) << ")" << std::endl;

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
        if (member.type().kind() == TypeKind::ARRAY_TYPE)
        {
            ss << generator::array_member(union_node, member); //Special member syntax
        }
        else
        {
            ss << generator::type_name(union_node, member.type()) << " " << member.name() << ";";
        }
        ss << std::endl;
    }
    ss << std::string(tabs * 4, ' ') << "};" << std::endl;
    return ss.str();
}

inline std::string aliase(
        const std::string& name,
        const DynamicType& type,
        dependencytree::DependencyNode* alias_node)
{
    dependencynode_assert(alias_node, xALIAS);

    std::stringstream ss;
    ss << "typedef " << generator::type_name(alias_node, type) << " " << name;
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

inline std::string enumeration32(
        const std::string& name,
        const EnumerationType<uint32_t>& enumeration,
        size_t tabs)
{
    std::stringstream ss;
    // We must add them in order
    using map_pair = std::pair<std::string, uint32_t>;
    std::map<std::string, uint32_t> enumerators = enumeration.enumerators();
    ss << std::string(tabs * 4, ' ') << "enum " << name << std::endl;
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

inline std::string get_const_value(
        ReadableDynamicDataRef data)
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
    else if (data.type().kind() == TypeKind::STRING16_TYPE)
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
    else if (data.type().kind() == TypeKind::WIDE_CHAR_TYPE)
    {
        prefix = "L'";
        suffix = "'";
    }

    ss << prefix << data.cast<std::string>() << suffix;

    return ss.str();
}

} //namespace generator
} //namespace idl
} //namespace xtypes
} //namespace eprosima

#endif //EPROSIMA_XTYPES_IDL_GENERATOR_HPP_
