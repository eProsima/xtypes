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
 */

#ifndef EPROSIMA_XTYPES_MAP_INSTANCE_HPP_
#define EPROSIMA_XTYPES_MAP_INSTANCE_HPP_

#include <xtypes/PairType.hpp>

#include <cstdint>
#include <cstring>

namespace eprosima {
namespace xtypes {

/// \brief Implementation of a dynamic map of DynamicTypes.
/// This class is used internal by MapType to implement its behaviour.
class MapInstance
{
public:
    /// \brief Construct a MapInstance
    /// \param[in] content Content of the map
    /// \param[in] capacity Reserved memory for the map.
    MapInstance(
            const PairType& content,
            uint32_t capacity = 0)
        : content_(content)
        , block_size_(content.memory_size())
        , capacity_(capacity)
        , memory_(capacity > 0 ? new uint8_t[capacity * block_size_] : nullptr)
        , size_(0)
    {}

    MapInstance(
            const MapInstance& other)
        : content_(other.content_)
        , block_size_(other.block_size_)
        , capacity_(other.capacity_)
        , memory_(capacity_ > 0 ? new uint8_t[capacity_ * block_size_] : nullptr)
        , size_(other.size_)
    {
        if(memory_ != nullptr)
        {
            copy_content(memory_, other.memory_);
        }
    }

    /// \brief Copy constructor from a MapInstance with other but compatible content.
    /// \param[in] other Map from copy the values.
    /// \param[in] content Content of the map
    /// \param[in] bounds Max copied elements.
    /// \pre content and other.content compatibles (see dds::core::xtypes::DynamicType::is_compatible())
    MapInstance(
            const MapInstance& other,
            const PairType& content,
            uint32_t bounds)
        : content_(content)
        , block_size_(content.memory_size())
        , capacity_(bounds == 0 ? other.capacity_ : std::min(other.capacity_, bounds))
        , memory_(capacity_ > 0 ? new uint8_t[capacity_ * block_size_] : nullptr)
        , size_(bounds == 0 ? other.size_ : std::min(other.size_, bounds))
    {
        if(memory_ != nullptr)
        {
            copy_content_from_type(memory_, other.memory_, other.content_);
        }
    }

    MapInstance(
            MapInstance&& other)
        : content_(std::move(other.content_))
        , block_size_(std::move(other.block_size_))
        , capacity_(std::move(other.capacity_))
        , memory_(std::move(other.memory_))
        , size_(std::move(other.size_))
    {
        other.memory_ = nullptr;
    }

    /// \brief Deep equality operator
    bool operator == (
            const MapInstance& other) const
    {
        if(other.size() != size_)
        {
            return false;
        }

        if(content_.first().is_constructed_type() || content_.second().is_constructed_type())
        {
            bool comp = true;
            for(uint32_t i = 0; i < size_; i++)
            {
                comp &= content_.compare_instance(memory_ + i * block_size_, other.memory_ + i * block_size_);
            }
            return comp;
        }
        else //optimization when the pair are both primitive
        {
            return std::memcmp(memory_, other.memory_, size_ * block_size_) == 0;
        }
    }

    virtual ~MapInstance()
    {
        if(memory_ != nullptr)
        {
            if(content_.is_constructed_type())
            {
                uint32_t block_size = block_size_;
                for(int32_t i = size_ - 1; i >= 0; i--)
                {
                    content_.destroy_instance(memory_ + i * block_size);
                }
            }

            delete[] memory_;
            memory_ = nullptr;
        }
    }

    /// \brief Inserts an instance into the map
    /// A reallocation can be done in order to allocate this new value.
    /// A reorder can be done in order to keep the keys in order.
    /// \param[in] instance Instance of the pair to insert into the map.
    /// \returns Returns the location of the new instance added.
    uint8_t* insert(
            const uint8_t* instance)
    {
        if(size_ == capacity_)
        {
            realloc((capacity_ > 0) ? capacity_ * 2 : 1);
        }

        uint8_t* place = create_place(instance);
        content_.copy_instance(place, instance);

        size_++;

        return place;
    }

    /// \brief Resize the map. All new elements will be default-initialized.
    /// If the new size is equal or less than the current size, nothing happens.
    /// \param[in] new_size New map size.
    void resize(
            size_t new_size)
    {
        if(size_ >= new_size)
        {
            return;
        }

        realloc(new_size);

        for(size_t i = size_; i < new_size; i++)
        {
            uint8_t* place = memory_ + size_ * block_size_;
            content_.construct_instance(place);
        }

        size_ = new_size;
    }

    /// \brief Key access operator.
    /// \param[in] key_instance Requested key
    /// \returns The pair instance with the key.
    uint8_t* operator [] (
            uint8_t* key_instance) const
    {
        return find_place(key_instance, true);
    }

    /// \brief Index access operator.
    /// \param[in] index Requested index
    /// \returns The pair at index.
    uint8_t* operator [] (
            uint32_t index) const
    {
        return get_element(index);
    }

    /// \brief Checks a key for existance
    bool contains_key(
            uint8_t* key_instance) const
    {
        return operator[](key_instance) != nullptr;
    }

    /// \brief Size of the map.
    /// \returns Size of the map.
    uint32_t size() const { return size_; }

private:
    const PairType& content_;
    uint32_t block_size_;
    uint32_t capacity_;
    uint8_t* memory_;
    uint32_t size_;

    void realloc(size_t new_capacity)
    {
        uint8_t* new_memory = new uint8_t[new_capacity * block_size_];

        move_content(new_memory, memory_);

        delete[] memory_;
        memory_ = new_memory;
        capacity_ = new_capacity;
    }

    void copy_content(
            uint8_t* target,
            const uint8_t* source) const
    {
        if(content_.is_constructed_type())
        {
            for(uint32_t i = 0; i < size_; i++)
            {
                content_.copy_instance(target + i * block_size_, source + i * block_size_);
            }
        }
        else //optimization when the type is primitive
        {
            std::memcpy(target, source, size_ * block_size_);
        }
    }

    void copy_content_from_type(
            uint8_t* target,
            const uint8_t* source,
            const PairType& other_content) const
    {
        size_t other_first_size = other_content.first().memory_size();
        size_t other_second_size = other_content.second().memory_size();
        if(content_.first().is_constructed_type()
            || content_.second().is_constructed_type()
            || content_.first().memory_size() != other_first_size
            || content_.second().memory_size() != other_second_size)
        {
            for(uint32_t i = 0; i < size_; i++)
            {
                content_.copy_instance_from_type(
                        target + i * block_size_,
                        source + i * other_content.memory_size(),
                        other_content);
            }
        }
        else //optimization when the pair are both primitive with same size
        {
            std::memcpy(target, source, size_ * block_size_);
        }
    }

    void move_content(
            uint8_t* target,
            uint8_t* source,
            bool overlap = false)
    {
        if(content_.first().is_constructed_type() || content_.second().is_constructed_type())
        {
            if (overlap)
            {
                for(uint32_t i = size_ - 1; i >= 0; --i)
                {
                    content_.move_instance(target + i * block_size_, source + i * block_size_);
                }
            }
            else
            {
                for(uint32_t i = 0; i < size_; ++i)
                {
                    content_.move_instance(target + i * block_size_, source + i * block_size_);
                }
            }
        }
        else //optimization when the pair are both primitive
        {
            std::memmove(target, source, size_ * block_size_);
        }
    }

    uint8_t* create_place(
            const uint8_t* instance)
    {
        uint8_t* place = find_place(instance);
        xtypes_assert(hash(instance) != hash(place), "Key already exists.");
        move_content(place + block_size_, place, true);
        return place;
    }

    uint8_t* find_place(
            const uint8_t* instance,
            bool exact = false) const
    {
        // Binary search by hash
        uint64_t istart = 0;
        uint64_t iend = size_;
        uint64_t icurrent = (istart + iend) / 2;
        uint64_t instance_hash = hash(instance);
        uint8_t* start = get_element(0);
        uint8_t* end = get_element(size_);
        uint8_t* current = get_element(icurrent);

        while (icurrent != istart)
        {
            uint64_t current_hash = hash(current);
            if (instance_hash == current_hash)
            {
                return current;
            }
            else if (instance_hash > current_hash)
            {
                start = current;
                istart = icurrent;
            }
            else
            {
                end = current;
                iend = icurrent;
            }

            icurrent = (istart + iend) / 2;
            current = get_element(icurrent);
        }

        if (exact && hash(current) != instance_hash)
        {
            return nullptr;
        }
        return current;
    }

    uint8_t* get_element(
            size_t index) const
    {
        return memory_ + index * block_size_;
    }

    uint64_t hash(
            const uint8_t* instance) const
    {
        return hash64(instance, content_.first().memory_size(), 0x2145654af87a5b6dULL);
    }

    // Hash function based on fasthash (https://github.com/ZilongTan/fast-hash)
    uint64_t& mix(
            uint64_t& h) const
    {
        h ^= (h >> 23);
        h *= 0x2127599bf4325c37ULL;
        h ^= (h >> 47);
        return h;
    }

    uint64_t hash64(
            const uint8_t* buf,
            size_t len,
            uint64_t seed) const
    {
        const uint64_t m = 0x880355f21e6d1965ULL;
        uint64_t* pos = reinterpret_cast<uint64_t*>(const_cast<uint8_t*>(buf));
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

        pos2 = reinterpret_cast<uint8_t*>(pos);
        v = 0;

        switch (len & 7)
        {
        case 7: v ^= static_cast<uint64_t>(pos2[6]) << 48;
        case 6: v ^= static_cast<uint64_t>(pos2[6]) << 48;
        case 5: v ^= static_cast<uint64_t>(pos2[6]) << 48;
        case 4: v ^= static_cast<uint64_t>(pos2[6]) << 48;
        case 3: v ^= static_cast<uint64_t>(pos2[6]) << 48;
        case 2: v ^= static_cast<uint64_t>(pos2[6]) << 48;
        case 1: v ^= static_cast<uint64_t>(pos2[6]) << 48;
            h ^= mix(v);
            h *= m;
        }

        return mix(h);
    }
};

} //namespace xtypes
} //namespace eprosima

#endif //EPROSIMA_XTYPES_MAP_INSTANCE_HPP_
