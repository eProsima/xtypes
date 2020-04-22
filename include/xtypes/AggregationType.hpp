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

#ifndef EPROSIMA_AGGREGATION_TYPE_HPP_
#define EPROSIMA_AGGREGATION_TYPE_HPP_

#include <xtypes/DynamicType.hpp>
#include <xtypes/Member.hpp>
#include <xtypes/AliasType.hpp>

#include <string>
#include <map>
#include <vector>

namespace eprosima {
namespace xtypes {

/// \brief DynamicType representing an aggregation of members.
/// An AggregationType represents a TypeKind::AGGREGATION_TYPE.
class AggregationType : public DynamicType
{
public:
    /// \brief Check for a member name existence.
    /// \param[in] name Member name to check.
    /// \returns true if found.
    bool has_member(const std::string& name) const { return indexes_.count(name) != 0; }

    /// \brief Members of the aggregaton.
    /// \returns A reference to a vector of members.
    const std::vector<Member>& members() const { return members_; }

    /// \brief Get a member by index. O(1).
    /// \param[in] index Member index.
    /// \pre index < members().size()
    /// \returns A reference to the found member.
    const Member& member(size_t index) const
    {
        xtypes_assert(index < members().size(),
            "member(" << index << ") is out of bounds.");
        return members_[index];
    }

    /// \brief Get a member by name. O(log(n)).
    /// \param[in] name Member name.
    /// \pre has_member() == true
    /// \returns A reference to the found member.
    const Member& member(const std::string& name) const
    {
        xtypes_assert(has_member(name),
            "Type '" << this->name() << "' doesn't have a member named '" << name << "'.");
        return members_[indexes_.at(name)];
    }

    /// \brief Check for a parent existence.
    /// \returns true if found.
    bool has_parent() const { return parent_.get() != nullptr; }

    /// \brief Get the parent.
    /// \pre has_parent()
    /// \returns The parent type.
    const AggregationType& parent() const
    {
        xtypes_assert(has_parent(),
            "Called 'parent()' from a type without parent. Call 'has_parent()' to ensure that the "
            << "type has parent.");
        return static_cast<const AggregationType&>(*parent_);
    }

protected:
    DynamicType::Ptr parent_;

    AggregationType(
            TypeKind kind,
            const std::string& name,
            const AggregationType* p= nullptr)
        : DynamicType(kind, name)
    {
        if (p != nullptr)
        {
            parent(*p);
        }
    }

    /// \brief Insert a member into the aggregation.
    /// \param[in] member Member to add.
    /// \pre !has_member(member.name())
    /// \returns A reference to the internal member.
    Member& insert_member(const Member& member)
    {
        xtypes_assert(!has_member(member.name()),
            "Type '" << name() << "' already have a member named '" << member.name() << "'.");
        indexes_.emplace(member.name(), members_.size());
        members_.emplace_back(member);
        return members_.back();
    }

    /// \brief Set the parent
    /// \pre !has_parent() && members().size() == 0
    void parent(
            const AggregationType& p)
    {
        xtypes_assert(!has_parent(),
            "Only one parent is allowed.");
        xtypes_assert(members().size() == 0,
            "Cannot set parent to a type with already defined members.");
        if (!has_parent() && members_.size() == 0)
        {
            parent_ = DynamicType::Ptr(p);
            // Copy the parent's members directly
            for (const Member& mem : p.members())
            {
                insert_member(mem);
            }
        }
    }

private:
    std::map<std::string, size_t> indexes_;
    std::vector<Member> members_;
};

} //namespace xtypes
} //namespace eprosima

#endif //EPROSIMA_XTYPES_AGGREGATION_TYPE_HPP_
