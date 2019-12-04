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

#ifndef EPROSIMA_ENUMERATION_TYPE_HPP_
#define EPROSIMA_ENUMERATION_TYPE_HPP_

#include <xtypes/EnumeratedType.hpp>

namespace eprosima {
namespace xtypes {

/// \brief DynamicType representing an enumeration type.
/// An EnumerationType represents a TypeKind::ENUMERATED_TYPE.
template<typename T>
class EnumerationType : public EnumeratedType<T>
{
public:
    EnumerationType& add_enumerator(
            const std::string& name,
            T value)
    {
        // New values must be always greater than the previous ones
        xtypes_assert(value >= next_value_,
            "Expected a value greater than " + std::to_string(next_value_) + " but received " + std::to_string(value));
        EnumeratedType<T>::insert_enumerator(name, value);
        next_value_ = value + 1;
        return *this;
    }

    EnumerationType& add_enumerator(
            const std::string& name)
    {
        return add_enumerator(name, next_value_);
    }

    T default_value() const
    {
        // TODO using @default tag
        if (EnumeratedType<T>::values_.empty())
        {
            return T();
        }
        else
        {
            auto it = EnumeratedType<T>::values_.begin();
            return it.second;
        }
    }

    EnumerationType(
            const std::string& name)
        : EnumeratedType<T>(TypeKind::ENUMERATION_TYPE, name)
    {
        static_assert(std::is_same<uint8_t, T>::value ||
            std::is_same<uint16_t, T>::value ||
            std::is_same<uint32_t, T>::value, "EnumerationType can only by bound to uint8_t, uint16_t or uint32_t.");
    }

protected:
    T next_value_ = T();

};

} //namespace xtypes
} //namespace eprosima

#endif //EPROSIMA_XTYPES_ENUMERATION_TYPE_HPP_
