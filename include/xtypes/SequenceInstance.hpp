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

#ifndef EPROSIMA_XTYPES_SEQUENCE_INSTANCE_HPP_
#define EPROSIMA_XTYPES_SEQUENCE_INSTANCE_HPP_

#include <xtypes/DynamicType.hpp>

#include <cstdint>
#include <cstring>

namespace eprosima {
namespace xtypes {

/// \brief Implementation of a dynamic sequence of DynamicTypes.
/// This class is used internal by SequenceType to implement its behaviour.
class SequenceInstance
{
public:

    /// \brief Construct a SequenceInstance
    /// \param[in] content Content of the sequence
    /// \param[in] capacity Reserved memory for the sequence.
    SequenceInstance(
            const DynamicType& content,
            uint32_t capacity = 0)
        : block_size_(content.memory_size())
        , capacity_(capacity)
        , size_(0)
    {
        try
        {
            // is already associated to the DynamicData object
            content_ = content.shared_from_this();
        }
        catch(const std::bad_weak_ptr&)
        {
            // make a copy (if type changes may blow the DynamicData object)
            content_ = content.clone();
        }

        init_memory(memory_, capacity_);
    }

    SequenceInstance(
            const SequenceInstance& other)
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

    /// \brief Copy constructor from a SequenceInstance with other but compatible content.
    /// \param[in] other Sequence from copy the values.
    /// \param[in] content Content of the sequence
    /// \param[in] bounds Max copied elements.
    /// \pre content and other.content compatibles (see dds::core::xtypes::DynamicType::is_compatible())
    SequenceInstance(
            const SequenceInstance& other,
            const DynamicType& content,
            uint32_t bounds)
        : block_size_(content.memory_size())
        , capacity_(bounds == 0 ? other.capacity_ : std::min(other.capacity_, bounds))
        , size_(bounds == 0 ? other.size_ : std::min(other.size_, bounds))
    {
        try
        {
            // is already associated to the DynamicData object
            content_ = content.shared_from_this();
        }
        catch(const std::bad_weak_ptr&)
        {
            // make a copy (if type changes may blow the DynamicData object)
            content_ = content.clone();
        }

        init_memory(memory_, capacity_);

        if (memory_ != nullptr)
        {
            copy_content(other, size_);
        }
    }

    SequenceInstance(
            SequenceInstance&& other)
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
            const SequenceInstance& other) const
    {
        if (other.size() != size_)
        {
            return false;
        }

        if (content_->is_constructed_type())
        {
            bool comp = true;
            for (uint32_t i = 0; i < size_; i++)
            {
                comp &= content_->compare_instance(memory_ + i * block_size_, other.memory_ + i * block_size_);
            }
            return comp;
        }
        else //optimization when the type is primitive
        {
            return std::memcmp(memory_, other.memory_, size_ * block_size_) == 0;
        }
    }

    virtual ~SequenceInstance()
    {
        free_memory();
    }

    /// \brief Push a instance into the sequence
    /// A reallocation can be done in order to allocate this new value.
    /// \param[in] instance Instance to push into the sequence.
    /// \returns Returns the location of the new instance added.
    uint8_t* push(
            const uint8_t* instance,
            uint32_t bounds)
    {
        if (memory_ == nullptr || size_ == capacity_)
        {
            realloc((capacity_ > 0) ? capacity_ * 2 : 1, bounds);
        }

        uint8_t* place = memory_ + size_ * block_size_;
        content_->copy_instance(place, instance);

        size_++;

        return place;
    }

    /// \brief Resize the sequence. All new elements will be default-initialized.
    /// If the new size is equal or less than the current size, nothing happens.
    /// \param[in] new_size New sequence size.
    void resize(
            size_t new_size,
            uint32_t bounds)
    {
        if (size_ >= new_size)
        {
            return;
        }

        realloc(new_size, bounds);

        for (size_t i = size_; i < new_size; i++)
        {
            uint8_t* place = memory_ + i * block_size_;
            content_->construct_instance(place);
        }

        size_ = new_size;
    }

    /// \brief Index access operator.
    /// \param[in] index Requested index
    /// \returns The instance placed in index location.
    uint8_t* operator [] (
            uint32_t index) const
    {
        xtypes_assert(index < size_,
                "operator [" << index << "] is out of bounds.");
        return memory_ + index * block_size_;
    }

    /// \brief Size of the sequence.
    /// \returns Size of the sequence.
    uint32_t size() const
    {
        return size_;
    }

private:

    friend class SequenceType;

    std::shared_ptr<const DynamicType> content_;
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

        move_content(new_memory, memory_);

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
            const SequenceInstance& other,
            uint32_t bounds)
    {
        size_t other_block_size = other.content_->memory_size();

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

        if (memory_ == nullptr || // Unbounded sequences could reach this point without reserving memory yet.
                min_size > capacity_)
        {
            realloc(min_size, bounds);
        }

        if (content_->is_constructed_type() || block_size_ != other_block_size)
        {
            for (uint32_t i = 0; i < min_size; i++)
            {
                content_->copy_instance_from_type(
                    memory_ + i * block_size_,
                    other.memory_ + i * other_block_size,
                    *other.content_);
            }
        }
        else //optimization when the type is primitive with same block_size
        {
            std::memcpy(memory_, other.memory_, min_size * block_size_);
        }
        size_ = min_size;
    }

    void move_content(
            uint8_t* target,
            uint8_t* source)
    {
        if (source != nullptr)
        {
            if (content_->is_constructed_type())
            {
                for (uint32_t i = 0; i < size_; i++)
                {
                    content_->move_instance(target + i * block_size_, source + i * block_size_, true);
                }
            }
            else //optimization when the type is primitive
            {
                std::memmove(target, source, size_ * block_size_);
            }
        }
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

#endif //EPROSIMA_XTYPES_SEQUENCE_INSTANCE_HPP_
