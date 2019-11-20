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

#ifndef EPROSIMA_XTYPES_DYNAMIC_DATA_HPP_
#define EPROSIMA_XTYPES_DYNAMIC_DATA_HPP_

#include <xtypes/StructType.hpp>
#include <xtypes/CollectionType.hpp>
#include <xtypes/SequenceType.hpp>
#include <xtypes/PrimitiveType.hpp>

#include <cassert>
#include <iostream>

namespace eprosima {
namespace xtypes {

/// \brief Check if a C type can promote to a PrimitiveType or StringType.
template<typename T>
using PrimitiveOrString = typename std::enable_if<
    std::is_arithmetic<T>::value ||
    std::is_same<std::string, T>::value ||
    std::is_same<std::wstring, T>::value
    >::type;

/// \brief Check if a C type is a primitive type.
template<typename T>
using Primitive = typename std::enable_if<std::is_arithmetic<T>::value>::type;

/// \brief Class representing a only readable DynamicData reference.
/// Only readable methods are available.
class ReadableDynamicDataRef
{
public:
    virtual ~ReadableDynamicDataRef() = default;

    /// \brief Deep equality operator. All DynamicData tree will be evaluated for equality.
    bool operator == (
            const ReadableDynamicDataRef& other) const
    {
        return type_.compare_instance(instance_, other.instance_);
    }

    /// \brief Deep inequality operator. Inverse of == operator.
    bool operator != (
            const ReadableDynamicDataRef& other) const
    {
        return !(*this == other);
    }

    /// \brief Enable a transparent access to the internal value of the PrimitiveType or W/StringType.
    /// Avoid the use of .value<T>() at return.
    template<typename T>
    operator T() const
    {
        return value<T>();
    }

    /// \brief The representing type of this DynamicData.
    /// \returns a reference to the representing DynamicType
    const DynamicType& type() const { return type_; }

    /// \brief Returns the id of the managed instance.
    /// \returns A unique id of the managed instace.
    size_t instance_id() const { return size_t(instance_); }

    /// \brief String representing the DynamicData tree.
    /// \returns A string representing the DynamicData tree.
    inline std::string to_string() const; // Into DynamicDataImpl.hpp

    /// \brief Returns a value as primitive or string.
    /// \pre The DynamicData must represent a primitive or string value.
    /// \returns the value stored in the DynamicData.
    template<typename T, class = PrimitiveOrString<T>>
    T& value() const
    {
        assert((type_.kind() == TypeKind::STRING_TYPE && std::is_same<std::string, T>::value)
            || (type_.kind() == TypeKind::WSTRING_TYPE && std::is_same<std::wstring, T>::value)
            || (type_.kind() == primitive_type<T>().kind()));
        return *reinterpret_cast<T*>(instance_);
    }

    /// \brief Member access operator by name.
    /// \param[in] member_name Name of the member to access.
    /// \pre The DynamicData must represent an AggregationType.
    /// \pre The member_name must exists
    /// \returns A readable reference of the DynamicData accessed.
    ReadableDynamicDataRef operator [] (
            const std::string& member_name) const
    {
        assert(type_.is_aggregation_type());
        const AggregationType& aggregation = static_cast<const AggregationType&>(type_);
        assert(aggregation.has_member(member_name));

        const Member& member = aggregation.member(member_name);
        return ReadableDynamicDataRef(member.type(), instance_ + member.offset());
    }

    /// \brief index access operator by name.
    /// Depends of the underlying DynamicType, the index can be represent the member or element position.
    /// \param[in] index Index requested.
    /// \pre The DynamicData must represent an AggregationType or a CollectionType.
    /// \pre index < size()
    /// \returns A readable reference of the DynamicData accessed.
    ReadableDynamicDataRef operator [] (
            size_t index) const
    {
        assert((type_.is_aggregation_type() || type_.is_collection_type()) && index < size());
        if(type_.is_collection_type())
        {
            const CollectionType& collection = static_cast<const CollectionType&>(type_);
            return ReadableDynamicDataRef(collection.content_type(), collection.get_instance_at(instance_, index));
        }

        const AggregationType& aggregation = static_cast<const StructType&>(type_);
        const Member& member = aggregation.member(index);
        return ReadableDynamicDataRef(member.type(), instance_ + member.offset());
    }

    /// \brief Size of the DynamicData.
    /// Aggregation types will return the member count.
    /// Collection types will return the member count.
    /// Primtive types are considered as size 1.
    /// \returns Element size of the DynamicData.
    size_t size() const
    {
        assert(type_.is_collection_type() || type_.is_aggregation_type());
        if(type_.is_collection_type())
        {
            const CollectionType& collection = static_cast<const CollectionType&>(type_);
            return collection.get_instance_size(instance_);
        }
        if(type_.is_aggregation_type())
        {
            const AggregationType& aggregation = static_cast<const AggregationType&>(type_);
            return aggregation.members().size();
        }
        return 1;
    }

    /// \brief Shortcut for ((MutableCollectionType)type()).bounds()
    /// \pre The DynamicData must represent a CollectionType.
    /// \returns Bound (max size) of the type. If zero, means the collecition is unbound.
    /// If the DynamicData represents an Array, then bounds() == size()
    size_t bounds() const
    {
        assert(type_.is_collection_type());
        if (type_.is_collection_type())
        {
            if (type_.kind() == TypeKind::ARRAY_TYPE)
            {
                return size();
            }
            const MutableCollectionType& collection = static_cast<const MutableCollectionType&>(type_);
            return collection.bounds();
        }
        return 0;
    }

    /// \brief Returns a std::vector representing the underlying collection of types.
    /// \pre The collection must have primitive or string values.
    /// \returns a std::vector representing the internal collection.
    template<typename T, class = PrimitiveOrString<T>>
    std::vector<T> as_vector() const
    {
        const CollectionType& collection = static_cast<const CollectionType&>(type_);
        assert(type_.is_collection_type());
        assert((collection.content_type().kind() == TypeKind::STRING_TYPE && std::is_same<std::string, T>::value)
            || (collection.content_type().kind() == TypeKind::WSTRING_TYPE && std::is_same<std::wstring, T>::value)
            || (collection.content_type().kind() == primitive_type<T>().kind()));

        const T* location = reinterpret_cast<T*>(collection.get_instance_at(instance_, 0));
        return std::vector<T>(location, location + size());
    }

    /// \brief Class used by for_each() function to represent a readable DynamicData node in the tree.
    class ReadableNode
    {
    public:
        ReadableNode(const Instanceable::InstanceNode& instance_node) : internal_(instance_node) {}

        /// \brief Check the parent existance in the tree.
        /// \returns true if has parent.
        bool has_parent() const { return internal_.parent != nullptr; }

        /// \brief Get the parent
        /// \returns A ReadableNode representing the parent in the DynamicData tree.
        ReadableNode parent() const { assert(has_parent()); return ReadableNode(*internal_.parent); }

        /// \brief Get the associated data.
        /// \returns A readable reference of the data.
        ReadableDynamicDataRef data() const { return ReadableDynamicDataRef(internal_.type, internal_.instance); }

        /// \brief Get the representing type.
        /// \returns The DynamicType associated to the data.
        const DynamicType& type() const { return internal_.type; }

        /// \brief Current deep in the DynamicData tree (starts at deep 0).
        /// \returns The current deep.
        size_t deep() const { return internal_.deep; }

        /// \brief The index used to access to this ReadableNode
        /// \returns The index used.
        size_t from_index() const { return internal_.from_index; }

        /// \brief The Member used to access to this ReadableNode
        /// \returns The member or null if the accessor is not an aggregation type.
        const Member* from_member() const { return internal_.from_member; }
    private:
        const Instanceable::InstanceNode& internal_;
    };

    /// \brief Iterate the DynamicData in deep. Each node visited will call to the user visitor function.
    /// \param[in] visitor User visitor function.
    /// \returns true if no exceptions by the user were throw. Otherwise, the user boolean exception value.
    bool for_each(std::function<void(const ReadableNode& node)> visitor) const
    {
        Instanceable::InstanceNode root(type_, instance_);
        try
        {
            type_.for_each_instance(root, [&](const Instanceable::InstanceNode& instance_node)
            {
                visitor(ReadableNode(instance_node));
            });
            return true;
        }
        catch(bool value) { return value; }
    }

    /// \brief Class used for iterate ReadableDynamicDataRef
    class Iterator
    {
    public:
        Iterator(
                const Iterator& it)
            : type_(it.type_)
            , instance_(it.instance_)
            , index_(it.index_)
        {}

        Iterator& operator = (
                const Iterator& other)
        {
            instance_ = other.instance_;
            index_ = other.index_;
            return *this;
        }

        bool operator == (
                const Iterator& other) const
        {
            return other.instance_ == instance_ && other.index_ == index_;
        }

        bool operator != (
                const Iterator& other) const
        {
            return !(*this == other);
        }

        ReadableDynamicDataRef operator * () const
        {
            const CollectionType& collection = static_cast<const CollectionType&>(type_);
            return ReadableDynamicDataRef(collection.content_type(), collection.get_instance_at(instance_, index_));
        }

        Iterator& operator ++ ()
        {
            ++index_;
            return *this;
        }

        Iterator operator ++ (
                int)
        {
            Iterator prev = *this;
            ++index_;
            return prev;
        }

    protected:

        friend class ReadableDynamicDataRef;

        Iterator(
                const ReadableDynamicDataRef& ref,
                bool end)
            : type_(ref.type_)
            , instance_(ref.instance_)
            , index_(end ? ref.size() : 0)
        {
        }

        const DynamicType& type_;
        uint8_t* instance_;
        size_t index_;
    };

    /// \brief Returns the initial iterator of a collection dynamic data.
    /// \pre The DynamicData must represent a CollectionType.
    /// \returns The initial iterator.
    Iterator begin() const
    {
        assert(type_.is_collection_type());
        return Iterator(*this, false);
    }

    /// \brief Returns the final iterator of a collection dynamic data.
    /// \pre The DynamicData must represent a CollectionType.
    /// \returns The final iterator.
    Iterator end() const
    {
        assert(type_.is_collection_type());
        return Iterator(*this, true);
    }

    class MemberPair
    {
    public:
        MemberPair(
                const Member& member,
                uint8_t* data)
            : member_(member)
            , instance_(data)
        {}

        const Member& member() const
        {
            return member_;
        }

        ReadableDynamicDataRef data() const
        {
            return ReadableDynamicDataRef(member_.type(), instance_);
        }

        //! Shortcut to member().type().kind() or data().type().kind()
        TypeKind kind() const
        {
            return member_.type().kind();
        }

    protected:
        const Member& member_;
        uint8_t* instance_;
    };

    class MemberIterator : public Iterator
    {
    public:
        MemberIterator(
                const MemberIterator& it)
            : Iterator (it)
            , ref_(it.ref_)
        {}

        const MemberPair operator * () const
        {
            const AggregationType& aggregation = static_cast<const AggregationType&>(type_);
            return MemberPair(
                aggregation.member(index_),
                instance_ + aggregation.member(index_).offset());
        }

        MemberIterator& operator ++ ()
        {
            ++index_;
            return *this;
        }

        MemberIterator operator ++ (
                int)
        {
            MemberIterator prev = *this;
            ++index_;
            return prev;
        }

        MemberIterator begin() const
        {
            assert(type_.is_aggregation_type());
            return MemberIterator(ref_, false);
        }

        MemberIterator end() const
        {
            assert(type_.is_aggregation_type());
            return MemberIterator(ref_, true);
        }

    protected:
        friend class ReadableDynamicDataRef;
        friend class WritableDynamicDataRef;

        const ReadableDynamicDataRef& ref_;

        MemberIterator(
                const ReadableDynamicDataRef& ref,
                bool end)
            : Iterator(ref, end)
            , ref_(ref)
        {
        }
    };

    /// \brief Returns an iterable representation of an aggregation dynamic data.
    /// \pre The DynamicData must represent an AggregationType.
    /// \returns An iterable representation of an aggregation dynamic data.
    MemberIterator items() const
    {
        assert(type_.is_aggregation_type());
        return MemberIterator(*this, false);
    }

protected:
    ReadableDynamicDataRef(
            const DynamicType& type,
            uint8_t* source)
        : type_(type)
        , instance_(source)
    {}

    const DynamicType& type_;
    uint8_t* instance_;

    /// \brief protected access to other DynamicData instace.
    /// \param[in] other readable reference from who get the instance.
    /// \result The raw instance.
    uint8_t* p_instance(const ReadableDynamicDataRef& other) const { return other.instance_; }

};


/// \brief Class representing a writable DynamicData reference.
/// This class extends the ReadableDynamicDataRef with a several writable methods.
class WritableDynamicDataRef : public ReadableDynamicDataRef
{
public:
    using ReadableDynamicDataRef::operator [];

    /// \brief Assignment operator.
    WritableDynamicDataRef& operator = (
            const WritableDynamicDataRef& other)
    {
        type_.destroy_instance(instance_);
        type_.copy_instance(instance_, p_instance(other));
        return *this;
    }

    /// \brief A shortcut of WritableDynamicDataRef::value()
    /// \returns A reference to this DynamicData.
    template<typename T, class = PrimitiveOrString<T>>
    WritableDynamicDataRef& operator = (
            const T& other)
    {
        value(other);
        return *this;
    }

    /// \brief Specialization of WritableDynamicDataRef::operator =() for string
    WritableDynamicDataRef& operator = (
            const std::string& other)
    {
        value<std::string>(other);
        return *this;
    }

    /// \brief Specialization of WritableDynamicDataRef::operator =() for wstring
    WritableDynamicDataRef& operator = (
            const std::wstring& other)
    {
        value<std::wstring>(other);
        return *this;
    }

    /// \brief Request a readable reference from this DynamicData.
    /// \returns a ReadableDynamicDataRef identifying the writable DynamicData.
    ReadableDynamicDataRef cref() const { return ReadableDynamicDataRef(*this); }

    /// \brief See ReadableDynamicDataRef::value()
    template<typename T, class = PrimitiveOrString<T>>
    const T& value()
    {
        return ReadableDynamicDataRef::value<T>();
    }

    /// \brief See ReadableDynamicDataRef::operator[]()
    /// \returns A writable reference to the DynamicData accessed.
    WritableDynamicDataRef operator [] (
            const std::string& member_name)
    {
        return ReadableDynamicDataRef::operator[](member_name);
    }

    /// \brief See ReadableDynamicDataRef::operator[]()
    /// \returns A writable reference to the DynamicData accessed.
    WritableDynamicDataRef operator [] (
            size_t index) //
    {
        return ReadableDynamicDataRef::operator[](index);
    }

    /// \brief Set a primitive or string value into the DynamicData
    /// \input[in] t The primitive or string value.
    /// \pre The DynamicData must represent a PrimitiveType or W/StringType value.
    template<typename T, class = PrimitiveOrString<T>>
    void value(const T& t)
    {
        assert((type_.kind() == TypeKind::STRING_TYPE && std::is_same<std::string, T>::value)
            || (type_.kind() == TypeKind::WSTRING_TYPE && std::is_same<std::wstring, T>::value)
            || (type_.kind() == PrimitiveTypeKindTrait<T>::kind));

        type_.destroy_instance(instance_);
        type_.copy_instance(instance_, reinterpret_cast<const uint8_t*>(&t));
    }

    /// \brief Push a primitive or string value into the DynamicData that represents a SequenceType
    /// \input[in] t The primitive or string value.
    /// \pre The DynamicData must represent a SequenceType.
    /// \pre The sequence must have enough space to place this element or be unbounded.
    /// \returns The writable reference to this DynamicData
    template<typename T, class = PrimitiveOrString<T>>
    WritableDynamicDataRef& push(const T& t) // this = SequenceType
    {
        assert(type_.kind() == TypeKind::SEQUENCE_TYPE);
        const SequenceType& sequence = static_cast<const SequenceType&>(type_);
        assert((sequence.content_type().kind() == TypeKind::STRING_TYPE && std::is_same<std::string, T>::value)
            || (sequence.content_type().kind() == TypeKind::WSTRING_TYPE && std::is_same<std::wstring, T>::value)
            || (sequence.content_type().kind() == primitive_type<T>().kind()));

        uint8_t* element = sequence.push_instance(instance_, reinterpret_cast<const uint8_t*>(&t));
        assert(element != nullptr); (void) element;
        return *this;
    }

    /// \brief Push another DynamicData into the DynamicData that represents a SequenceType
    /// \input[in] data DynamicData to add into the sequence
    /// \pre The DynamicData must represent a SequenceType.
    /// \pre The sequence must have enough space to place this element or be unbounded.
    /// \returns The writable reference to this DynamicData
    WritableDynamicDataRef& push(const ReadableDynamicDataRef& data) // this = SequenceType
    {
        assert(type_.kind() == TypeKind::SEQUENCE_TYPE);
        const SequenceType& sequence = static_cast<const SequenceType&>(type_);

        uint8_t* element = sequence.push_instance(instance_, p_instance(data));
        assert(element != nullptr); (void) element;
        return *this;
    }

    /// \brief resize the Sequence representing by the DynamicData.
    /// If size is less or equals that the current size, nothing happens,
    /// otherwise a default-initialized values are insert to the sequence to increase its size.
    /// \param[int] size New sequence size
    /// \pre The DynamicData must represent a SequenceType.
    /// \returns The writable reference to this DynamicData
    WritableDynamicDataRef& resize(size_t size) // this = SequenceType
    {
        assert(type_.kind() == TypeKind::SEQUENCE_TYPE);
        const SequenceType& sequence = static_cast<const SequenceType&>(type_);

        sequence.resize_instance(instance_, size);
        return *this;
    }

    /// \brief (See ReadableDynamicData::for_each())
    bool for_each(std::function<void(const ReadableNode& node)> visitor) const
    {
        return ReadableDynamicDataRef::for_each(visitor);
    }

    /// \brief Class used by for_each() function to represent a writable DynamicData node in the tree.
    class WritableNode : public ReadableNode
    {
    public:
        WritableNode(const Instanceable::InstanceNode& instance_node) : ReadableNode(instance_node) {}

        /// \brief See ReadableNode::data()
        /// \returns A writable reference of the data.
        WritableDynamicDataRef data() { return ReadableNode::data(); }
    };

    /// \brief (See ReadableDynamicData::for_each())
    /// A writable specialization of ReadableDynamicDataRef::for_each() function.
    bool for_each(std::function<void(WritableNode& node)> visitor)
    {
        Instanceable::InstanceNode root(type_, instance_);
        try
        {
            type_.for_each_instance(root, [&](const Instanceable::InstanceNode& instance_node)
            {
                WritableNode node(instance_node);
                visitor(node);
            });
            return true;
        }
        catch(bool value) { return value; }
    }

    /// \brief Class used for iterate WritableDynamicDataRef
    class Iterator : public ReadableDynamicDataRef::Iterator
    {
    public:
        Iterator(
                const ReadableDynamicDataRef::Iterator& rit)
            : ReadableDynamicDataRef::Iterator (rit)
        {}

        WritableDynamicDataRef operator * ()
        {
            return ReadableDynamicDataRef::Iterator::operator*();
        }

    protected:

        friend class WritableDynamicDataRef;

        Iterator(
                WritableDynamicDataRef& ref,
                bool end)
            : ReadableDynamicDataRef::Iterator(ref, end)
        {
        }

    };

    /// \brief Returns the initial iterator of a collection dynamic data.
    /// \pre The DynamicData must represent a CollectionType.
    /// \returns The initial iterator.
    Iterator begin()
    {
        return static_cast<Iterator>(ReadableDynamicDataRef::begin());
    }

    /// \brief Returns the final iterator of a collection dynamic data.
    /// \pre The DynamicData must represent a CollectionType.
    /// \returns The final iterator.
    Iterator end()
    {
        return static_cast<Iterator>(ReadableDynamicDataRef::end());
    }

    class MemberPair : public ReadableDynamicDataRef::MemberPair
    {
    public:
        MemberPair(
                const Member& member,
                uint8_t* data)
            : ReadableDynamicDataRef::MemberPair(member, data)
        {}

        WritableDynamicDataRef data()
        {
            return WritableDynamicDataRef(member_.type(), instance_);
        }

        ReadableDynamicDataRef data() const
        {
            return ReadableDynamicDataRef::MemberPair::data();
        }

    };

    class MemberIterator : public Iterator
    {
    public:
        MemberIterator(
                const MemberIterator& it)
            : Iterator(it)
            , ref_(it.ref_)
        {}

        MemberPair operator * ()
        {
            const AggregationType& aggregation = static_cast<const AggregationType&>(type_);
            return MemberPair(
                aggregation.member(index_),
                instance_ + aggregation.member(index_).offset());
        }

        MemberIterator& operator ++ ()
        {
            ++index_;
            return *this;
        }

        MemberIterator operator ++ (
                int)
        {
            MemberIterator prev = *this;
            ++index_;
            return prev;
        }

        MemberIterator begin()
        {
            return ReadableDynamicDataRef::MemberIterator(ref_, false);
        }

        MemberIterator end()
        {
            return ReadableDynamicDataRef::MemberIterator(ref_, true);
        }

    protected:
        friend class WritableDynamicDataRef;

        WritableDynamicDataRef& ref_;

        MemberIterator(
                WritableDynamicDataRef& ref,
                bool end)
            : Iterator(ref, end)
            , ref_(ref)
        {
        }

        MemberIterator(
                const ReadableDynamicDataRef::MemberIterator& mit)
            : Iterator (mit)
            , ref_(static_cast<WritableDynamicDataRef&>(const_cast<ReadableDynamicDataRef&>(mit.ref_)))
        {}
    };

    /// \brief Returns an iterable representation of an aggregation dynamic data.
    /// \pre The DynamicData must represent an AggregationType.
    /// \returns An iterable representation of an aggregation dynamic data.
    MemberIterator items()
    {
        return MemberIterator(*this, false);
    }

    /// \brief Returns a read-only iterable representation of an aggregation dynamic data.
    /// \pre The DynamicData must represent an AggregationType.
    /// \returns An iterable representation of an aggregation dynamic data.
    ReadableDynamicDataRef::MemberIterator citems()
    {
        assert(type_.is_aggregation_type());
        return ReadableDynamicDataRef::MemberIterator(*this, false);
    }

protected:
    WritableDynamicDataRef(
            const DynamicType& type,
            uint8_t* source)
        : ReadableDynamicDataRef(type, source)
    {}

    /// \brief Internal cast from readable to writable
    WritableDynamicDataRef(
            const ReadableDynamicDataRef& other)
        : ReadableDynamicDataRef(other)
    {}

    WritableDynamicDataRef(
            ReadableDynamicDataRef&& other)
        : ReadableDynamicDataRef(std::move(other))
    {}

};

/// \brief Class that represents a DynamicType instantation in memory.
class DynamicData : public WritableDynamicDataRef
{
public:
    /// \brief Construct a DynamicData from a DynamicType specification.
    /// The required memory for holding the instance is reserved at this point.
    /// \param[in] type DynamicType from which the DynamicData is created.
    DynamicData(
            const DynamicType& type)
        : WritableDynamicDataRef(type, new uint8_t[type.memory_size()])
    {
        type_.construct_instance(instance_);
    }

    /// \brief Copy constructor from a ReadableDynamicDataRef
    DynamicData(
            const ReadableDynamicDataRef& other)
        : WritableDynamicDataRef(other)
    {
    }

    /// \brief Move constructor from a WritableDynamicDataRef
    DynamicData(
            WritableDynamicDataRef&& other)
        : WritableDynamicDataRef(std::move(other))
    {
    }

    /// \brief Construct a DynamicData from another DynamicData with a compatible type.
    /// (see DynamicType::is_compatible())
    /// \param[in] other A compatible DynamicData from which de data will be copies.
    /// \param[in] type DynamicType from which the DynamicData is created.
    DynamicData(
            const ReadableDynamicDataRef& other,
            const DynamicType& type)
        : WritableDynamicDataRef(type, new uint8_t[type.memory_size()])
    {
        assert(type_.is_compatible(other.type()) != TypeConsistency::NONE);
        type_.copy_instance_from_type(instance_, p_instance(other), other.type());
    }

    /// \brief Copy constructor
    DynamicData(const DynamicData& other)
        : WritableDynamicDataRef(other.type_, new uint8_t[other.type_.memory_size()])
    {
        assert(type_.is_compatible(other.type()) == TypeConsistency::EQUALS);
        type_.copy_instance(instance_, p_instance(other));
    }

    /// \brief Move constructor
    DynamicData(DynamicData&& other)
        : WritableDynamicDataRef(other.type_, new uint8_t[other.type_.memory_size()])
    {
        assert(type_.is_compatible(other.type()) == TypeConsistency::EQUALS);
        type_.move_instance(instance_, p_instance(other));
    }

    /// \brief Assignment operator
    DynamicData& operator = (
            const DynamicData& other)
    {
        assert(type_.is_compatible(other.type()) == TypeConsistency::EQUALS);
        type_.destroy_instance(instance_);
        type_.copy_instance(instance_, p_instance(other));
        return *this;
    }

    /// \brief See WritableDynamicDataRef::operator =()
    template<typename T, class = PrimitiveOrString<T>>
    WritableDynamicDataRef& operator = (
            const T& other)
    {
        return WritableDynamicDataRef::operator=(other);
    }

    /// \brief See WritableDynamicDataRef::operator =()
    WritableDynamicDataRef& operator = (
            const std::string& other)
    {
        return WritableDynamicDataRef::operator=(other);
    }

    /// \brief See WritableDynamicDataRef::operator =()
    WritableDynamicDataRef& operator = (
            const std::wstring& other)
    {
        return WritableDynamicDataRef::operator=(other);
    }

    virtual ~DynamicData() override
    {
        type_.destroy_instance(instance_);
        delete[] instance_;
    }

    /// \brief Request a writable reference from this DynamicData.
    /// \returns a WritableDynamicDataRef identifying th DynamicData.
    WritableDynamicDataRef ref() const { return WritableDynamicDataRef(*this); }

    template<typename T, class = Primitive<T>>
    inline T cast() const;

    template<typename T = std::string>
    inline T cast() const;

};

} //namespace xtypes
} //namespace eprosima

#include <xtypes/DynamicDataImpl.hpp>

#endif //EPROSIMA_XTYPES_DYNAMIC_DATA_HPP_
