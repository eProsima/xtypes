/*
 * Copyright 2020, Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef EPROSIMA_XTYPES_UNION_TYPE_HPP_
#define EPROSIMA_XTYPES_UNION_TYPE_HPP_

#include <xtypes/AggregationType.hpp>
#include <xtypes/AliasType.hpp>

#include <string>
#include <map>
#include <vector>

namespace eprosima {
namespace xtypes {

static const std::string UNION_DISCRIMINATOR("discriminator");

/// \brief DynamicType representing an union.
/// A UnionType represents a TypeKind::UNION_TYPE.
class UnionType : public AggregationType
{
public:
    //static const std::string DISCRIMINATOR("discriminator");

    /// \brief Construct a UnionType given a name.
    /// \param[in] name Name of the union.
    UnionType(
            const std::string& name,
            const DynamicType& discriminator)
        : AggregationType(TypeKind::UNION_TYPE, name)
        , default_(nullptr)
        , memory_size_(0)
    {
        xtypes_assert(
            check_discriminator(discriminator),
            "Discriminator type for Union '" << name << "' isn't allowed.");

        set_discriminator(Member(UNION_DISCRIMINATOR, discriminator));
    }

    UnionType(const UnionType& other) = default;
    UnionType(UnionType&& other) = default;

    /// \brief Add a member to the union.
    /// \param[in] member Member to add
    /// \pre The member name must not exists in this UnionType.
    /// \returns A reference to this UnionType.
    template<typename T>
    UnionType& add_case_member(
            const std::vector<T>& labels,
            const Member& member,
            bool is_default = false)
    {
        xtypes_assert(!labels.empty(), "Cannot add a case member without labels.");
        // Check labels
        for (T label : labels)
        {
            xtypes_assert(
                labels_.count(static_cast<size_t>(label)) == 0,
                "Label with value '" << label << "' already in use while adding case member '" << member.name()
                    << "' to UnionType '" << name() << "'.");
        }

        Member& inner = insert_member(member);
        inner.offset_ = memory_size_;
        memory_size_ += inner.type().memory_size();

        // Add labels
        for (T label : labels)
        {
            labels_[static_cast<size_t>(label)] = &inner;
        }

        if (is_default)
        {
            default_ = &inner;
        }

        return *this;
    }

    /// \brief Create a member in this union.
    /// \param[in] name Member name to create.
    /// \param[in] type Member type of the member.
    /// \pre The member name must not exists in this UnionType.
    /// \returns A reference to this UnionType.
    template<typename T>
    UnionType& add_case_member(
            const std::vector<T>& labels,
            const std::string& name,
            const DynamicType& type,
            bool is_default = false)
    {
        return add_case_member(labels, Member(name, type), is_default);
    }

    /// \brief Create a member in this union with a type as rvalue.
    /// \param[in] name Member name to create.
    /// \param[in] type Member type fo the member.
    /// \pre The member name must not exists in this UnionType.
    /// \returns A reference to this UnionType.
    template<typename T,
             typename DynamicTypeImpl>
    UnionType& add_case_member(
            const std::vector<T>& labels,
            const std::string& name,
            const DynamicTypeImpl&& type,
            bool is_default = false)
    {
        return add_case_member(labels, Member(name, type), is_default);
    }

    bool select_case(
            uint8_t* instance,
            const std::string& case_member_name)
    {
        const Member& disc = member(UNION_DISCRIMINATOR);
        size_t disc_value = current_label(disc.type(), instance);
        for (const auto& pair : labels_)
        {
            const Member* member = pair.second;
            if (member->name() == case_member_name)
            {
                disc_value = pair.first;
                return true;
            }
        }
        return false;
    }

    Member* get_current_selection(
            uint8_t* instance)
    {
        const Member& disc = member(UNION_DISCRIMINATOR);
        size_t disc_value = current_label(disc.type(), instance);
        return labels_[disc_value];
    }

    virtual size_t memory_size() const override
    {
        return memory_size_;
    }

    virtual void construct_instance(
            uint8_t* instance) const override
    {
        for(auto&& member: members())
        {
            member.type().construct_instance(instance + member.offset());
        }
    }

    virtual void copy_instance(
            uint8_t* target,
            const uint8_t* source) const override
    {
        for(auto&& member: members())
        {
            member.type().copy_instance(target + member.offset(), source + member.offset());
        }
    }

    virtual void copy_instance_from_type(
            uint8_t* target,
            const uint8_t* source,
            const DynamicType& other) const override
    {
        xtypes_assert(other.kind() == TypeKind::UNION_TYPE,
            "Cannot copy data from different types: From '" << other.name() << "' to '" << name() << "'.");
        const UnionType& other_union = static_cast<const UnionType&>(other);

        auto other_member = other_union.members().begin();
        for(auto&& member: members())
        {
            if(other_member != other_union.members().end())
            {
                member.type().copy_instance_from_type(
                        target + member.offset(),
                        source + other_member->offset(),
                        other_member->type());
            }
            else
            {
                member.type().construct_instance(target + member.offset());
            }
            other_member++;
        }
    }

    virtual void move_instance(
            uint8_t* target,
            uint8_t* source) const override
    {
        for(auto&& member: members())
        {
            member.type().move_instance(target + member.offset(), source + member.offset());
        }
    }

    virtual void destroy_instance(
            uint8_t* instance) const override
    {
        for (auto&& member = members().rbegin(); member != members().rend(); ++member)
        {
            member->type().destroy_instance(instance + member->offset());
        }
    }

    virtual bool compare_instance(
            const uint8_t* instance,
            const uint8_t* other_instance) const override
    {
        for(auto&& member: members())
        {
            if(!member.type().compare_instance(instance + member.offset(), other_instance + member.offset()))
            {
                return false;
            }
        }
        return true;
        /*
        Member* current = labels_[current_];
        xtypes_assert(current != nullptr, "UnionType '" << name() << "' doesn't have a case member selected.");
        if(!current->type().compare_instance(instance + current->offset(), other_instance + current->offset()))
        {
            return false;
        }
        return true;
        */
    }

    virtual TypeConsistency is_compatible(
            const DynamicType& other) const override
    {
        if(other.kind() != TypeKind::UNION_TYPE)
        {
            return TypeConsistency::NONE;
        }

        const UnionType& other_union = static_cast<const UnionType&>(other);

        TypeConsistency consistency = TypeConsistency::EQUALS;
        auto other_member = other_union.members().begin();
        for(auto&& member: members())
        {
            if(other_member != other_union.members().end())
            {
                TypeConsistency internal_consistency = member.type().is_compatible(other_member->type());
                if(internal_consistency == TypeConsistency::NONE)
                {
                    return TypeConsistency::NONE;
                }

                if(member.name() != other_member->name())
                {
                    consistency |= TypeConsistency::IGNORE_MEMBER_NAMES;
                }
                consistency |= internal_consistency;
            }
            else
            {
                return consistency | TypeConsistency::IGNORE_MEMBERS;
            }
            other_member++;
        }
        if(other_member != other_union.members().end())
        {
            consistency |= TypeConsistency::IGNORE_MEMBERS;
        }

        return consistency;
    }

    virtual void for_each_instance(
            const InstanceNode& node,
            InstanceVisitor visitor) const override
    {
        const Member& disc = member(UNION_DISCRIMINATOR);
        size_t disc_value = current_label(disc.type(), node.instance);

        Member* current = labels_.at(disc_value);
        xtypes_assert(current != nullptr, "UnionType '" << name() << "' doesn't have a case member selected.");
        visitor(node);
        InstanceNode child(node, current->type(), node.instance + current->offset(), disc_value, current);
        current->type().for_each_instance(child, visitor);
    }

    virtual void for_each_type(
            const TypeNode& node,
            TypeVisitor visitor) const override
    {
        visitor(node);
        for(size_t i = 0; i < members().size(); i++)
        {
            const Member& member = members()[i];
            TypeNode child(node, member.type(), i, &member);
            member.type().for_each_type(child, visitor);
        }
    }

protected:
    virtual DynamicType* clone() const override
    {
        return new UnionType(*this);
    }

    UnionType& set_discriminator(
            const Member& member)
    {
        Member& inner = insert_member(member);
        inner.offset_ = memory_size_;
        memory_size_ += inner.type().memory_size();
        return *this;
    }

    bool check_discriminator(
            const DynamicType& type) const
    {
        bool result =
            (type.is_primitive_type()
                && type.kind() != TypeKind::FLOAT_32_TYPE
                && type.kind() != TypeKind::FLOAT_64_TYPE
                && type.kind() != TypeKind::FLOAT_128_TYPE)
            || type.is_enumerated_type();

        if (!result && type.kind() == TypeKind::ALIAS_TYPE)
        {
            const AliasType& alias = static_cast<const AliasType&>(type);
            result = check_discriminator(alias.rget());
        }
        return result;
    }

    void current_label(
            const DynamicType& type,
            uint8_t* instance,
            uint8_t* label_instance)
    {
        const Member& disc = member(UNION_DISCRIMINATOR);
        xtypes_assert(
            disc.type().kind() == type.kind(),
            "Cannot set label value of type '" << type.name() << "' to the UnionType '" << name()
                << "' with discriminator type '" << disc.type().name() << "'.");
        // Direct instance memory hack to avoid using DynamicData
        // NOTE: THe discriminator offset is always 0.
        switch (type.kind())
        {
            case TypeKind::BOOLEAN_TYPE:
                {
                    bool lvalue = *reinterpret_cast<bool*>(label_instance);
                    bool& value = *reinterpret_cast<bool*>(instance);
                    value = static_cast<size_t>(lvalue);
                }
                break;
            case TypeKind::INT_8_TYPE:
                {
                    int8_t lvalue = *reinterpret_cast<int8_t*>(label_instance);
                    int8_t& value = *reinterpret_cast<int8_t*>(instance);
                    value = static_cast<size_t>(lvalue);
                }
                break;
            case TypeKind::UINT_8_TYPE:
                {
                    uint8_t lvalue = *reinterpret_cast<uint8_t*>(label_instance);
                    uint8_t& value = *reinterpret_cast<uint8_t*>(instance);
                    value = static_cast<size_t>(lvalue);
                }
                break;
            case TypeKind::INT_16_TYPE:
                {
                    int16_t lvalue = *reinterpret_cast<int16_t*>(label_instance);
                    int16_t& value = *reinterpret_cast<int16_t*>(instance);
                    value = static_cast<size_t>(lvalue);
                }
                break;
            case TypeKind::UINT_16_TYPE:
                {
                    uint16_t lvalue = *reinterpret_cast<uint16_t*>(label_instance);
                    uint16_t& value = *reinterpret_cast<uint16_t*>(instance);
                    value = static_cast<size_t>(lvalue);
                }
                break;
            case TypeKind::INT_32_TYPE:
                {
                    int32_t lvalue = *reinterpret_cast<int32_t*>(label_instance);
                    int32_t& value = *reinterpret_cast<int32_t*>(instance);
                    value = static_cast<size_t>(lvalue);
                }
                break;
            case TypeKind::UINT_32_TYPE:
                {
                    uint32_t lvalue = *reinterpret_cast<uint32_t*>(label_instance);
                    uint32_t& value = *reinterpret_cast<uint32_t*>(instance);
                    value = static_cast<size_t>(lvalue);
                }
                break;
            case TypeKind::INT_64_TYPE:
                {
                    int64_t lvalue = *reinterpret_cast<int64_t*>(label_instance);
                    int64_t& value = *reinterpret_cast<int64_t*>(instance);
                    value = static_cast<size_t>(lvalue);
                }
                break;
            case TypeKind::UINT_64_TYPE:
                {
                    uint64_t lvalue = *reinterpret_cast<uint64_t*>(label_instance);
                    uint64_t& value = *reinterpret_cast<uint64_t*>(instance);
                    value = static_cast<size_t>(lvalue);
                }
                break;
            case TypeKind::CHAR_8_TYPE:
                {
                    char lvalue = *reinterpret_cast<char*>(label_instance);
                    char& value = *reinterpret_cast<char*>(instance);
                    value = static_cast<size_t>(lvalue);
                }
                break;
            case TypeKind::CHAR_16_TYPE:
                {
                    wchar_t lvalue = *reinterpret_cast<wchar_t*>(label_instance);
                    wchar_t& value = *reinterpret_cast<wchar_t*>(instance);
                    value = static_cast<size_t>(lvalue);
                }
                break;
            case TypeKind::ENUMERATION_TYPE:
                {
                    // TODO: If other enumeration types are added, switch again.
                    uint32_t lvalue = *reinterpret_cast<uint32_t*>(label_instance);
                    uint32_t& value = *reinterpret_cast<uint32_t*>(instance);
                    value = static_cast<size_t>(lvalue);
                }
                break;
            case TypeKind::ALIAS_TYPE:
                {
                    const AliasType& alias = static_cast<const AliasType&>(type);
                    current_label(alias.rget(), instance, label_instance);
                }
                break;
            default:
                xtypes_assert(false, "Unsupported discriminator type: " << type.name());
        }
    }

    size_t current_label(
            const DynamicType& type,
            uint8_t* instance) const
    {
        // Direct instance memory hack to avoid using DynamicData
        // NOTE: THe discriminator offset is always 0.
        size_t disc_value = 0;
        switch (type.kind())
        {
            case TypeKind::BOOLEAN_TYPE:
                {
                    bool value = *reinterpret_cast<bool*>(instance);
                    disc_value = static_cast<size_t>(value);
                }
                break;
            case TypeKind::INT_8_TYPE:
                {
                    int8_t value = *reinterpret_cast<int8_t*>(instance);
                    disc_value = static_cast<size_t>(value);
                }
                break;
            case TypeKind::UINT_8_TYPE:
                {
                    uint8_t value = *reinterpret_cast<uint8_t*>(instance);
                    disc_value = static_cast<size_t>(value);
                }
                break;
            case TypeKind::INT_16_TYPE:
                {
                    int16_t value = *reinterpret_cast<int16_t*>(instance);
                    disc_value = static_cast<size_t>(value);
                }
                break;
            case TypeKind::UINT_16_TYPE:
                {
                    uint16_t value = *reinterpret_cast<uint16_t*>(instance);
                    disc_value = static_cast<size_t>(value);
                }
                break;
            case TypeKind::INT_32_TYPE:
                {
                    int32_t value = *reinterpret_cast<int32_t*>(instance);
                    disc_value = static_cast<size_t>(value);
                }
                break;
            case TypeKind::UINT_32_TYPE:
                {
                    uint32_t value = *reinterpret_cast<uint32_t*>(instance);
                    disc_value = static_cast<size_t>(value);
                }
                break;
            case TypeKind::INT_64_TYPE:
                {
                    int64_t value = *reinterpret_cast<int64_t*>(instance);
                    disc_value = static_cast<size_t>(value);
                }
                break;
            case TypeKind::UINT_64_TYPE:
                {
                    uint64_t value = *reinterpret_cast<uint64_t*>(instance);
                    disc_value = static_cast<size_t>(value);
                }
                break;
            case TypeKind::CHAR_8_TYPE:
                {
                    char value = *reinterpret_cast<char*>(instance);
                    disc_value = static_cast<size_t>(value);
                }
                break;
            case TypeKind::CHAR_16_TYPE:
                {
                    wchar_t value = *reinterpret_cast<wchar_t*>(instance);
                    disc_value = static_cast<size_t>(value);
                }
                break;
            case TypeKind::ENUMERATION_TYPE:
                {
                    // TODO: If other enumeration types are added, switch again.
                    uint32_t value = *reinterpret_cast<uint32_t*>(instance);
                    disc_value = static_cast<size_t>(value);
                }
                break;
            case TypeKind::ALIAS_TYPE:
                {
                    const AliasType& alias = static_cast<const AliasType&>(type);
                    disc_value = current_label(alias.rget(), instance);
                }
                break;
            default:
                xtypes_assert(false, "Unsupported discriminator type: " << type.name());
        }
        return disc_value;
    }

private:
    std::map<size_t, Member*> labels_;
    Member* default_;
    size_t memory_size_;
};

} //namespace xtypes
} //namespace eprosima

#endif //EPROSIMA_XTYPES_UNION_TYPE_HPP_
