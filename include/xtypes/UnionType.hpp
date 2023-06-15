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
#include <xtypes/EnumerationType.hpp>
#include <xtypes/StringConversion.hpp>

#include <string>
#include <map>
#include <vector>
#include <regex>
#include <cwchar>
#include <limits>

namespace eprosima {
namespace xtypes {

class DynamicData;
class ReadableDynamicDataRef;
class WritableDynamicDataRef;

static const std::string UNION_DISCRIMINATOR("discriminator");

constexpr int64_t default_union_label(
        size_t type_size)
{
    return std::numeric_limits<int64_t>::max() >> ((sizeof(int64_t) - type_size) * 8);
}

template<typename T>
constexpr int64_t shrink_label(
        T value,
        size_t type_size)
{
    return static_cast<int64_t>(value) & (std::numeric_limits<uint64_t>::max()  >> ((sizeof(int64_t) - type_size) * 8));
}

/// \brief DynamicType representing an union.
/// A UnionType represents a TypeKind::UNION_TYPE.
class UnionType : public AggregationType
{
public:

    /// \brief Construct a UnionType given a name.
    /// \param[in] name Name of the union.
    UnionType(
            const std::string& name,
            const DynamicType& discriminator)
        : AggregationType(TypeKind::UNION_TYPE, name)
        , memory_size_(0)
        , default_value_(default_union_label(discriminator.memory_size()))
        , invalid_value_(default_value_ - 1)
        , default_(invalid_value_)
        , maximum_case_member_memory_(0)
    {
        xtypes_assert(
            check_discriminator(discriminator),
            "Discriminator type for Union '" << name << "' isn't allowed.");

        set_discriminator(Member(UNION_DISCRIMINATOR, discriminator));
    }

    UnionType(
            const UnionType& other) = default;
    UnionType(
            UnionType&& other) = default;

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
        static_assert (std::is_integral<T>::value, "Only 'integral' types are allowed.");
        xtypes_assert(is_default || !labels.empty(), "Cannot add a non default case member without labels.");
        xtypes_assert(UNION_DISCRIMINATOR != member.name(), "Case member name 'discriminator' is reserved.");
        // Check labels
        for (T label : labels)
        {
            int64_t l = shrink_label<T>(label, disc()->type().memory_size());
            check_label_value(l);
            xtypes_assert(
                labels_.count(l) == 0,
                "Label with value '" << label << "' already in use while adding case member '" << member.name()
                                     << "' to UnionType '" << name() << "'.");
        }

        Member& inner = insert_member(member);
        Member* disc_ = disc();

        // In an Union, all members share the memory just after the discriminator.
        inner.offset_ = disc_->type().memory_size();

        // And the memory_size_ is the size of the biggest member plus the size of the discriminator.
        if (inner.type().memory_size() > maximum_case_member_memory_)
        {
            maximum_case_member_memory_ = inner.type().memory_size();
            memory_size_ = disc_->type().memory_size() + maximum_case_member_memory_;
        }

        // Add labels
        int64_t first_label = default_value_;
        for (T label : labels)
        {
            int64_t l = shrink_label<T>(label, disc()->type().memory_size());
            if (l != default_value_)
            {
                labels_[l] = inner.name();
                if (first_label == default_value_)
                {
                    first_label = l;
                }
            }
        }

        if (is_default)
        {
            xtypes_assert(default_ == invalid_value_, "Cannot set more than one case member as default.");
            default_ = first_label;
            if (default_ == default_value_)
            {
                labels_[default_] = inner.name(); // If this default has no other labels, must be added now.
            }
        }

        return *this;
    }

    /// \brief Specialized version for labels as string representation
    /// It will convert these strings to the discriminator type.
    /// If the member is going to be the default one, it is expected to receive a label named "default".
    UnionType& add_case_member(
            const std::vector<std::string>& labels,
            const Member& member)
    {
        std::vector<int64_t> values;
        bool is_default = parse_labels(labels, values);
        return add_case_member(values, member, is_default);
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

    /// \brief This method retrieves the discriminator type.
    const DynamicType& discriminator() const
    {
        return disc()->type();
    }

    /// \brief Return a list of labels for the given case member name.
    std::vector<int64_t> get_labels(
            const std::string& member) const
    {
        std::vector<int64_t> result;
        for (const auto& pair : labels_)
        {
            if (pair.second == member && pair.first != default_value_)
            {
                result.push_back(pair.first);
            }
        }
        return result;
    }

    /// \brief Returns the default case member name.
    std::string get_default() const
    {
        if (default_ != invalid_value_)
        {
            return labels_.at(default_);
        }
        return std::string();
    }

    /// \brief Checks if the case member name is set as default.
    bool is_default(
            const std::string& name) const
    {
        if (default_ != invalid_value_)
        {
            const std::string& member = labels_.at(default_);
            return member == name;
        }
        return false;
    }

    /// \brief Returns a list of case member names.
    std::vector<std::string> get_case_members() const
    {
        std::vector<std::string> result;
        for (size_t i = 1; i < members().size(); ++i) // Skip "disciminator" member
        {
            result.push_back(member(i).name());
        }
        return result;
    }

    virtual size_t memory_size() const override
    {
        return memory_size_;
    }

    virtual void construct_instance(
            uint8_t* instance) const override
    {
        // Discriminator must be built.
        disc()->type().construct_instance(instance);

        // If a default exists, built it too. And set it as active.
        if (default_ != invalid_value_)
        {
            const Member& def = member(labels_.at(default_));
            def.type().construct_instance(instance + def.offset());
            current_label(instance, default_);
        }
        else
        {
            current_label(instance, invalid_value_);
        }
    }

    virtual void copy_instance(
            uint8_t* target,
            const uint8_t* source) const override
    {
        disc()->type().copy_instance(target, source);
        int64_t label = current_label(disc()->type(), source);
        xtypes_assert(labels_.count(label) != 0, "Stored label in source is invalid");
        Member* member = &const_cast<Member&>(AggregationType::member(labels_.at(label)));
        member->type().copy_instance(target + member->offset(), source + member->offset());
    }

    virtual void copy_instance_from_type(
            uint8_t* target,
            const uint8_t* source,
            const DynamicType& other) const override
    {
        if (other.kind() == TypeKind::ALIAS_TYPE)
        {
            [[maybe_unused]] const AliasType& alias = static_cast<const AliasType&>(other);

            xtypes_assert(alias.rget().kind() == TypeKind::UNION_TYPE,
                    "Cannot copy data from different types: From '" << alias.rget().name() << "' to '" << name() <<
                    "'.");
        }
        else
        {
            xtypes_assert(other.kind() == TypeKind::UNION_TYPE,
                    "Cannot copy data from different types: From '" << other.name() << "' to '" << name() << "'.");
        }
        const UnionType& other_union = static_cast<const UnionType&>(other);

        Member* disc_ = disc();
        disc_->type().copy_instance_from_type(target, source, other_union.disc()->type());

        int64_t label = current_label(disc_->type(), target);
        if (0 < labels_.count(label))
        {
            Member* member = &const_cast<Member&>(AggregationType::member(labels_.at(label)));
            member->type().copy_instance_from_type(
                target + member->offset(),
                source + member->offset(),
                member->type());
        }
    }

    virtual void move_instance(
            uint8_t* target,
            uint8_t* source,
            bool initialized) const override
    {
        disc()->type().move_instance(target, source, initialized);
        int64_t label = current_label(disc()->type(), target);
        if (0 < labels_.count(label))
        {
            Member* member = &const_cast<Member&>(AggregationType::member(labels_.at(label)));
            member->type().move_instance(target + member->offset(),
                    source + member->offset(), initialized);
        }
    }

    virtual void destroy_instance(
            uint8_t* instance) const override
    {
        disc()->type().destroy_instance(instance);
        int64_t label = current_label(disc()->type(), instance);
        if (0 < labels_.count(label))
        {
            Member* member = &const_cast<Member&>(AggregationType::member(labels_.at(label)));
            member->type().destroy_instance(instance + member->offset());
        }
    }

    virtual bool compare_instance(
            const uint8_t* instance,
            const uint8_t* other_instance) const override
    {
        bool result = disc()->type().compare_instance(instance, other_instance);

        if (result)
        {
            int64_t label = current_label(disc()->type(), instance);
            if (0 < labels_.count(label))
            {
                Member* member = &const_cast<Member&>(AggregationType::member(labels_.at(label)));
                // Compare active members
                result = result
                        && member->type().compare_instance(instance + member->offset(),
                                other_instance + member->offset());
            }
        }

        return result;
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

        if (other.kind() != TypeKind::UNION_TYPE)
        {
            return TypeConsistency::NONE;
        }

        const UnionType& other_union = static_cast<const UnionType&>(other);

        TypeConsistency consistency = TypeConsistency::EQUALS;
        auto other_member = other_union.members().begin();
        for (auto&& member: members())
        {
            if (other_member != other_union.members().end())
            {
                TypeConsistency internal_consistency = member.type().is_compatible(other_member->type());
                if (internal_consistency == TypeConsistency::NONE)
                {
                    return TypeConsistency::NONE;
                }

                if (member.name() != other_member->name())
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
        if (other_member != other_union.members().end())
        {
            consistency |= TypeConsistency::IGNORE_MEMBERS;
        }

        return consistency;
    }

    virtual void for_each_instance(
            const InstanceNode& node,
            InstanceVisitor visitor) const override
    {
        int64_t disc_value = current_label(disc()->type(), node.instance);

        xtypes_assert(0 < labels_.count(disc_value),
                "UnionType '" << name() << "' doesn't have a case member selected.");
        Member* member = &const_cast<Member&>(AggregationType::member(labels_.at(disc_value)));
        visitor(node);
        InstanceNode child(
            node,
            member->type(),
            node.instance + member->offset(),
            disc_value, member);
        member->type().for_each_instance(child, visitor);
    }

    virtual void for_each_type(
            const TypeNode& node,
            TypeVisitor visitor) const override
    {
        visitor(node);
        for (size_t i = 0; i < members().size(); i++)
        {
            const Member& member = members()[i];
            TypeNode child(node, member.type(), i, &member);
            member.type().for_each_type(child, visitor);
        }
    }

    virtual uint64_t hash(
            const uint8_t* instance) const override
    {
        uint64_t h = disc()->type().hash(instance);
        if (0 < labels_.count(h))
        {
            Member* member = &const_cast<Member&>(AggregationType::member(labels_.at(h)));
            Instanceable::hash_combine(h, member->type().hash(instance + member->offset()));
        }
        return h;
    }

protected:

    friend DynamicData;
    friend ReadableDynamicDataRef;
    friend WritableDynamicDataRef;

    std::shared_ptr<DynamicType> clone() const override
    {
        return std::make_shared<UnionType>(*this);
    }

    /// \brief This method adds the discriminator has the first member of the aggregation.
    /// Its offset will be always 0.
    UnionType& set_discriminator(
            const Member& member)
    {
        Member& inner = insert_member(member);
        inner.offset_ = 0;
        memory_size_ += inner.type().memory_size();
        return *this;
    }

    /// \brief This method verifies that the discriminator type is allowed.
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

    /// \brief This method verifies the validity of a given label.
    void check_label_value(
         [[maybe_unused]] int64_t label)
    {
        xtypes_assert(label != default_value_, "Label '" << label << "' is reserved.");
        DynamicType* type = &const_cast<DynamicType&>(disc()->type());

        if (type->kind() == TypeKind::ALIAS_TYPE)
        {
            AliasType* alias = static_cast<AliasType*>(type);
            type = &const_cast<DynamicType&>(alias->rget());
        }

        if (type->kind() == TypeKind::ENUMERATION_TYPE)
        {
            [[maybe_unused]] EnumerationType<uint32_t>* enum_type = static_cast<EnumerationType<uint32_t>*>(type);
            xtypes_assert(
                enum_type->is_allowed_value(label),
                "Value '" << label << "' isn't allowed by the discriminator enumeration '" << type->name() << "'");
        }
    }

    /// \brief This method sets changes the discriminator's value given a label_instance and its type.
    void current_label(
            const DynamicType& type,
            uint8_t* instance,
            uint8_t* label_instance)
    {
        xtypes_assert(
            disc()->type().kind() == type.kind(),
            "Cannot set label value of type '" << type.name() << "' to the UnionType '" << name()
                                               << "' with discriminator type '" << disc()->type().name() << "'.");
        // Direct instance memory hack to avoid using DynamicData
        // NOTE: THe discriminator offset is always 0.
        switch (type.kind())
        {
            case TypeKind::BOOLEAN_TYPE:
            case TypeKind::INT_8_TYPE:
            {
                int8_t lvalue = *reinterpret_cast<int8_t*>(label_instance);
                int64_t new_value = static_cast<int64_t>(lvalue);
                current_label(instance, new_value);
            }
            break;
            case TypeKind::UINT_8_TYPE:
            {
                uint8_t lvalue = *reinterpret_cast<uint8_t*>(label_instance);
                int64_t new_value = static_cast<int64_t>(lvalue);
                current_label(instance, new_value);
            }
            break;
            case TypeKind::INT_16_TYPE:
            {
                int16_t lvalue = *reinterpret_cast<int16_t*>(label_instance);
                int64_t new_value = static_cast<int64_t>(lvalue);
                current_label(instance, new_value);
            }
            break;
            case TypeKind::UINT_16_TYPE:
            {
                uint16_t lvalue = *reinterpret_cast<uint16_t*>(label_instance);
                int64_t new_value = static_cast<int64_t>(lvalue);
                current_label(instance, new_value);
            }
            break;
            case TypeKind::INT_32_TYPE:
            {
                int32_t lvalue = *reinterpret_cast<int32_t*>(label_instance);
                int64_t new_value = static_cast<int64_t>(lvalue);
                current_label(instance, new_value);
            }
            break;
            case TypeKind::UINT_32_TYPE:
            {
                uint32_t lvalue = *reinterpret_cast<uint32_t*>(label_instance);
                int64_t new_value = static_cast<int64_t>(lvalue);
                current_label(instance, new_value);
            }
            break;
            case TypeKind::INT_64_TYPE:
            {
                int64_t lvalue = *reinterpret_cast<int64_t*>(label_instance);
                current_label(instance, lvalue);
            }
            break;
            case TypeKind::UINT_64_TYPE:
            {
                uint64_t lvalue = *reinterpret_cast<uint64_t*>(label_instance);
                int64_t new_value = static_cast<int64_t>(lvalue);
                current_label(instance, new_value);
            }
            break;
            case TypeKind::CHAR_8_TYPE:
            {
                char lvalue = *reinterpret_cast<char*>(label_instance);
                int64_t new_value = static_cast<int64_t>(lvalue);
                current_label(instance, new_value);
            }
            break;
            case TypeKind::CHAR_16_TYPE:
            {
                char16_t lvalue = *reinterpret_cast<char16_t*>(label_instance);
                int64_t new_value = static_cast<int64_t>(lvalue);
                current_label(instance, new_value);
            }
            break;
            case TypeKind::WIDE_CHAR_TYPE:
            {
                wchar_t lvalue = *reinterpret_cast<wchar_t*>(label_instance);
                int64_t new_value = static_cast<int64_t>(lvalue);
                current_label(instance, new_value);
            }
            break;
            case TypeKind::ENUMERATION_TYPE:
            {
                // TODO: If other enumeration types are added, switch again.
                uint32_t lvalue = *reinterpret_cast<uint32_t*>(label_instance);
                int64_t new_value = static_cast<int64_t>(lvalue);
                current_label(instance, new_value);
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

    /// \brief This method sets changes the discriminator's value.
    void current_label(
            uint8_t* instance,
            int64_t new_value) const
    {
        Member* disc_ = disc();
        TypeKind kind = disc_->type().kind();
        if (kind == TypeKind::ALIAS_TYPE)
        {
            const AliasType& alias = static_cast<const AliasType&>(disc_->type());
            kind = alias.rget().kind();
        }
        // Direct instance memory hack to avoid using DynamicData
        // NOTE: THe discriminator offset is always 0.
        switch (kind)
        {
            case TypeKind::BOOLEAN_TYPE:
            case TypeKind::INT_8_TYPE:
            {
                int8_t lvalue = static_cast<int8_t>(new_value);
                int8_t& value = *reinterpret_cast<int8_t*>(instance);
                value = lvalue;
            }
            break;
            case TypeKind::UINT_8_TYPE:
            {
                uint8_t lvalue = static_cast<uint8_t>(new_value);
                uint8_t& value = *reinterpret_cast<uint8_t*>(instance);
                value = lvalue;
            }
            break;
            case TypeKind::INT_16_TYPE:
            {
                int16_t lvalue = static_cast<int16_t>(new_value);
                int16_t& value = *reinterpret_cast<int16_t*>(instance);
                value = lvalue;
            }
            break;
            case TypeKind::UINT_16_TYPE:
            {
                uint16_t lvalue = static_cast<uint16_t>(new_value);
                uint16_t& value = *reinterpret_cast<uint16_t*>(instance);
                value = lvalue;
            }
            break;
            case TypeKind::INT_32_TYPE:
            {
                int32_t lvalue = static_cast<int32_t>(new_value);
                int32_t& value = *reinterpret_cast<int32_t*>(instance);
                value = lvalue;
            }
            break;
            case TypeKind::UINT_32_TYPE:
            {
                uint32_t lvalue = static_cast<uint32_t>(new_value);
                uint32_t& value = *reinterpret_cast<uint32_t*>(instance);
                value = lvalue;
            }
            break;
            case TypeKind::INT_64_TYPE:
            {
                int64_t lvalue = static_cast<int64_t>(new_value);
                int64_t& value = *reinterpret_cast<int64_t*>(instance);
                value = lvalue;
            }
            break;
            case TypeKind::UINT_64_TYPE:
            {
                uint64_t lvalue = static_cast<uint64_t>(new_value);
                uint64_t& value = *reinterpret_cast<uint64_t*>(instance);
                value = lvalue;
            }
            break;
            case TypeKind::CHAR_8_TYPE:
            {
                char lvalue = static_cast<char>(new_value);
                char& value = *reinterpret_cast<char*>(instance);
                value = lvalue;
            }
            break;
            case TypeKind::CHAR_16_TYPE:
            {
                char16_t lvalue = static_cast<char16_t>(new_value);
                char16_t& value = *reinterpret_cast<char16_t*>(instance);
                value = lvalue;
            }
            break;
            case TypeKind::WIDE_CHAR_TYPE:
            {
                wchar_t lvalue = static_cast<wchar_t>(new_value);
                wchar_t& value = *reinterpret_cast<wchar_t*>(instance);
                value = lvalue;
            }
            break;
            case TypeKind::ENUMERATION_TYPE:
            {
                // TODO: If other enumeration types are added, switch again.
                uint32_t lvalue = static_cast<uint32_t>(new_value);
                uint32_t& value = *reinterpret_cast<uint32_t*>(instance);
                value = lvalue;
            }
            break;
            case TypeKind::ALIAS_TYPE:
            {
                xtypes_assert(false, "Internal and ugly error: " << disc_->type().name());
            }
            break;
            default:
                xtypes_assert(false, "Unsupported discriminator type: " << disc_->type().name());
        }
    }

    /// \brief This method returns the value of the discriminator represented by instance and its type.
    int64_t current_label(
            const DynamicType& type,
            const uint8_t* instance) const
    {
        // Direct instance memory hack to avoid using DynamicData
        // NOTE: THe discriminator offset is always 0.
        int64_t disc_value = 0;
        switch (type.kind())
        {
            case TypeKind::BOOLEAN_TYPE:
            case TypeKind::INT_8_TYPE:
            {
                int8_t value = *reinterpret_cast<const int8_t*>(instance);
                disc_value = shrink_label(value, type.memory_size());
            }
            break;
            case TypeKind::UINT_8_TYPE:
            {
                uint8_t value = *reinterpret_cast<const uint8_t*>(instance);
                disc_value = shrink_label(value, type.memory_size());
            }
            break;
            case TypeKind::INT_16_TYPE:
            {
                int16_t value = *reinterpret_cast<const int16_t*>(instance);
                disc_value = shrink_label(value, type.memory_size());
            }
            break;
            case TypeKind::UINT_16_TYPE:
            {
                uint16_t value = *reinterpret_cast<const uint16_t*>(instance);
                disc_value = shrink_label(value, type.memory_size());
            }
            break;
            case TypeKind::INT_32_TYPE:
            {
                int32_t value = *reinterpret_cast<const int32_t*>(instance);
                disc_value = shrink_label(value, type.memory_size());
            }
            break;
            case TypeKind::UINT_32_TYPE:
            {
                uint32_t value = *reinterpret_cast<const uint32_t*>(instance);
                disc_value = shrink_label(value, type.memory_size());
            }
            break;
            case TypeKind::INT_64_TYPE:
            {
                disc_value = *reinterpret_cast<const int64_t*>(instance);
            }
            break;
            case TypeKind::UINT_64_TYPE:
            {
                uint64_t value = *reinterpret_cast<const uint64_t*>(instance);
                disc_value = shrink_label(value, type.memory_size());
            }
            break;
            case TypeKind::CHAR_8_TYPE:
            {
                char value = *reinterpret_cast<const char*>(instance);
                disc_value = shrink_label(value, type.memory_size());
            }
            break;
            case TypeKind::CHAR_16_TYPE:
            {
                char16_t value = *reinterpret_cast<const char16_t*>(instance);
                disc_value = shrink_label(value, type.memory_size());
            }
            break;
            case TypeKind::WIDE_CHAR_TYPE:
            {
                wchar_t value = *reinterpret_cast<const wchar_t*>(instance);
                disc_value = shrink_label(value, type.memory_size());
            }
            break;
            case TypeKind::ENUMERATION_TYPE:
            {
                // TODO: If other enumeration types are added, switch again.
                uint32_t value = *reinterpret_cast<const uint32_t*>(instance);
                disc_value = shrink_label(value, type.memory_size());
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

    /// \brief This method switches the current selected discriminator, checking its validity.
    bool select_disc(
            const DynamicType& type,
            uint8_t* instance,
            uint8_t* label_instance)
    {
        int64_t new_value = current_label(type, label_instance);
        return select_disc(instance, new_value);
    }

    /// \brief This method switches the current selected discriminator, checking its validity.
    bool select_disc(
            uint8_t* instance,
            int64_t value) const
    {
        if (labels_.count(value) == 0 &&
                default_ == invalid_value_)
        {
            return false;
        }

        // Retrieve current label.
        int64_t current_value = current_label(disc()->type(), instance);

        if (0 < labels_.count(current_value) &&
                0 != labels_.at(current_value).compare(labels_.at(
                    value)))
        {
            xtypes_assert(false, "Cannot switch member using direct disciminator value.");
            return false;
        }

        current_label(instance, value);
        return true;
    }

    /// \brief This method switches the current selected case member by its name.
    bool select_case(
            uint8_t* instance,
            const std::string& case_member_name)
    {
        for (const auto& pair : labels_)
        {
            Member* member = &const_cast<Member&>(AggregationType::member(pair.second));
            if (member->name() == case_member_name)
            {
                activate_member(instance, member);
                current_label(instance, pair.first);
                return true;
            }
        }
        return false;
    }

    /// \brief This method selects the default case member.
    bool select_default(
            uint8_t* instance)
    {
        if (default_ != invalid_value_)
        {
            Member* def = &const_cast<Member&>(member(labels_.at(default_)));
            activate_member(instance, def);
            current_label(instance, default_);
            return true;
        }
        return false;
    }

    /// \brief This method allows to retrieve the current selected case member.
    Member& get_current_selection(
            uint8_t* instance)
    {
        int64_t label = current_label(disc()->type(), instance);
        xtypes_assert(0 < labels_.count(label), "UnionType '" << name() << "' doesn't have a case member selected.");
        Member* member = &const_cast<Member&>(AggregationType::member(labels_.at(label)));
        return *member;
    }

    /// \brief This method allows to retrieve the current selected case member (const).
    const Member& get_current_selection(
            const uint8_t* instance) const
    {
        int64_t label = current_label(disc()->type(), instance);
        xtypes_assert(0 < labels_.count(label), "UnionType '" << name() << "' doesn't have a case member selected.");
        Member* member = &const_cast<Member&>(AggregationType::member(labels_.at(label)));
        return *member;
    }

    /// \brief This method destroys previous active member, if any, and constructs the new one,
    /// setting it as active.
    void activate_member(
            uint8_t* instance,
            Member* member)
    {
        int64_t label = current_label(disc()->type(), instance);
        if (0 < labels_.count(label))
        {
            Member* active_member = &const_cast<Member&>(AggregationType::member(labels_.at(label)));
            if (active_member == member)
            {
                return;
            }
            active_member->type().destroy_instance(instance + active_member->offset());
        }

        member->type().construct_instance(instance + member->offset());
    }

    /// \brief This method converts labels represented as strings to the internal int64_t representation.
    /// It resolves Enumeration names.
    /// Doesn't resolves constants names, so they must be resolved previously.
    bool parse_labels(
            const std::vector<std::string>& labels,
            std::vector<int64_t>& result)
    {
        bool is_default = false;
        Member* disc_ = disc();
        TypeKind kind = disc_->type().kind();
        if (kind == TypeKind::ALIAS_TYPE)
        {
            const AliasType& alias = static_cast<const AliasType&>(disc_->type());
            kind = alias.rget().kind();
        }

        for (const std::string& label : labels)
        {
            if (label == "default")
            {
                xtypes_assert(!is_default, "Received two 'default' cases.");
                is_default = true;
            }
            else
            {
                int base = 10;
                if (label.find("0x") == 0 || label.find("0X") == 0)
                {
                    base = 16;
                }
                else if (label.find("0") == 0)
                {
                    base = 8;
                }

                switch (kind)
                {
                    case TypeKind::BOOLEAN_TYPE:
                    {
                        if (label == "TRUE")
                        {
                            result.emplace_back(1);
                        }
                        else if (label == "FALSE")
                        {
                            result.emplace_back(0);
                        }
                        else
                        {
                            xtypes_assert(
                                false,
                                "Received '" << label
                                             << "' while parsing a bool label. Only 'TRUE' or 'FALSE' are allowed");
                        }
                    }
                    break;
                    case TypeKind::INT_8_TYPE:
                    {
                        int8_t value = std::strtoll(label.c_str(), nullptr, base);
                        result.emplace_back(static_cast<int64_t>(value));
                    }
                    break;
                    case TypeKind::UINT_8_TYPE:
                    {
                        uint8_t value = std::strtoull(label.c_str(), nullptr, base);
                        result.emplace_back(static_cast<int64_t>(value));
                    }
                    break;
                    case TypeKind::INT_16_TYPE:
                    {
                        int16_t value = std::strtoll(label.c_str(), nullptr, base);
                        result.emplace_back(static_cast<int64_t>(value));
                    }
                    break;
                    case TypeKind::UINT_16_TYPE:
                    {
                        uint32_t value = std::strtoull(label.c_str(), nullptr, base);
                        result.emplace_back(static_cast<int64_t>(value));
                    }
                    break;
                    case TypeKind::INT_32_TYPE:
                    {
                        int32_t value = std::strtoll(label.c_str(), nullptr, base);
                        result.emplace_back(static_cast<int64_t>(value));
                    }
                    break;
                    case TypeKind::UINT_32_TYPE:
                    {
                        uint32_t value = std::strtoull(label.c_str(), nullptr, base);
                        result.emplace_back(static_cast<int64_t>(value));
                    }
                    break;
                    case TypeKind::INT_64_TYPE:
                    {
                        int64_t value = std::strtoll(label.c_str(), nullptr, base);
                        result.emplace_back(value);
                    }
                    break;
                    case TypeKind::UINT_64_TYPE:
                    {
                        uint64_t value = std::strtoull(label.c_str(), nullptr, base);
                        result.emplace_back(static_cast<int64_t>(value));
                    }
                    break;
                    case TypeKind::CHAR_8_TYPE:
                    {
                        // Check if comes with "'"
                        if (label.size() == 1)
                        {
                            result.emplace_back(static_cast<int64_t>(label[0]));
                        }
                        else
                        {
                            result.emplace_back(static_cast<int64_t>(label[label.find("'") + 1]));
                        }
                    }
                    break;
                    case TypeKind::CHAR_16_TYPE:
                    {
                        std::basic_string<XTYPES_CHAR> aux(label.begin(), label.end());
                        auto wstr = code_conversion_tool<char16_t>(aux);
                        char16_t value;
                        // Check if comes with "'"
                        if (label.size() == 1)
                        {
                            value = wstr[0];
                        }
                        else
                        {
                            value = wstr[wstr.find(u"'") + 1];
                        }
                        result.emplace_back(static_cast<int64_t>(value));
                    }
                    break;
                    case TypeKind::WIDE_CHAR_TYPE:
                    {
                        std::basic_string<XTYPES_CHAR> aux(label.begin(), label.end());
                        auto aux2 = code_conversion_tool<char16_t>(aux);
                        std::wstring temp(aux2.begin(), aux2.end());
                        wchar_t value;
                        // Check if comes with "'"
                        if (label.size() == 1)
                        {
                            value = temp[0];
                        }
                        else
                        {
                            value = temp[temp.find(L"'") + 1];
                        }
                        result.emplace_back(static_cast<int64_t>(value));
                    }
                    break;
                    case TypeKind::ENUMERATION_TYPE:
                    {
                        // TODO: If other enumeration types are added, switch again.
                        uint32_t value = std::strtoull(label.c_str(), nullptr, base);
                        if (value == 0)
                        {
                            // Check if strtoull failed because it was an string or it was really a '0'.
                            std::regex re("[_A-Za-z]");
                            std::string new_s = std::regex_replace(label, re, "*");
                            if (new_s.find("*") != std::string::npos)
                            {
                                // Get the Enum value
                                const EnumerationType<uint32_t>& enum_type =
                                        static_cast<const EnumerationType<uint32_t>&>(disc_->type());
                                value = enum_type.value(label);
                            }
                        }
                        result.emplace_back(static_cast<int64_t>(value));
                    }
                    break;
                    default:
                        xtypes_assert(false, "Internal UnionType error!");
                }
            }
        }

        return is_default;
    }

private:

    std::map<int64_t, std::string> labels_;
    size_t memory_size_;
    // Direct access
    const int64_t default_value_;
    const int64_t invalid_value_;
    int64_t default_;
    // Aux memory_size_ calculations
    size_t maximum_case_member_memory_;

    /// \brief This method retrieves the discriminator Member.
    /// It cannot be stored as a pointer (like active_member_) because the internal AggregationType vector
    /// could be resized or moved, or we being cloned, invalidating the address. Updating it in each case,
    /// could be a runtime perfomance improvement if needed.
    Member* disc() const
    {
        return &const_cast<Member&>(AggregationType::member(0));
    }

};

} //namespace xtypes
} //namespace eprosima

#endif //EPROSIMA_XTYPES_UNION_TYPE_HPP_
