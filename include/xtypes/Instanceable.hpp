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

#ifndef EPROSIMA_XTYPES_INSTANCEABLE_TYPE_HPP_
#define EPROSIMA_XTYPES_INSTANCEABLE_TYPE_HPP_

#include <cstdint>
#include <cstddef>
#include <functional>

namespace eprosima {
namespace xtypes {

class DynamicType;
class Member;

/// \brief Abstract class for all instanceable types.
/// It contains the needed declarations to manage the instance creation and destruction.
class Instanceable
{
public:

    virtual ~Instanceable() = default;

    /// \brief Calculate the memory size that the instance will occupy in memory.
    /// \returns Instance memory size.
    virtual size_t memory_size() const = 0;

    /// \brief Constructs an instance in memory.
    /// \param[out] instance Location where the instance will be constructed.
    virtual void construct_instance(
            uint8_t* instance) const = 0;

    /// \brief Copy construction of a instance.
    /// \param[out] target Location where the instance will be constructed.
    /// \param[in] source Location from the instance will be copied.
    virtual void copy_instance(
            uint8_t* target,
            const uint8_t* source) const = 0;

    /// \brief Copy construction of a instance from another type.
    /// \pre other type needs to be compatible with the current type
    /// (see DynamicType::is_compatible() function).
    /// \param[out] target Location where the instance will be constructed.
    /// \param[in] source Location from the instance will be copied.
    /// \param[in] other Type representing the source instance.
    virtual void copy_instance_from_type(
            uint8_t* target,
            const uint8_t* source,
            const DynamicType& other) const = 0;

    /// \brief Move construction of a instance from another type.
    /// \post source instance will be invalidated.
    /// \param[out] target Location where the instance will be constructed.
    /// \param[in, out] source Location from the instance will be moved.
    /// \param[in], initialized Indicated whether target was previously initialized.
    virtual void move_instance(
            uint8_t* target,
            uint8_t* source,
            bool initialized) const = 0;

    /// \brief Destroy an instance.
    /// \param[in, out] instance Location where the instance to be removed is placed.
    virtual void destroy_instance(
            uint8_t* instance) const = 0;

    /// \brief Deep equality comparation of 2 instances.
    /// \pre the instances must represent the same DynamicType.
    /// \param[in] instance first instance to be checked.
    /// \param[in] instance second instance to be checked.
    /// \returns true if both instance are equals.
    virtual bool compare_instance(
            const uint8_t* instance,
            const uint8_t* other_instance) const = 0;

    /// \brief Internal structure used to iterate the instance tree.
    struct InstanceNode
    {
        const InstanceNode* parent;
        const DynamicType& type;
        uint8_t* instance;
        size_t deep;
        size_t from_index;
        const Member* from_member;

        InstanceNode(
                const DynamicType& type,
                uint8_t* instance)
            : parent(nullptr)
            , type(type)
            , instance(instance)
            , deep(0)
            , from_index(0)
            , from_member(nullptr)
        {
        }

        InstanceNode(
                const InstanceNode& parent,
                const DynamicType& type,
                uint8_t* instance,
                size_t from_index,
                const Member* from_member)
            : parent(&parent)
            , type(type)
            , instance(instance)
            , deep(parent.deep + 1)
            , from_index(from_index)
            , from_member(from_member)
        {
        }

    };

    using InstanceVisitor = std::function<void (const InstanceNode& node)>;

    /// \brief Function used to iterate the instance tree.
    /// The iteration will go through the tree in deep, calling the visitor function for each instance type.
    /// \param[in] node Relative information about the current instance iteration.
    /// \param[in] visitor Function called each time a new node in the tree is visited.
    virtual void for_each_instance(
            const InstanceNode& node,
            InstanceVisitor visitor) const = 0;

    virtual uint64_t hash(
            const uint8_t* instance) const
    {
        // Default implementation valid for non_constructed types.
        return hash64(instance, memory_size(), 0x2145654af87a5b6dULL);
    }

    static inline uint64_t hash64(
            const uint8_t* buf,
            size_t len,
            uint64_t seed)
    {
        const uint64_t m = 0x880355f21e6d1965ULL;
        const uint64_t* pos = reinterpret_cast<const uint64_t*>(buf);
        const uint64_t* end = pos + (len / 8);
        const uint8_t* pos2;
        uint64_t h = seed ^ (len * m);
        uint64_t v;

        while (pos != end)
        {
            v = *pos++;
            h ^= mix(v);
            h *= m;
        }

        pos2 = reinterpret_cast<const uint8_t*>(pos);
        v = 0;

        switch (len & 7)
        {
            case 7: v ^= static_cast<uint64_t>(pos2[6]) << 48; [[fallthrough]];
            case 6: v ^= static_cast<uint64_t>(pos2[5]) << 40; [[fallthrough]];
            case 5: v ^= static_cast<uint64_t>(pos2[4]) << 32; [[fallthrough]];
            case 4: v ^= static_cast<uint64_t>(pos2[3]) << 24; [[fallthrough]];
            case 3: v ^= static_cast<uint64_t>(pos2[2]) << 16; [[fallthrough]];
            case 2: v ^= static_cast<uint64_t>(pos2[1]) <<  8; [[fallthrough]];
            case 1: v ^= static_cast<uint64_t>(pos2[0]);
                h ^= mix(v);
                h *= m;
        }

        return mix(h);
    }

    // Similar to boost::hash_combine
    static void hash_combine(
            uint64_t& result,
            const uint64_t& hash)
    {
        result ^= hash + 0x9e3779b9 + (result << 6) + (result >> 2);
    }

protected:

    Instanceable() = default;

private:

    // Hash function based on fasthash (https://github.com/ZilongTan/fast-hash)
    static inline uint64_t& mix(
            uint64_t& h)
    {
        h ^= (h >> 23);
        h *= 0x2127599bf4325c37ULL;
        h ^= (h >> 47);
        return h;
    }

};

} //namespace xtypes
} //namespace eprosima

#endif //EPROSIMA_XTYPES_INSTANCIABLE_TYPE_HPP_
