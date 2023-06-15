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
 */

#ifndef EPROSIMA_XTYPES_ALIAS_TYPE_HPP_
#define EPROSIMA_XTYPES_ALIAS_TYPE_HPP_

#include <xtypes/DynamicType.hpp>

namespace eprosima {
namespace xtypes {

class AliasType : public DynamicType
{
public:

    AliasType(
            const DynamicType::Ptr& aliased,
            const std::string& name)
        : DynamicType(TypeKind::ALIAS_TYPE, name)
        , aliased_(aliased)
    {
    }

    AliasType(
            const DynamicType::Ptr&& aliased,
            const std::string& name)
        : DynamicType(TypeKind::ALIAS_TYPE, name)
        , aliased_(std::move(aliased))
    {
    }

    AliasType(
            const AliasType& aliased,
            const std::string& name)
        : DynamicType(TypeKind::ALIAS_TYPE, name)
        , aliased_(aliased)
    {
    }

    AliasType(
            const AliasType& other) = default;
    AliasType(
            AliasType&& other) = default;

    virtual size_t memory_size() const override
    {
        return aliased_->memory_size();
    }

    virtual void construct_instance(
            uint8_t* instance) const override
    {
        aliased_->construct_instance(instance);
    }

    virtual void copy_instance(
            uint8_t* target,
            const uint8_t* source) const override
    {
        aliased_->copy_instance(target, source);
    }

    virtual void copy_instance_from_type(
            uint8_t* target,
            const uint8_t* source,
            const DynamicType& other) const override
    {
        aliased_->copy_instance_from_type(target, source, other);
    }

    virtual void move_instance(
            uint8_t* target,
            uint8_t* source,
            bool initialized) const override
    {
        aliased_->move_instance(target, source, initialized);
    }

    virtual void destroy_instance(
            uint8_t* instance) const override
    {
        aliased_->destroy_instance(instance);
    }

    virtual bool compare_instance(
            const uint8_t* instance,
            const uint8_t* other_instance) const override
    {
        return aliased_->compare_instance(instance, other_instance);
    }

    virtual void for_each_instance(
            const InstanceNode& node,
            InstanceVisitor visitor) const override
    {
        InstanceNode alias(*node.parent, *aliased_, node.instance, node.from_index, node.from_member);
        aliased_->for_each_instance(alias, visitor);
    }

    virtual TypeConsistency is_compatible(
            const DynamicType& other) const override
    {
        TypeConsistency consistency = TypeConsistency::EQUALS;
        if (other.kind() == TypeKind::ALIAS_TYPE)
        {
            const AliasType& other_alias = static_cast<const AliasType&>(other);
            consistency |= rget().is_compatible(other_alias.rget());
        }
        else
        {
            consistency = rget().is_compatible(other);
        }

        return consistency;
    }

    virtual void for_each_type(
            const TypeNode& node,
            TypeVisitor visitor) const override
    {
        return aliased_->for_each_type(node, visitor);
    }

    const DynamicType& get() const
    {
        return *aliased_;
    }

    const DynamicType* operator -> () const
    {
        return aliased_.get();
    }

    const DynamicType& operator * () const
    {
        return *aliased_;
    }

    const DynamicType& rget() const
    {
        if (aliased_->kind() == TypeKind::ALIAS_TYPE)
        {
            return static_cast<const AliasType&>(*aliased_).rget();
        }
        return *aliased_;
    }

    template <typename T>
    operator const T& () const
    {
        std::stringstream err;
        err << "Alias [" << name() << "] cannot be cast to the specified type: ["
            << typeid(T).name() << "].";
        const T* t =  dynamic_cast<const T*>(aliased_.get());
        xtypes_assert(t != nullptr, err.str());
        return *t;
    }

    virtual uint64_t hash(
            const uint8_t* instance) const override
    {
        return aliased_->hash(instance);
    }

protected:

    std::shared_ptr<DynamicType> clone() const override
    {
        return std::make_shared<AliasType>(*this);
    }

private:

    DynamicType::Ptr aliased_;
};

} //namespace xtypes
} //namespace eprosima

#endif //EPROSIMA_XTYPES_ALIAS_TYPE_HPP_
