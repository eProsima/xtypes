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

protected:
    AggregationType(
            TypeKind kind,
            const std::string& name)
        : DynamicType(kind, name)
    {}

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

private:
    std::map<std::string, size_t> indexes_;
    std::vector<Member> members_;
};

} //namespace xtypes
} //namespace eprosima

#endif //EPROSIMA_XTYPES_AGGREGATION_TYPE_HPP_
