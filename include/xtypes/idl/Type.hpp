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

#ifndef EPROSIMA_XTYPES_IDL_TYPE_HPP_
#define EPROSIMA_XTYPES_IDL_TYPE_HPP_

#include <xtypes/DynamicType.hpp>

namespace eprosima {
namespace xtypes {
namespace idl {

class Module;

class Type
{
public:

    Type(
            Module& parent,
            const DynamicType& type)
        : type_(type)
        , parent_(parent)
    {
    }

    Type(
            Module& parent,
            DynamicType&& type)
        : type_(std::move(type))
        , parent_(parent)
    {
    }

    Type(
            Type&& type) = default;

    Module& parent() const
    {
        return parent_;
    }

    DynamicType::Ptr& get()
    {
        return type_;
    }

    const DynamicType::Ptr& get() const
    {
        return type_;
    }

    const DynamicType& operator * () const
    {
        return *type_;
    }

    const DynamicType* operator -> () const
    {
        return type_.get();
    }

private:

    Type(
            const Type& type) = delete;

    DynamicType::Ptr type_;

    Module& parent_;

};

} // namespace idl
} // namespace xtypes
} // namespace eprosima

#endif // EPROSIMA_XTYPES_IDL_TYPE_HPP_
