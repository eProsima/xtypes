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

#ifndef EPROSIMA_ENUMERATED_TYPE_HPP_
#define EPROSIMA_ENUMERATED_TYPE_HPP_

#include <xtypes/PrimitiveType.hpp>

#include <string>
#include <map>

namespace eprosima {
namespace xtypes {

/// \brief DynamicType representing an enumerated type.
/// An EnumeratedType represents a TypeKind::ENUMERATED_TYPE.
template<typename T>
class EnumeratedType : public PrimitiveType<T>
{
public:
    /// \brief Check for a enumerator name existence.
    /// \param[in] name enumerator name to check.
    /// \returns true if found.
    bool has_enumerator(const std::string& name) const { return values_.count(name) != 0; }

    /// \brief Enumerators of the enumerator.
    /// \returns A reference to the map of enumerators.
    const std::map<std::string, T>& enumerators() const { return values_; }

    /// \brief Get a enumerator value by name. O(log(n)).
    /// \param[in] name EnumMember name.
    /// \pre has_enumerator() == true
    /// \returns A reference to the found enumerator.
    T value(const std::string& name) const
    {
        xtypes_assert(has_enumerator(name),
            "Type '" + this->name() + "' doesn't have an enumerator named '" + name
            + "'. Use 'has_enumerator' function to ensure the asked enumerator exists.");
        return values_.at(name);
    }

    bool is_allowed_value(T value) const
    {
        for (const auto& pair : values_)
        {
            if (pair.second == value)
            {
                return true;
            }
        }
        return false;
    }

    /// \brief Returns the enumerator for the given value.
    /// \returns The enumerator as std::string if exists, empty is doesn't.
    std::string enumerator(T value) const
    {
        for (const auto& pair : values_)
        {
            if (pair.second == value)
            {
                return pair.first;
            }
        }
        return "";
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

        if (other.is_enumerated_type())
        {
            if (PrimitiveType<T>::memory_size() == other.memory_size())
            {
                return TypeConsistency::EQUALS;
            }

            return TypeConsistency::IGNORE_TYPE_WIDTH;
        }

        if(!other.is_primitive_type())
        {
            return TypeConsistency::NONE;
        }

        TypeConsistency consistency = TypeConsistency::EQUALS;
        if(PrimitiveType<T>::memory_size() != other.memory_size())
        {
            consistency |= TypeConsistency::IGNORE_TYPE_WIDTH;
        }

        if((other.kind() & TypeKind::UNSIGNED_TYPE) == TypeKind::NO_TYPE)
        {
            consistency |= TypeConsistency::IGNORE_TYPE_SIGN;
        }
        return consistency;
    }

    EnumeratedType(
            TypeKind kind,
            const std::string& name)
        : PrimitiveType<T>(typename PrimitiveType<T>::use_function_primitive_type{}, kind, name)
    {
    }

protected:

    std::shared_ptr<DynamicType> clone() const override
    {
        auto clon = std::make_shared<EnumeratedType>(this->kind(), this->name());
        clon->values_ = values_;
        return clon;
    }

    /// \brief Insert a enumerator into the enumerated.
    /// \param[in] name enumerator identifier to add.
    /// \param[in] value value of the enumerator to add.
    /// \pre !has_enumerator(name)
    /// \returns A reference to the EnumeratedType (this).
    EnumeratedType& insert_enumerator(const std::string& name, T value)
    {
        xtypes_assert(!has_enumerator(name),
            "Type '" + this->name() + "' already has an enumerator named '" + name + "'.");
        values_.emplace(name, value);
        return *this;
    }

    std::map<std::string, T> values_;
};

} //namespace xtypes
} //namespace eprosima

#endif //EPROSIMA_XTYPES_ENUMERATED_TYPE_HPP_
