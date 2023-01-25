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
        : block_size_(content.memory_size())
        , capacity_(capacity)
        , size_(0)
    {
        std::shared_ptr<const DynamicType> tmp;
        try
        {
            tmp = (content.shared_from_this());
        }
        catch(const std::bad_weak_ptr&)
        {
            tmp = content.clone();
        }

        content_ = std::static_pointer_cast<const PairType>(std::move(tmp));

        init_memory(memory_, capacity_);
    }

    MapInstance(
            const MapInstance& other)
        : content_(other.content_)
        , block_size_(other.block_size_)
        , capacity_(other.capacity_)
        , size_(other.size_)
    {
        init_memory(memory_, capacity_);

        if (memory_ != nullptr)
        {
            copy_content(other, size_);
        }
    }

    /// \brief Copy constructor from a MapInstance with other but compatible content.
    /// \param[in] other Map from copy the values.
    /// \param[in] bounds Max copied elements.
    /// \pre content and other.content compatibles (see dds::core::xtypes::DynamicType::is_compatible())
    MapInstance(
            const MapInstance& other,
            uint32_t bounds)
        : content_(other.content_)
        , block_size_(content_->memory_size())
        , capacity_(bounds == 0 ? other.capacity_ : std::min(other.capacity_, bounds))
        , size_(bounds == 0 ? other.size_ : std::min(other.size_, bounds))
    {
        init_memory(memory_, capacity_);

        if (memory_ != nullptr)
        {
            copy_content(other, size_);
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
        if (other.size() != size_)
        {
            return false;
        }

        if (content_->first().is_constructed_type() || content_->second().is_constructed_type())
        {
            bool comp = true;
            for (uint32_t i = 0; i < size_; i++)
            {
                comp &= content_->compare_instance(memory_ + i * block_size_, other.memory_ + i * block_size_);
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
        free_memory();
    }

    /// \brief Inserts a key instance into the map, allocating the needed space.
    /// A reallocation can be done in order to allocate this new value.
    /// A reorder can be done in order to keep the keys in order.
    /// \param[in] instance Instance of the pair to insert into the map.
    /// \returns Returns the location of the new instance added.
    uint8_t* insert(
            const uint8_t* instance,
            uint32_t bounds)
    {
        if (memory_ == nullptr || size_ == capacity_)
        {
            realloc((capacity_ > 0) ? capacity_ * 2 : 1, bounds);
        }

        uint8_t* place = create_place(instance);
        content_->first().copy_instance(place, instance);
        content_->second().construct_instance(place + content_->first().memory_size());

        size_++;

        return place;
    }

    /// \brief Key access operator.
    /// \param[in] key_instance Requested key
    /// \returns The pair instance with the key.
    uint8_t* operator [] (
            const uint8_t* key_instance) const
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
            const uint8_t* key_instance) const
    {
        return operator [](key_instance) != nullptr;
    }

    size_t index_of(
            const uint8_t* key_instance) const
    {
        return get_key_index(key_instance);
    }

    /// \brief Size of the map.
    /// \returns Size of the map.
    uint32_t size() const
    {
        return size_;
    }

    uint64_t map_hash() const
    {
        if (size_ > 0)
        {
            uint64_t h = content_->hash(memory_);
            for (uint32_t i = 1; i < size_; ++i)
            {
                Instanceable::hash_combine(h, content_->hash(get_element(i)));
            }
            return h;
        }
        return 0;
    }

private:

    friend class MapType;

    std::shared_ptr<const PairType> content_;
    uint32_t block_size_ = 0;
    uint32_t capacity_ = 0;
    uint8_t* memory_ = nullptr;
    uint32_t size_ = 0;

    void realloc(
            size_t desired_capacity,
            uint32_t bounds)
    {
        size_t new_capacity = bounds == 0 ? desired_capacity : std::min(desired_capacity, size_t(bounds));
        uint8_t* new_memory = nullptr;
        init_memory(new_memory, new_capacity);

        move_content(new_memory, memory_, true);

        free_memory();
        memory_ = new_memory;
        capacity_ = new_capacity;
    }

    void init_memory(
            uint8_t*& memory,
            uint32_t size)
    {
        if (memory == nullptr || size != capacity_)
        {
            if (memory != nullptr)
            {
                free_memory();
            }
            memory = size > 0 ? new uint8_t[size * block_size_] : nullptr;
            if (memory != nullptr)
            {
                memset(memory, 0, size * block_size_);
                for (uint32_t idx = 0; idx < size; ++idx)
                {
                    content_->construct_instance(memory + idx * block_size_);
                }
            }
        }
    }

    void copy_content(
            const MapInstance& other,
            uint32_t bounds)
    {
        size_t other_first_size = other.content_->first().memory_size();
        size_t other_second_size = other.content_->second().memory_size();

        // Check bytes to copy
        uint32_t min_capacity = std::min(capacity_, other.capacity_);
        if (min_capacity == 0)
        {
            min_capacity = std::max(capacity_, other.capacity_);
        }
        uint32_t min_size = min_capacity == 0 ? other.size_ : std::min(min_capacity, other.size_);

        if (bounds != 0)
        {
            // Keep in mind our bounds
            min_size = std::min(min_size, bounds);
        }

        if (memory_ == nullptr || // Unbounded maps could reach this point without reserving memory yet.
                min_size > capacity_)
        {
            realloc(min_size, bounds);
        }

        if (content_->first().is_constructed_type()
                || content_->second().is_constructed_type()
                || content_->first().memory_size() != other_first_size
                || content_->second().memory_size() != other_second_size)
        {
            for (uint32_t i = 0; i < min_size; i++)
            {
                content_->copy_instance_from_type(
                    memory_ + i * block_size_,
                    other.memory_ + i * other.content_->memory_size(),
                    *other.content_);
            }
        }
        else //optimization when the pair are both primitive with same size
        {
            std::memcpy(memory_, other.memory_, min_size * block_size_);
        }
        size_ = min_size;
    }

    void move_content(
            uint8_t* target,
            uint8_t* source,
            bool overlap = false)
    {
        if (source != nullptr)
        {
            if (content_->first().is_constructed_type() || content_->second().is_constructed_type())
            {
                if (overlap && check_overlap(target, source))
                {
                    // Creating a place
                    uint32_t to_move = size_ - get_key_index(source);
                    for (uint32_t i = to_move; i > 0; --i)
                    {
                        content_->move_instance(target + (i - 1) * block_size_, source + (i - 1) * block_size_, true);
                    }
                }
                else
                {
                    // Moving full memory
                    for (uint32_t i = 0; i < size_; ++i)
                    {
                        content_->move_instance(target + i * block_size_, source + i * block_size_, true);
                    }
                }
            }
            else //optimization when the pair are both primitive
            {
                std::memmove(target, source, (size_ - get_key_index(source)) * block_size_);
            }
        }
    }

    bool check_overlap(
            const uint8_t* target,
            const uint8_t* source) const
    {
        uint8_t* end = get_element(size_);
        return source >= memory_ && target > source && end >= target; // target and source are inside the block of memory
    }

    uint8_t* create_place(
            const uint8_t* instance)
    {
        uint8_t* place = find_place(instance);
        xtypes_assert(
            size_ == 0 ||
            place == get_element(size_) ||     // Insert at the end
            hash(instance) != hash(place),
            "Key already exists.");
        if (place != get_element(size_))
        {
            move_content(place + block_size_, place, true);
        }
        return place;
    }

    uint8_t* find_place(
            const uint8_t* instance,
            bool exact = false) const
    {
        uint64_t instance_hash = 0;
        // Special case, it is empty
        if (size_ == 0)
        {
            if (exact)
            {
                return nullptr;
            }
            else
            {
                return memory_;
            }
        }

        // Binary search by hash
        uint64_t istart = 0;
        uint64_t iend = size_;
        uint64_t icurrent = (istart + iend) / 2;
        uint8_t* start = get_element(0);
        uint8_t* end = get_element(size_);
        uint8_t* current = get_element(icurrent);

        instance_hash = hash(instance);
        uint64_t current_hash = hash(current);

        while (icurrent != istart)
        {
            if (instance_hash == current_hash)
            {
                return current;
            }
            else if (instance_hash > current_hash)
            {
                if (current == start)
                {
                    // The spot is "end"
                    current = end;
                    continue;
                }
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
            current_hash = hash(current);
        }

        if (instance_hash == current_hash)
        {
            return current;
        }
        else if (instance_hash < current_hash)
        {
            return exact ? nullptr : current;
        }
        else
        {
            return exact ? nullptr : end;
        }
    }

    uint8_t* get_element(
            size_t index) const
    {
        return memory_ + index * block_size_;
    }

    size_t get_key_index(
            const uint8_t* instance) const
    {
        if (memory_ != nullptr)
        {
            uint8_t* place = find_place(instance, true);
            xtypes_assert(place != nullptr, "Key doesn't exists.");
            return (place - memory_) / block_size_;
        }
        return 0;
    }

    uint64_t hash(
            const uint8_t* instance) const
    {
        return content_->first().hash(instance);
    }

    void free_memory()
    {
        if (memory_ != nullptr)
        {
            if (content_->is_constructed_type())
            {
                for (int32_t i = capacity_ - 1; i >= 0; i--)
                {
                    content_->destroy_instance(memory_ + i * block_size_);
                }
            }

            delete[] memory_;
            memory_ = nullptr;
        }
    }

};

} //namespace xtypes
} //namespace eprosima

#endif //EPROSIMA_XTYPES_MAP_INSTANCE_HPP_
