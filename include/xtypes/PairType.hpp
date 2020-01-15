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

#ifndef EPROSIMA_XTYPES_PAIR_TYPE_HPP_
#define EPROSIMA_XTYPES_PAIR_TYPE_HPP_

#include <xtypes/DynamicType.hpp>

namespace eprosima {
namespace xtypes {

class PairType : public DynamicType
{
public:
    /// \brief Construct a PairType
    PairType(
            const DynamicType& first,
            const DynamicType& second)
        : DynamicType (TypeKind::PAIR_TYPE, "pair_" + first.name() + "_" + second.name())
        , first_(first)
        , second_(second)
    {
    }

    static std::string name(
            const DynamicType& first,
            const DynamicType& second)
    {
        return "pair_" + first.name() + "_" + second.name();
    }

    const DynamicType& first() const
    {
        return *first_;
    }

    const DynamicType& second() const
    {
        return *second_;
    }

    void first(const DynamicType& first)
    {
        first_ = DynamicType::Ptr(first);
    }

    void first(DynamicType&& first)
    {
        first_ = DynamicType::Ptr(std::move(first));
    }

    void second(const DynamicType& second)
    {
        second_ = DynamicType::Ptr(second);
    }

    void second(DynamicType&& second)
    {
        second_ = DynamicType::Ptr(std::move(second));
    }

    virtual size_t memory_size() const override
    {
        return first_->memory_size() + second_->memory_size();
    }

    virtual void construct_instance(
            uint8_t* instance) const override
    {
        first_->construct_instance(instance);
        second_->construct_instance(instance + first_->memory_size());
    }

    virtual void copy_instance(
            uint8_t* target,
            const uint8_t* source) const override
    {
        first_->copy_instance(target, source);
        second_->copy_instance(target + first_->memory_size(), source + first_->memory_size());
    }

    virtual void copy_instance_from_type(
            uint8_t* target,
            const uint8_t* source,
            const DynamicType& other) const override
    {
        first_->copy_instance_from_type(target, source, other);
        second_->copy_instance_from_type(target + first_->memory_size(), source + first_->memory_size(), other);
    }

    virtual void move_instance(
            uint8_t* target,
            uint8_t* source) const override
    {
        first_->move_instance(target, source);
        second_->move_instance(target + first_->memory_size(), source + first_->memory_size());
    }

    virtual void destroy_instance(
            uint8_t* instance) const override
    {
        first_->destroy_instance(instance);
        second_->destroy_instance(instance + first_->memory_size());
    }

    virtual bool compare_instance(
            const uint8_t* instance,
            const uint8_t* other_instance) const override
    {
        return
            first_->compare_instance(instance, other_instance) &&
            second_->compare_instance(instance + first_->memory_size(), other_instance + first_->memory_size());
    }

    virtual TypeConsistency is_compatible(
            const DynamicType& other) const override
    {
        bool check = other.kind() == TypeKind::PAIR_TYPE;

        if (check)
        {
            const PairType& pair = static_cast<const PairType&>(other);
            return first_->is_compatible(pair.first()) | second_->is_compatible(pair.second());
        }
        return TypeConsistency::NONE;
    }

    virtual void for_each_type(
            const TypeNode& node,
            TypeVisitor visitor) const override
    {
        visitor(node);
        TypeNode f(node, first(), 0, nullptr);
        first_->for_each_type(f, visitor);
        TypeNode s(node, second(), 1, nullptr);
        second_->for_each_type(s, visitor);
    }

    virtual void for_each_instance(
            const InstanceNode& node,
            InstanceVisitor visitor) const override
    {
        visitor(node);
        InstanceNode f(node, first(), node.instance, 0, nullptr);
        first_->for_each_instance(f, visitor);
        InstanceNode s(node, second(), node.instance + first_->memory_size(), 1, nullptr);
        second_->for_each_instance(s, visitor);
    }

protected:
    DynamicType::Ptr first_;
    DynamicType::Ptr second_;

    virtual DynamicType* clone() const override
    {
        return new PairType(*this);
    }

};

} // namespace xtypes
} // namespace eprosima

#endif // EPROSIMA_XTYPES_PAIR_TYPE_HPP_
