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

#ifndef EPROSIMA_XTYPES_MAP_TYPE_HPP_
#define EPROSIMA_XTYPES_MAP_TYPE_HPP_

#include <xtypes/MutableCollectionType.hpp>
#include <xtypes/MapInstance.hpp>
#include <xtypes/StructType.hpp>

#include <vector>

namespace eprosima {
namespace xtypes {

/// \brief DynamicType representing mutable map of elements.
/// A MapType represents a TypeKind::MAP_TYPE.
class MapType : public MutableCollectionType
{
public:

    /// \brief Construct a MapType.
    /// \param[in] key Key type of the map.
    /// \param[in] content Content type of the map.
    /// \param[in] bounds Size limit of the map, 0 means that no limits.
    explicit MapType(
            const DynamicType& key,
            const DynamicType& content,
            uint32_t bounds = 0)
        : MutableCollectionType(
            TypeKind::MAP_TYPE,
            "map_" + ((bounds > 0) ? std::to_string(bounds) + "_" : "") + PairType::name(key, content),
            DynamicType::Ptr(PairType(key, content)),
            bounds)
    {
    }

    /// \brief Construct a MapType.
    /// \param[in] key Key type of the map.
    /// \param[in] content Content type of the map.
    /// \param[in] bounds Size limit of the map, 0 means that no limits.
    template<typename DynamicTypeImpl>
    MapType(
            const DynamicTypeImpl&& key,
            const DynamicTypeImpl&& content,
            uint32_t bounds)
        : MutableCollectionType(
            TypeKind::MAP_TYPE,
            "map_" + ((bounds > 0) ? std::to_string(bounds) + "_" : "") + PairType::name(key, content),
            DynamicType::Ptr(PairType(key, content)),
            bounds)
    {
    }

    MapType(
            const MapType& other) = default;
    MapType(
            MapType&& other) = default;

    size_t memory_size() const override
    {
        return sizeof(MapInstance);
    }

    virtual void construct_instance(
            uint8_t* instance) const override
    {
        new (instance) MapInstance(static_cast<const PairType&>(content_type()), bounds());
    }

    virtual void copy_instance(
            uint8_t* target,
            const uint8_t* source) const override
    {
        new (target) MapInstance(*reinterpret_cast<const MapInstance*>(source));
    }

    virtual void copy_instance_from_type(
            uint8_t* target,
            const uint8_t* source,
            const DynamicType& arg_other) const override
    {
        const DynamicType& other = (arg_other.kind() == TypeKind::ALIAS_TYPE)
                ? static_cast<const AliasType&>(arg_other).rget()
                : arg_other;

        if (other.kind() == TypeKind::STRUCTURE_TYPE)
        {
            // Resolve one-member struct compatibility
            const StructType& struct_type = static_cast<const StructType&>(other);
            if (struct_type.members().size() == 1)
            {
                copy_instance_from_type(target, source, struct_type.members().at(0).type());
                return;
            }
        }

        xtypes_assert(
            other.kind() == TypeKind::MAP_TYPE
            && content_type().name() == static_cast<const MapType&>(other).content_type().name(),
            "Cannot copy data from different types: From '" << other.name() << "' to '" << name() << "'.");

        (void) other;
        new (target) MapInstance(*reinterpret_cast<const MapInstance*>(source), bounds());
    }

    virtual void move_instance(
            uint8_t* target,
            uint8_t* source,
            bool initialized) const override
    {
        if (initialized)
        {
            destroy_instance(target);
        }
        new (target) MapInstance(std::move(*reinterpret_cast<const MapInstance*>(source)));
    }

    virtual void destroy_instance(
            uint8_t* instance) const override
    {
        reinterpret_cast<MapInstance*>(instance)->~MapInstance();
    }

    virtual uint8_t* get_instance_at(
            uint8_t* instance,
            size_t index) const override
    {
        return reinterpret_cast<MapInstance*>(instance)->operator [](uint32_t(index));
    }

    virtual uint8_t* get_instance_at(
            uint8_t* instance,
            uint8_t* key_instance) const
    {
        return reinterpret_cast<MapInstance*>(instance)->operator [](key_instance);
    }

    virtual size_t get_instance_size(
            const uint8_t* instance) const override
    {
        return reinterpret_cast<const MapInstance*>(instance)->size();
    }

    virtual bool compare_instance(
            const uint8_t* instance,
            const uint8_t* other_instance) const override
    {
        return *reinterpret_cast<const MapInstance*>(instance)
               == *reinterpret_cast<const MapInstance*>(other_instance);
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

        if (other.kind() != TypeKind::MAP_TYPE)
        {
            return TypeConsistency::NONE;
        }

        const MapType& other_map = static_cast<const MapType&>(other);

        if (bounds() == other_map.bounds())
        {
            return TypeConsistency::EQUALS
                   | content_type().is_compatible(other_map.content_type());
        }

        return TypeConsistency::IGNORE_MAP_BOUNDS
               | content_type().is_compatible(other_map.content_type());
    }

    virtual void for_each_instance(
            const InstanceNode& node,
            InstanceVisitor visitor) const override
    {
        const MapInstance& map = *reinterpret_cast<const MapInstance*>(node.instance);
        visitor(node);
        for (uint32_t i = 0; i < map.size(); i++)
        {
            InstanceNode child(node, content_type(), map[i], i, nullptr);
            content_type().for_each_instance(child, visitor);
        }
    }

    virtual void for_each_type(
            const TypeNode& node,
            TypeVisitor visitor) const override
    {
        visitor(node);
        TypeNode child(node, content_type(), 0, nullptr);
        content_type().for_each_type(child, visitor);
    }

    /// \brief Push a value to a map instance.
    /// \param[in, out] instance Memory instance representing a MapInstance.
    /// \param[in] new key instance to add into the map.
    /// \returns a instance location representing the new value added
    /// or nullptr if the map reach the limit.
    uint8_t* insert_instance(
            uint8_t* instance,
            const uint8_t* key_instance) const
    {
        if (get_instance_size(instance) < bounds() || bounds() == 0)
        {
            return reinterpret_cast<MapInstance*>(instance)->insert(key_instance, bounds());
        }
        return nullptr;
    }

    /// \brief checks if a key is contained in the map.
    bool has_key(
            uint8_t* instance,
            uint8_t* key) const
    {
        return reinterpret_cast<MapInstance*>(instance)->contains_key(key);
    }

    virtual uint64_t hash(
            const uint8_t* c_instance) const override
    {
        uint8_t* instance = const_cast<uint8_t*>(c_instance);
        return reinterpret_cast<MapInstance*>(instance)->map_hash();
    }

protected:

    std::shared_ptr<DynamicType> clone() const override
    {
        return std::make_shared<MapType>(*this);
    }

};

} //namespace xtypes
} //namespace eprosima

#endif //EPROSIMA_XTYPES_MAP_TYPE_HPP_
