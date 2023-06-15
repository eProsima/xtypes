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

#ifndef EPROSIMA_XTYPES_STRING_TYPE_HPP_
#define EPROSIMA_XTYPES_STRING_TYPE_HPP_

#include <xtypes/MutableCollectionType.hpp>
#include <xtypes/PrimitiveType.hpp>
#include <xtypes/SequenceInstance.hpp>
#include <xtypes/StructType.hpp>

namespace eprosima {
namespace xtypes {

/// \brief DynamicType representing a string.
/// A StringType represents a TypeKind::STRING_TYPE.
/// A WStringType represents a TypeKind::WSTRING_TYPE.
/// A String16Type represents a TypeKind::STRING16_TYPE.
template<typename CHAR_T, TypeKind KIND, const char* TYPE_NAME>
class TStringType : public MutableCollectionType
{
public:

    /// \brief Construct a string
    /// Depends of the specialization used, the string will contain PrimitiveType of char or wchar elements
    /// \param[in] bounds Limits of the string, 0 means no limits.
    TStringType(
            int bounds = 0)
        : MutableCollectionType(
            KIND,
            TYPE_NAME + ((bounds > 0) ? "_" + std::to_string(bounds) : ""),
            DynamicType::Ptr(primitive_type<CHAR_T>()),
            bounds)
    {
    }

    virtual size_t memory_size() const override
    {
        return sizeof(std::basic_string<CHAR_T>);
    }

    virtual void construct_instance(
            uint8_t* instance) const override
    {
        new (instance) std::basic_string<CHAR_T>();
        reinterpret_cast<std::basic_string<CHAR_T>*>(instance)->reserve(bounds());
    }

    virtual void copy_instance(
            uint8_t* target,
            const uint8_t* source) const override
    {
        const std::basic_string<CHAR_T>& source_string = *reinterpret_cast<const std::basic_string<CHAR_T>*>(source);
        size_t max_size = bounds() > 0 ? size_t(bounds()) : std::basic_string<CHAR_T>::npos;
        new (target) std::basic_string<CHAR_T>(source_string, 0, std::min(max_size, source_string.size()));
    }

    virtual void copy_instance_from_type(
            uint8_t* target,
            const uint8_t* source,
            const DynamicType& arg_other) const override
    {
        const DynamicType& other = (arg_other.kind() == TypeKind::ALIAS_TYPE)
                ? static_cast<const AliasType&>(arg_other).rget()
                : arg_other;

        if (other.kind() == TypeKind::STRUCTURE_TYPE)
        {
            // Resolve one-member struct compatibility
            const StructType& struct_type = static_cast<const StructType&>(other);
            if (struct_type.members().size() == 1)
            {
                copy_instance_from_type(target, source, struct_type.members().at(0).type());
                return;
            }
        }

        xtypes_assert(other.kind() == KIND,
                "Cannot copy data from different types: From '" << other.name() << "' to '" << name() << "'.");

        (void) other;
        const std::basic_string<CHAR_T>& source_string = *reinterpret_cast<const std::basic_string<CHAR_T>*>(source);
        size_t max_size = bounds() > 0 ? size_t(bounds()) : std::basic_string<CHAR_T>::npos;
        new (target) std::basic_string<CHAR_T>(source_string, 0, std::min(max_size, source_string.size()));
    }

    virtual void move_instance(
            uint8_t* target,
            uint8_t* source,
            bool) const override
    {
        new (target) std::basic_string<CHAR_T>(std::move(*reinterpret_cast<const std::basic_string<CHAR_T>*>(source)));
    }

    virtual void destroy_instance(
            uint8_t* instance) const override
    {
        reinterpret_cast<std::basic_string<CHAR_T>*>(instance)->~basic_string();
    }

    virtual bool compare_instance(
            const uint8_t* instance,
            const uint8_t* other_instance) const override
    {
        return *reinterpret_cast<const std::basic_string<CHAR_T>*>(instance) ==
               *reinterpret_cast<const std::basic_string<CHAR_T>*>(other_instance);
    }

    virtual TypeConsistency is_compatible(
            const DynamicType& other) const override
    {
        if (other.kind() == TypeKind::ALIAS_TYPE)
        {
            const AliasType& other_alias = static_cast<const AliasType&>(other);
            return is_compatible(other_alias.rget());
        }

        if (other.kind() == TypeKind::STRUCTURE_TYPE) // Resolve one-member structs
        {
            return other.is_compatible(*this);
        }

        if (other.kind() != KIND)
        {
            return TypeConsistency::NONE;
        }

        const TStringType& other_string = static_cast<const TStringType&>(other);

        if (bounds() == other_string.bounds())
        {
            return TypeConsistency::EQUALS;
        }

        return TypeConsistency::IGNORE_STRING_BOUNDS;
    }

    virtual void for_each_instance(
            const InstanceNode& node,
            InstanceVisitor visitor) const override
    {
        visitor(node);
    }

    virtual void for_each_type(
            const TypeNode& node,
            TypeVisitor visitor) const override
    {
        visitor(node);
    }

    virtual uint8_t* get_instance_at(
            uint8_t* instance,
            size_t index) const override
    {
        void* char_addr = &reinterpret_cast<std::basic_string<CHAR_T>*>(instance)->operator [](index);
        return static_cast<uint8_t*>(char_addr);
    }

    virtual size_t get_instance_size(
            const uint8_t* instance) const override
    {
        return reinterpret_cast<const std::basic_string<CHAR_T>*>(instance)->size();
    }

    virtual uint64_t hash(
            const uint8_t* instance) const override
    {
        std::hash<std::basic_string<CHAR_T> > hash_fn;
        return hash_fn(*reinterpret_cast<const std::basic_string<CHAR_T>*>(instance));
    }

protected:

    std::shared_ptr<DynamicType> clone() const override
    {
        return std::make_shared<TStringType>(*this);
    }

};

/// \brief Specialization for strings that contains chars
constexpr const char string_type_name[] = "std::string";
using StringType = TStringType<char, TypeKind::STRING_TYPE, string_type_name>;

/// \brief Specialization for strings that contains wchars
constexpr const char wstring_type_name[] = "std::wstring";
using WStringType = TStringType<wchar_t, TypeKind::WSTRING_TYPE, wstring_type_name>;

/// \brief Specialization for strings that contains char16s
constexpr const char string16_type_name[] = "std::u16string";
using String16Type = TStringType<char16_t, TypeKind::STRING16_TYPE, string16_type_name>;

} //namespace xtypes
} //namespace eprosima

#endif //EPROSIMA_XTYPES_STRING_TYPE_HPP_
