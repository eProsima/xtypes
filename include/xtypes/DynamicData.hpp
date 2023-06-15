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
#include <xtypes/UnionType.hpp>
#include <xtypes/CollectionType.hpp>
#include <xtypes/SequenceType.hpp>
#include <xtypes/PrimitiveType.hpp>
#include <xtypes/EnumerationType.hpp>
#include <xtypes/AliasType.hpp>
#include <xtypes/MapType.hpp>

#include <ostream>

namespace eprosima {
namespace xtypes {

template < template <typename ...> class base, typename derived>
struct is_base_of_template_impl
{
    template<typename ... Ts>
    static constexpr std::true_type  test(
            const base<Ts ...>*);
    static constexpr std::false_type test(
            ...);
    using type = decltype(test(std::declval<derived*>()));
};

/// \brief Check if a class inherits from a templated class.
template < template <typename ...> class base, typename derived>
using is_base_of_template = typename is_base_of_template_impl<base, derived>::type;

/// \brief Check if a C type can promote to a PrimitiveType or StringType.
template<typename T>
using PrimitiveOrString = typename std::enable_if<
    std::is_arithmetic<T>::value ||
    std::is_same<std::string, T>::value ||
    std::is_same<std::wstring, T>::value ||
    std::is_same<std::u16string, T>::value ||
    is_base_of_template<EnumeratedType, T>::value
    >::type;

/// \brief Check if a C type is a primitive type.
template<typename T>
using Primitive = typename std::enable_if<std::is_arithmetic<T>::value>::type;

/// \brief Class representing a only readable DynamicData reference.
/// Only readable methods are available.
class ReadableDynamicDataRef
{
public:

    ReadableDynamicDataRef(const ReadableDynamicDataRef& other) = default;

    ReadableDynamicDataRef(ReadableDynamicDataRef&& other)
        : type_(std::move(other.type_))
        , instance_(other.instance_)
        , initialize_(other.initialize_)
    {
        other.initialize_ = false;
    }

    virtual ~ReadableDynamicDataRef() = default;

    /// \brief Deep equality operator. All DynamicData tree will be evaluated for equality.
    bool operator == (
            const ReadableDynamicDataRef& other) const
    {
        if (type_->kind() == TypeKind::ARRAY_TYPE)
        {
            // If the data is Array, a fast way to discard equality is that the content or size of the array
            // is different. We can check both without casting the type by comparing the type (array) name.
            return type_->name() == other.type().name() && type_->compare_instance(instance_, other.instance_);
        }
        return type_->compare_instance(instance_, other.instance_);
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
    const DynamicType& type() const
    {
        return *type_;
    }

    /// \brief Returns the id of the managed instance.
    /// \returns A unique id of the managed instace.
    size_t instance_id() const
    {
        return size_t(instance_);
    }

    /// \brief String representing the DynamicData tree.
    /// \returns A string representing the DynamicData tree.
    inline std::string to_string() const; // Into DynamicDataImpl.hpp

    /// \brief Returns a value as primitive or string.
    /// \pre The DynamicData must represent a primitive, enumerated or string value.
    /// \returns the value stored in the DynamicData.
    template<typename T, class = PrimitiveOrString<T> >
    const T& value() const
    {
        xtypes_assert((type_->kind() == TypeKind::STRING_TYPE && std::is_same<std::string, T>::value)
                || (type_->kind() == TypeKind::WSTRING_TYPE && std::is_same<std::wstring, T>::value)
                || (type_->kind() == TypeKind::STRING16_TYPE && std::is_same<std::u16string, T>::value)
                || (type_->kind() == primitive_type<T>().kind())
                || (type_->is_enumerated_type()),
                "Expected type '" << type_->name()
                                  << "' but '" << PrimitiveTypeKindTrait<T>::name << "' received while getting value.");

        if (type_->is_enumerated_type())
        {
            xtypes_assert(type_->memory_size() == sizeof(T),
                    "Incompatible types: '" << type_->name() << "' and '"
                                            << PrimitiveTypeKindTrait<T>::name << "'.");
        }

        return *reinterpret_cast<T*>(instance_);
    }

    /// \brief Member access operator by name.
    /// \param[in] member_name Name of the member to access.
    /// \pre The DynamicData must represent an AggregationType.
    /// \pre The member_name must exists
    /// \returns A readable reference of the DynamicData accessed.
    ReadableDynamicDataRef operator [] (
            const char* member_name) const
    {
        return operator_at_impl(member_name);
    }

    /// \brief Member access operator by name.
    /// \param[in] member_name Name of the member to access.
    /// \pre The DynamicData must represent an AggregationType.
    /// \pre The member_name must exists
    /// \returns A readable reference of the DynamicData accessed.
    ReadableDynamicDataRef operator [] (
            const std::string& member_name) const
    {
        return operator_at_impl(member_name);
    }

    /// \brief index access operator by name.
    /// Depends of the underlying DynamicType, the index can be represent the member or element position.
    /// \param[in] index Index requested.
    /// \pre The DynamicData must represent an AggregationType (except an UnionType) or a CollectionType.
    /// \pre index < size()
    /// \returns A readable reference of the DynamicData accessed.
    template<typename T, typename = typename std::enable_if<std::is_integral<T>::value>::type>
    ReadableDynamicDataRef operator [] (
            T index) const
    {
        size_t s_index(static_cast<size_t>(index));

        xtypes_assert(type_->is_aggregation_type() || type_->is_collection_type() || type_->kind() == TypeKind::PAIR_TYPE,
                "operator [size_t] isn't available for type '" << type_->name() << "'.");
        xtypes_assert(s_index < size(),
                "operator [" << s_index << "] is out of bounds.");
        if (type_->is_collection_type())
        {
            const CollectionType& collection = static_cast<const CollectionType&>(*type_);
            // The following assert exists because it may be confusing by the user, it will return the pair instead of
            // the value associated to the "key" representation of the index.
            xtypes_assert(type_->kind() != TypeKind::MAP_TYPE, "Cannot access a MapType by index");
            return ReadableDynamicDataRef(collection.content_type(), collection.get_instance_at(instance_, s_index));
        }

        xtypes_assert(type_->kind() != TypeKind::UNION_TYPE, "Members of UnionType cannot be accessed by index.");

        if (type_->kind() == TypeKind::PAIR_TYPE)
        {
            xtypes_assert(s_index < 2, "operator[" << s_index << "] is out of bounds.");
            const PairType& pair = static_cast<const PairType&>(*type_);
            if (s_index == 0)
            {
                return ReadableDynamicDataRef(pair.first(), instance_);
            }
            else
            {
                return ReadableDynamicDataRef(pair.second(), instance_ + pair.first().memory_size());
            }
        }

        const AggregationType& aggregation = static_cast<const StructType&>(*type_);
        const Member& member = aggregation.member(s_index);
        return ReadableDynamicDataRef(member.type(), instance_ + member.offset());
    }

    /// \brief access to Union discriminator.
    /// \pre The DynamicData must represent an UnionType.
    /// \return A readable reference of the discriminator.
    ReadableDynamicDataRef d() const
    {
        xtypes_assert(type_->kind() == TypeKind::UNION_TYPE, "discriminator is only available for UnionType.");
        const UnionType& aggregation = static_cast<const UnionType&>(*type_);
        const Member& member = aggregation.member(0);
        return ReadableDynamicDataRef(member.type(), instance_);
    }

    /// \brief returns the current selected member of an UnionType.
    /// \pre The DynamicData must represent an UnionType.
    /// \return The current selected member.
    const Member& current_case() const
    {
        xtypes_assert(type_->kind() == TypeKind::UNION_TYPE, "current_case is only available for UnionType.");
        const UnionType& aggregation = static_cast<const UnionType&>(*type_);
        return aggregation.get_current_selection(instance_);
    }

    /// \brief key access method by DynamicData.
    /// \param[in] data DynamicData representing a MapType key.
    /// \pre The DynamicData must represent a MapType.
    /// |pre The key must exists.
    /// \returns A readable reference of the DynamicData accessed.
    ReadableDynamicDataRef at (
            ReadableDynamicDataRef data) const
    {
        xtypes_assert(
            type_->kind() == TypeKind::MAP_TYPE,
            "'at()' method is only available for MapType.");

        const MapType& map = static_cast<const MapType&>(*type_);
        const PairType& pair = static_cast<const PairType&>(map.content_type());
        uint8_t* instance = map.get_instance_at(instance_, data.instance_);
        xtypes_assert(instance != nullptr, "MapType '" << type_->name() << "' doesn't contains the requested key.");
        return ReadableDynamicDataRef(pair.second(), instance + pair.first().memory_size());
    }

    /// \brief Size of the DynamicData.
    /// Aggregation types will return the member count.
    /// Collection types will return the member count.
    /// Primtive types are considered as size 1.
    /// \returns Element size of the DynamicData.
    size_t size() const
    {
        xtypes_assert(type_->is_collection_type() || type_->is_aggregation_type() || type_->kind() == TypeKind::PAIR_TYPE,
                "size() isn't available for type '" << type_->name() << "'.");
        if (type_->is_collection_type())
        {
            const CollectionType& collection = static_cast<const CollectionType&>(*type_);
            return collection.get_instance_size(instance_);
        }
        if (type_->is_aggregation_type())
        {
            const AggregationType& aggregation = static_cast<const AggregationType&>(*type_);
            return aggregation.members().size();
        }
        if (type_->kind() == TypeKind::PAIR_TYPE)
        {
            return 2;
        }
        return 1;
    }

    /// \brief Shortcut for ((MutableCollectionType)type()).bounds()
    /// \pre The DynamicData must represent a CollectionType.
    /// \returns Bound (max size) of the type. If zero, means the collection is unbound.
    /// If the DynamicData represents an Array, then bounds() == size()
    size_t bounds() const
    {
        xtypes_assert(type_->is_collection_type(),
                "bounds() isn't available for type '" << type_->name() << "'.");
        if (type_->is_collection_type())
        {
            if (type_->kind() == TypeKind::ARRAY_TYPE)
            {
                return size();
            }
            const MutableCollectionType& collection = static_cast<const MutableCollectionType&>(*type_);
            return collection.bounds();
        }
        return 0;
    }

    uint64_t hash() const
    {
        return type_->hash(instance_);
    }

    /// \brief Returns a std::vector representing the underlying collection of types.
    /// \pre The collection must have primitive or string values.
    /// \returns a std::vector representing the internal collection.
    template<typename T, class = PrimitiveOrString<T> >
    std::vector<T> as_vector() const
    {
        const CollectionType& collection = static_cast<const CollectionType&>(*type_);
        xtypes_assert(type_->is_collection_type(),
                "as_vector() isn't available for type '" << type_->name() << "'.");
        xtypes_assert((collection.content_type().kind() == TypeKind::STRING_TYPE && std::is_same<std::string, T>::value)
                || (collection.content_type().kind() == TypeKind::WSTRING_TYPE && std::is_same<std::wstring, T>::value)
                || (collection.content_type().kind() == TypeKind::STRING16_TYPE &&
                std::is_same<std::u16string, T>::value)
                || (collection.content_type().kind() == primitive_type<T>().kind()),
                "as_vector<" << PrimitiveTypeKindTrait<T>::name << ">() isn't available for type '"
                             << type_->name() << "'.");

        const T* location = reinterpret_cast<T*>(collection.get_instance_at(instance_, 0));
        return std::vector<T>(location, location + size());
    }

    template<typename T>
    inline T cast() const
    {
        return _cast<T>();
    }

    /// \brief Class used by for_each() function to represent a readable DynamicData node in the tree.
    class ReadableNode
    {
    public:

        ReadableNode(
                const Instanceable::InstanceNode& instance_node)
            : internal_(instance_node)
        {
        }

        /// \brief Check the parent existance in the tree.
        /// \returns true if has parent.
        bool has_parent() const
        {
            return internal_.parent != nullptr;
        }

        /// \brief Get the parent
        /// \returns A ReadableNode representing the parent in the DynamicData tree.
        ReadableNode parent() const
        {
            xtypes_assert(has_parent(),
                    "Called 'parent()' from a ReadableNode without parent. Call 'has_parent()' to ensure that the "
                    << "Node has parent.");
            return ReadableNode(*internal_.parent);
        }

        /// \brief Get the associated data.
        /// \returns A readable reference of the data.
        ReadableDynamicDataRef data() const
        {
            return ReadableDynamicDataRef(internal_.type, internal_.instance);
        }

        /// \brief Get the representing type.
        /// \returns The DynamicType associated to the data.
        const DynamicType& type() const
        {
            return internal_.type;
        }

        /// \brief Current deep in the DynamicData tree (starts at deep 0).
        /// \returns The current deep.
        size_t deep() const
        {
            return internal_.deep;
        }

        /// \brief The index used to access to this ReadableNode
        /// \returns The index used.
        size_t from_index() const
        {
            return internal_.from_index;
        }

        /// \brief The Member used to access to this ReadableNode
        /// \returns The member or null if the accessor is not an aggregation type.
        const Member* from_member() const
        {
            return internal_.from_member;
        }

    private:

        const Instanceable::InstanceNode& internal_;
    };

    /// \brief Iterate the DynamicData in deep. Each node visited will call to the user visitor function.
    /// \param[in] visitor User visitor function.
    /// \returns true if no exceptions by the user were throw. Otherwise, the user boolean exception value.
    bool for_each(
            std::function<void(const ReadableNode& node)> visitor) const
    {
        Instanceable::InstanceNode root(*type_, instance_);
        try
        {
            type_->for_each_instance(root, [&](const Instanceable::InstanceNode& instance_node)
                    {
                        visitor(ReadableNode(instance_node));
                    });
            return true;
        }
        catch (bool value)
        {
            return value;
        }
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
        {
        }

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
            const CollectionType& collection = static_cast<const CollectionType&>(*type_);
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

        std::shared_ptr<const DynamicType> type_;
        uint8_t* instance_;
        size_t index_;
    };

    /// \brief Returns the initial iterator of a collection dynamic data.
    /// \pre The DynamicData must represent a CollectionType.
    /// \returns The initial iterator.
    Iterator begin() const
    {
        xtypes_assert(type_->is_collection_type(),
                "begin() isn't available for type '" << type_->name() << "'.");
        return Iterator(*this, false);
    }

    /// \brief Returns the final iterator of a collection dynamic data.
    /// \pre The DynamicData must represent a CollectionType.
    /// \returns The final iterator.
    Iterator end() const
    {
        xtypes_assert(type_->is_collection_type(),
                "end() isn't available for type '" << type_->name() << "'.");
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
        {
        }

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
        {
        }

        const MemberPair operator * () const
        {
            const AggregationType& aggregation = static_cast<const AggregationType&>(*type_);
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
            xtypes_assert(type_->is_aggregation_type(),
                    "begin() isn't available for type '" << type_->name() << "'.");
            return MemberIterator(ref_, false);
        }

        MemberIterator end() const
        {
            xtypes_assert(type_->is_aggregation_type(),
                    "end() isn't available for type '" << type_->name() << "'.");
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
        xtypes_assert(type_->is_aggregation_type(),
                "items() isn't available for type '" << type_->name() << "'.");
        return MemberIterator(*this, false);
    }

protected:

    ReadableDynamicDataRef(
            const DynamicType& type,
            uint8_t* source)
        : instance_(source)
        , initialize_(true)
    {
        const DynamicType* local = &type;

        if( type.kind() == TypeKind::ALIAS_TYPE )
        {
            local = &static_cast<const AliasType&>(type).rget();
        }

        try
        {
            // is already associated to the DynamicData object
            type_ = local->shared_from_this();
        }
        catch(const std::bad_weak_ptr&)
        {
            // make a copy (if type changes may blow the DynamicData object)
            type_ = local->clone();
        }
    }

    std::shared_ptr<const DynamicType> type_;
    uint8_t* instance_;
    bool initialize_ = false;

    /// \brief protected access to other DynamicData instace.
    /// \param[in] other readable reference from who get the instance.
    /// \result The raw instance.
    uint8_t* p_instance(
            const ReadableDynamicDataRef& other) const
    {
        return other.instance_;
    }

    ReadableDynamicDataRef operator_at_impl (
            const std::string& member_name,
            bool read_only = true) const
    {
        xtypes_assert(type_->is_aggregation_type(),
                "operator [const std::string&] isn't available for type '" << type_->name() << "'.");
        const AggregationType& aggregation = static_cast<const AggregationType&>(*type_);
        xtypes_assert(type_->kind() != TypeKind::PAIR_TYPE, "PairType doesn't have operator [const std::string&]");
        xtypes_assert(aggregation.has_member(member_name),
                "Type '" << type_->name() << "' doesn't have a member named '" << member_name << "'.");

        if (type_->kind() == TypeKind::UNION_TYPE)
        {
            xtypes_assert(
                member_name != UNION_DISCRIMINATOR,
                "Access to Union discriminator must be done through 'd()' method.");

            UnionType& union_type = const_cast<UnionType&>(static_cast<const UnionType&>(aggregation));
            if (read_only)
            {
                xtypes_assert(member_name == union_type.get_current_selection(instance_).name(),
                        "Cannot retrieve a non-selected case member.");
            }
            union_type.select_case(instance_, member_name);
        }

        const Member& member = aggregation.member(member_name);
        return ReadableDynamicDataRef(member.type(), instance_ + member.offset());
    }

    template<typename T, class = Primitive<T> >
    inline T _cast() const
    {
        xtypes_assert(type_->is_primitive_type() || type_->is_enumerated_type(),
                "Expected a primitive type but '" << PrimitiveTypeKindTrait<T>::name <<
                "' received while casting data.");
        switch (type_->kind())
        {
            case TypeKind::BOOLEAN_TYPE:
            {
                bool temp = *this;
                return static_cast<T>(temp);
            }
            case TypeKind::INT_8_TYPE:
            {
                int8_t temp = *this;
                return static_cast<T>(temp);
            }
            case TypeKind::UINT_8_TYPE:
            {
                uint8_t temp = *this;
                return static_cast<T>(temp);
            }
            case TypeKind::INT_16_TYPE:
            {
                int16_t temp = *this;
                return static_cast<T>(temp);
            }
            case TypeKind::UINT_16_TYPE:
            {
                uint16_t temp = *this;
                return static_cast<T>(temp);
            }
            case TypeKind::INT_32_TYPE:
            {
                int32_t temp = *this;
                return static_cast<T>(temp);
            }
            case TypeKind::UINT_32_TYPE:
            {
                uint32_t temp = *this;
                return static_cast<T>(temp);
            }
            case TypeKind::INT_64_TYPE:
            {
                int64_t temp = *this;
                return static_cast<T>(temp);
            }
            case TypeKind::UINT_64_TYPE:
            {
                uint64_t temp = *this;
                return static_cast<T>(temp);
            }
            case TypeKind::FLOAT_32_TYPE:
            {
                float temp = *this;
                return static_cast<T>(temp);
            }
            case TypeKind::FLOAT_64_TYPE:
            {
                double temp = *this;
                return static_cast<T>(temp);
            }
            case TypeKind::FLOAT_128_TYPE:
            {
                long double temp = *this;
                return static_cast<T>(temp);
            }
            case TypeKind::CHAR_8_TYPE:
            {
                char temp = *this;
                return static_cast<T>(temp);
            }
            case TypeKind::CHAR_16_TYPE:
            {
                char16_t temp = *this;
                return static_cast<T>(temp);
            }
            case TypeKind::WIDE_CHAR_TYPE:
            {
                wchar_t temp = *this;
                return static_cast<T>(temp);
            }
            case TypeKind::ENUMERATION_TYPE:
            {
                // For checking the associated_type, check for its memory_size
                if (type_->memory_size() == sizeof(uint8_t))
                {
                    uint8_t temp = *this;
                    return static_cast<T>(temp);
                }
                else if (type_->memory_size() == sizeof(uint16_t))
                {
                    uint16_t temp = *this;
                    return static_cast<T>(temp);
                }
                else if (type_->memory_size() == sizeof(uint32_t))
                {
                    uint32_t temp = *this;
                    return static_cast<T>(temp);
                }
            }
            default:
                return T();
        }
    }

};


/// \brief Class representing a writable DynamicData reference.
/// This class extends the ReadableDynamicDataRef with a several writable methods.
class WritableDynamicDataRef : public ReadableDynamicDataRef
{
public:

    WritableDynamicDataRef(const WritableDynamicDataRef&) = default;
    WritableDynamicDataRef(WritableDynamicDataRef&&) = default;

    using ReadableDynamicDataRef::operator [];
    using ReadableDynamicDataRef::value;
    using ReadableDynamicDataRef::d;

    /// \brief Assignment operator.
    WritableDynamicDataRef& operator = (
            const WritableDynamicDataRef& other)
    {
        type_->destroy_instance(instance_);
        type_->copy_instance(instance_, p_instance(other));
        return *this;
    }

    /// \brief A shortcut of WritableDynamicDataRef::value()
    /// \returns A reference to this DynamicData.
    template<typename T, class = PrimitiveOrString<T> >
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

    /// \brief Specialization of WritableDynamicDataRef::operator =() for u16string
    WritableDynamicDataRef& operator = (
            const std::u16string& other)
    {
        value<std::u16string>(other);
        return *this;
    }

    /// \brief Request a readable reference from this DynamicData.
    /// \returns a ReadableDynamicDataRef identifying the writable DynamicData.
    ReadableDynamicDataRef cref() const
    {
        return ReadableDynamicDataRef(*this);
    }

    /// \brief See ReadableDynamicDataRef::value()
    template<typename T, class = PrimitiveOrString<T> >
    const T& value()
    {
        return ReadableDynamicDataRef::value<T>();
    }

    /// \brief Similar to operator [], but allow checking read access.
    ReadableDynamicDataRef get_member (
            const std::string& member_name) const
    {
        return operator_at_impl(member_name);
    }

    /// \brief See ReadableDynamicDataRef::operator[]()
    /// \returns A writable reference to the DynamicData accessed.
    WritableDynamicDataRef operator [] (
            const char* member_name)
    {
        return operator_at_impl(member_name, false);
    }

    /// \brief See ReadableDynamicDataRef::operator[]()
    /// \returns A writable reference to the DynamicData accessed.
    WritableDynamicDataRef operator [] (
            const std::string& member_name)
    {
        return operator_at_impl(member_name, false);
    }

    /// \brief See ReadableDynamicDataRef::operator[]()
    /// \returns A writable reference to the DynamicData accessed.
    template<typename T, typename = typename std::enable_if<std::is_integral<T>::value>::type>
    WritableDynamicDataRef operator [] (
            T index)
    {
        return ReadableDynamicDataRef::operator [](index);
    }

    /// \brief Modifies the discriminator of an Union.
    /// Doesn't allow to change the current active member. That must be done through the 'operator[](std::string)'.
    /// \pre The DynamicData must represent an UnionType.
    /// \pre The new discriminator value must be a valid label for the current selected value.
    void d(
            int64_t disc)
    {
        xtypes_assert(type_->kind() == TypeKind::UNION_TYPE, "discriminator is only available for UnionType.");
        UnionType& un = const_cast<UnionType&>(static_cast<const UnionType&>(*type_));
        un.select_disc(instance_, disc);
    }

    /// \brief Modifies the discriminator of an Union.
    /// Doesn't allow to change the current active member. That must be done through the 'operator[](std::string)'.
    /// \pre The DynamicData must represent an UnionType.
    /// \pre The new discriminator value must be a valid label for the current selected value.
    void d(
            ReadableDynamicDataRef disc)
    {
        xtypes_assert(type_->kind() == TypeKind::UNION_TYPE, "discriminator is only available for UnionType.");
        UnionType& un = const_cast<UnionType&>(static_cast<const UnionType&>(*type_));
        un.select_disc(disc.type(), instance_, p_instance(disc));
    }

    /// \brief index access operator by DynamicData.
    /// \param[in] data DynamicData representing a MapType key.
    /// \pre The DynamicData must represent a MapType.
    /// If the key doesn't exists, creates an unitilized entry, adding the key.
    /// \returns A writable reference of the DynamicData accessed.
    WritableDynamicDataRef operator [] (
            ReadableDynamicDataRef data)
    {
        xtypes_assert(
            type_->kind() == TypeKind::MAP_TYPE,
            "operator[const DynamicData&] is only available for MapType.");

        const MapType& map = static_cast<const MapType&>(*type_);
        const PairType& pair = static_cast<const PairType&>(map.content_type());
        uint8_t* instance = map.get_instance_at(instance_, p_instance(data));
        if (instance == nullptr)
        {
            instance = map.insert_instance(instance_, p_instance(data));
            xtypes_assert(instance != nullptr, "Cannot insert new element into map.");
        }
        return WritableDynamicDataRef(pair.second(), instance + pair.first().memory_size());
    }

    /// \brief key access method by DynamicData.
    /// \param[in] data DynamicData representing a MapType key.
    /// \pre The DynamicData must represent a MapType.
    /// |pre The key must exists.
    /// \returns A writable reference of the DynamicData accessed.
    WritableDynamicDataRef at (
            ReadableDynamicDataRef data)
    {
        return ReadableDynamicDataRef::at(data);
    }

    /// \brief inserts a pair key, value into the DynamicData.
    /// \param[in] data DynamicData representing a PairType key.
    /// \pre The DynamicData must represent a MapType.
    /// If the key already exists, or the map reached its limits, returns false.
    /// \returns If the insertion succeded.
    bool insert (
            ReadableDynamicDataRef data) const
    {
        xtypes_assert(
            type_->kind() == TypeKind::MAP_TYPE,
            "insert method is only available for MapType.");

        const MapType& map = static_cast<const MapType&>(*type_);
        xtypes_assert(map.content_type().is_compatible(data.type()) == TypeConsistency::EQUALS, "Types doesn't match");
        return map.insert_instance(instance_, p_instance(data)) != nullptr;
    }

    /// \brief checks if a key is contained in the map represented by the DynamicData.
    /// \pre The DynamicData must represent a MapType.
    bool has_key(
            ReadableDynamicDataRef key) const
    {
        xtypes_assert(
            type_->kind() == TypeKind::MAP_TYPE,
            "has_key method is only available for MapType.");

        const MapType& map = static_cast<const MapType&>(*type_);
        [[maybe_unused]] const PairType& pair = static_cast<const PairType&>(map.content_type());
        xtypes_assert(pair.first().is_compatible(key.type()) == TypeConsistency::EQUALS, "Key types doesn't match.");
        return map.has_key(instance_, p_instance(key));
    }

    /// \brief Set a primitive or string value into the DynamicData
    /// \input[in] t The primitive, enumerated or string value.
    /// \pre The DynamicData must represent a PrimitiveType, EnumeratedType or W/StringType value.
    template<typename T, class = PrimitiveOrString<T> >
    void value(
            const T& t)
    {
        xtypes_assert((type_->kind() == TypeKind::STRING_TYPE && std::is_same<std::string, T>::value)
                || (type_->kind() == TypeKind::WSTRING_TYPE && std::is_same<std::wstring, T>::value)
                || (type_->kind() == TypeKind::STRING16_TYPE && std::is_same<std::u16string, T>::value)
                || (type_->kind() == PrimitiveTypeKindTrait<T>::kind)
                || (type_->is_enumerated_type()),
                "Expected type '" << type_->name()
                                  << "' but '" << PrimitiveTypeKindTrait<T>::name << "' received while setting value.",
                                  true);

        if (type_->is_enumerated_type())
        {
            xtypes_assert(type_->memory_size() == sizeof(T),
                    "Incompatible types: '" << type_->name() << "' and '"
                                            << PrimitiveTypeKindTrait<T>::name << "'.");
            [[maybe_unused]] const EnumeratedType<T>& enum_type = static_cast<const EnumeratedType<T>&>(*type_);
            xtypes_assert(enum_type.is_allowed_value(t),
                    "Trying to set an invalid value for enumerated type '" << type_->name() << "'.");
        }

        type_->destroy_instance(instance_);
        type_->copy_instance(instance_, reinterpret_cast<const uint8_t*>(&t));
    }

    /// \brief Push a primitive or string value into the DynamicData that represents a SequenceType
    /// \input[in] t The primitive or string value.
    /// \pre The DynamicData must represent a SequenceType.
    /// \pre The sequence must have enough space to place this element or be unbounded.
    /// \returns The writable reference to this DynamicData
    template<typename T, class = PrimitiveOrString<T> >
    WritableDynamicDataRef& push(
            const T& t)                      // this = SequenceType
    {
        xtypes_assert(type_->kind() == TypeKind::SEQUENCE_TYPE,
                "push() is only available for sequence types but called for '" << type_->name() << "'.");
        const SequenceType& sequence = static_cast<const SequenceType&>(*type_);
        xtypes_assert((sequence.content_type().kind() == TypeKind::STRING_TYPE && std::is_same<std::string, T>::value)
                || (sequence.content_type().kind() == TypeKind::WSTRING_TYPE && std::is_same<std::wstring, T>::value)
                || (sequence.content_type().kind() == TypeKind::STRING16_TYPE && std::is_same<std::u16string, T>::value)
                || (sequence.content_type().kind() == primitive_type<T>().kind()),
                "Expected type '" << static_cast<const SequenceType&>(*type_).content_type().name()
                                  << "' but '" << PrimitiveTypeKindTrait<T>::name << "' received while pushing value.");

        uint8_t* element = sequence.push_instance(instance_, reinterpret_cast<const uint8_t*>(&t));
        xtypes_assert(element != nullptr, "Bound limit reached while pushing value."); (void) element;
        return *this;
    }

    /// \brief Push another DynamicData into the DynamicData that represents a SequenceType
    /// \input[in] data DynamicData to add into the sequence
    /// \pre The DynamicData must represent a SequenceType.
    /// \pre The sequence must have enough space to place this element or be unbounded.
    /// \returns The writable reference to this DynamicData
    WritableDynamicDataRef& push(
            const ReadableDynamicDataRef& data)                      // this = SequenceType
    {
        xtypes_assert(type_->kind() == TypeKind::SEQUENCE_TYPE,
                "push() is only available for sequence types but called for '" << type_->name() << "'.");
        const SequenceType& sequence = static_cast<const SequenceType&>(*type_);

        uint8_t* element = sequence.push_instance(instance_, p_instance(data));
        xtypes_assert(element != nullptr, "Bound limit reached while pushing value."); (void) element;
        return *this;
    }

    /// \brief resize the Sequence representing by the DynamicData.
    /// If size is less or equals that the current size, nothing happens,
    /// otherwise a default-initialized values are insert to the sequence to increase its size.
    /// \param[int] size New sequence size
    /// \pre The DynamicData must represent a SequenceType.
    /// \pre The bounds must be greater or equal to the new size.
    /// \returns The writable reference to this DynamicData
    WritableDynamicDataRef& resize(
            size_t size)                        // this = SequenceType
    {
        xtypes_assert(type_->kind() == TypeKind::SEQUENCE_TYPE,
                "resize() is only available for sequence types but called for '" << type_->name() << "'.");
        [[maybe_unused]] size_t bound = bounds();
        xtypes_assert(!bound || bound >= size,
                "The desired size (" << size << ") is bigger than maximum allowed size for the type '"
                                     << type_->name() << "' (" << bounds() << ").");
        const SequenceType& sequence = static_cast<const SequenceType&>(*type_);
        sequence.resize_instance(instance_, size);
        return *this;
    }

    /// \brief (See ReadableDynamicData::for_each())
    bool for_each(
            std::function<void(const ReadableNode& node)> visitor) const
    {
        return ReadableDynamicDataRef::for_each(visitor);
    }

    /// \brief Class used by for_each() function to represent a writable DynamicData node in the tree.
    class WritableNode : public ReadableNode
    {
    public:

        WritableNode(
                const Instanceable::InstanceNode& instance_node)
            : ReadableNode(instance_node)
        {
        }

        /// \brief See ReadableNode::data()
        /// \returns A writable reference of the data.
        WritableDynamicDataRef data()
        {
            return ReadableNode::data();
        }

    };

    /// \brief (See ReadableDynamicData::for_each())
    /// A writable specialization of ReadableDynamicDataRef::for_each() function.
    bool for_each(
            std::function<void(WritableNode& node)> visitor)
    {
        Instanceable::InstanceNode root(*type_, instance_);
        try
        {
            type_->for_each_instance(root, [&](const Instanceable::InstanceNode& instance_node)
                    {
                        WritableNode node(instance_node);
                        visitor(node);
                    });
            return true;
        }
        catch (bool value)
        {
            return value;
        }
    }

    /// \brief Class used for iterate WritableDynamicDataRef
    class Iterator : public ReadableDynamicDataRef::Iterator
    {
    public:

        Iterator(
                const ReadableDynamicDataRef::Iterator& rit)
            : ReadableDynamicDataRef::Iterator (rit)
        {
        }

        WritableDynamicDataRef operator * ()
        {
            return ReadableDynamicDataRef::Iterator::operator *();
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
        {
        }

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
        {
        }

        MemberPair operator * ()
        {
            const AggregationType& aggregation = static_cast<const AggregationType&>(*type_);
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
        {
        }

    };

    /// \brief Returns an iterable representation of an aggregation dynamic data.
    /// \pre The DynamicData must represent an AggregationType.
    /// \returns An iterable representation of an aggregation dynamic data.
    MemberIterator items()
    {
        xtypes_assert(type_->is_aggregation_type(),
                "items() isn't available for type '" << type_->name() << "'.");
        return MemberIterator(*this, false);
    }

    /// \brief Returns a read-only iterable representation of an aggregation dynamic data.
    /// \pre The DynamicData must represent an AggregationType.
    /// \returns An iterable representation of an aggregation dynamic data.
    ReadableDynamicDataRef::MemberIterator citems()
    {
        xtypes_assert(type_->is_aggregation_type(),
                "citems() isn't available for type '" << type_->name() << "'.");
        return ReadableDynamicDataRef::MemberIterator(*this, false);
    }

protected:

    WritableDynamicDataRef(
            const DynamicType& type,
            uint8_t* source)
        : ReadableDynamicDataRef(type, source)
    {
    }

    /// \brief Internal cast from readable to writable
    WritableDynamicDataRef(
            const ReadableDynamicDataRef& other)
        : ReadableDynamicDataRef(other)
    {
    }

    WritableDynamicDataRef(
            ReadableDynamicDataRef&& other)
        : ReadableDynamicDataRef(std::move(other))
    {
    }

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
        memset(instance_, 0, type.memory_size());
        type_->construct_instance(instance_);
        initialize_ = true;
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
    /// \param[in] other A compatible DynamicData from which de data will be copied.
    /// \param[in] type DynamicType from which the DynamicData is created.
    DynamicData(
            const ReadableDynamicDataRef& other,
            const DynamicType& type)
        : WritableDynamicDataRef(type, new uint8_t[type.memory_size()])
    {
        xtypes_assert(type_->is_compatible(other.type()) != TypeConsistency::NONE,
                "Incompatible types in DynamicData(const ReadableDynamicDataRef&, const DynamicType&): '"
                << type_->name() << "' isn't compatible with '" << other.type().name() << "'.");
        memset(instance_, 0, type.memory_size());
        type_->copy_instance_from_type(instance_, p_instance(other), other.type());
        initialize_ = true;
    }

    /// \brief Copy constructor
    DynamicData(
            const DynamicData& other)
        : WritableDynamicDataRef(*other.type_, new uint8_t[other.type_->memory_size()])
    {
        memset(instance_, 0, other.type().memory_size());
        type_->copy_instance(instance_, p_instance(other));
        initialize_ = true;
    }

    /// \brief Move constructor
    DynamicData(
            DynamicData&& other)
        : WritableDynamicDataRef(*other.type_, new uint8_t[other.type_->memory_size()])
    {
        memset(instance_, 0, other.type().memory_size());
        type_->move_instance(instance_, p_instance(other), false);
        other.initialize_ = false;
    }

    /// \brief Assignment operator
    DynamicData& operator = (
            const DynamicData& other)
    {
        xtypes_assert(type_->is_compatible(other.type()) == TypeConsistency::EQUALS,
                "Cannot assign DynamicData of type '" << other.type().name() << "' to DynamicData of type '"
                                                      << type_->name() << "'.");
        type_->destroy_instance(instance_);
        type_->copy_instance(instance_, p_instance(other));
        return *this;
    }

    /// \brief Assignment operator
    DynamicData& operator = (
            ReadableDynamicDataRef other)
    {
        xtypes_assert(type_->is_compatible(other.type()) == TypeConsistency::EQUALS,
                "Cannot assign DynamicData of type '" << other.type().name() << "' to DynamicData of type '"
                                                      << type_->name() << "'.");
        type_->destroy_instance(instance_);
        type_->copy_instance(instance_, p_instance(other));
        return *this;
    }

    /// \brief See WritableDynamicDataRef::operator =()
    template<typename T, class = PrimitiveOrString<T> >
    WritableDynamicDataRef& operator = (
            const T& other)
    {
        return WritableDynamicDataRef::operator =(other);
    }

    /// \brief See WritableDynamicDataRef::operator =()
    WritableDynamicDataRef& operator = (
            const std::string& other)
    {
        return WritableDynamicDataRef::operator =(other);
    }

    /// \brief See WritableDynamicDataRef::operator =()
    WritableDynamicDataRef& operator = (
            const std::wstring& other)
    {
        return WritableDynamicDataRef::operator =(other);
    }

    /// \brief See WritableDynamicDataRef::operator =()
    WritableDynamicDataRef& operator = (
            const std::u16string& other)
    {
        return WritableDynamicDataRef::operator =(other);
    }

    /// \brief Returns a new DynamicData with the sign changed.
    /// \pre The DynamicData must represent a numeric value.
    inline DynamicData operator - () const;

    /// \brief Returns a new DynamicData, bitwise negated.
    /// \pre The DynamicData must represent a numeric value.
    inline DynamicData operator ~ () const;

    /// \brief Pre-increments the current DynamicData in one unit.
    /// \pre The DynamicData must represent a numeric integer value.
    /// \return A reference to the incremented DynamicData.
    inline DynamicData& operator ++ ();

    /// \brief Post-increments the current DynamicData in one unit.
    /// \pre The DynamicData must represent a numeric integer value.
    /// \param[in] int Dummy value.
    /// returns The incremented DynamicData.
    inline DynamicData operator ++ (
            int);

    /// \brief Pre-decrements the current DynamicData in one unit.
    /// \pre The DynamicData must represent a numeric integer value.
    /// \return A reference to the decremented DynamicData.
    inline DynamicData& operator -- ();

    /// \brief Post-decrements the current DynamicData in one unit.
    /// \pre The DynamicData must represent a numeric integer value.
    /// \param[in] int Dummy value.
    /// returns The decremented DynamicData.
    inline DynamicData operator -- (
            int);

    /// \brief Performs logical NOT operation upon a DynamicData.
    /// \pre The DynamicData must represent a numeric or boolean value.
    /// \returns boolean NOT operation result.
    inline bool operator ! () const;

    /// \brief Performs logical AND operation between two DynamicData.
    /// \pre The DynamicData must represent logical or numeric values.
    /// \returns logical AND operation result.
    inline bool operator && (
            const ReadableDynamicDataRef&) const;

    /// \brief Performs logical OR operation between two DynamicData.
    /// \pre The DynamicData must represent logical or numeric values.
    /// \returns logical OR operation result.
    inline bool operator || (
            const ReadableDynamicDataRef&) const;

    /// \brief Performs logical LESS THAN operation between two DynamicData.
    /// \pre The DynamicData must represent logical or numeric values.
    /// \returns logical LESS THAN operation result.
    inline bool operator < (
            const ReadableDynamicDataRef&) const;

    /// \brief Performs logical GREATER THAN operation between two DynamicData.
    /// \pre The DynamicData must represent logical or numeric values.
    /// \returns logical GREATER THAN operation result.
    inline bool operator > (
            const ReadableDynamicDataRef&) const;

    /// \brief Performs logical LESS OR EQUAL THAN operation between two DynamicData.
    /// \pre The DynamicData must represent logical or numeric values.
    /// \returns logical LESS OR EQUAL THAN operation result.
    inline bool operator <= (
            const ReadableDynamicDataRef&) const;

    /// \brief Performs logical GREATER OR EQUAL THAN operation between two DynamicData.
    /// \pre The DynamicData must represent logical or numeric values.
    /// \returns logical GREATER OR EQUAL THAN operation result.
    inline bool operator >= (
            const ReadableDynamicDataRef&) const;

    inline DynamicData operator * (
            const ReadableDynamicDataRef&) const;
    inline DynamicData operator / (
            const ReadableDynamicDataRef&) const;
    inline DynamicData operator % (
            const ReadableDynamicDataRef&) const;
    inline DynamicData operator + (
            const ReadableDynamicDataRef&) const;
    inline DynamicData operator - (
            const ReadableDynamicDataRef&) const;
    inline DynamicData operator << (
            const ReadableDynamicDataRef&) const;
    inline DynamicData operator >> (
            const ReadableDynamicDataRef&) const;
    inline DynamicData operator & (
            const ReadableDynamicDataRef&) const;
    inline DynamicData operator ^ (
            const ReadableDynamicDataRef&) const;
    inline DynamicData operator | (
            const ReadableDynamicDataRef&) const;

    inline DynamicData& operator *= (
            const ReadableDynamicDataRef&);
    inline DynamicData& operator /= (
            const ReadableDynamicDataRef&);
    inline DynamicData& operator %= (
            const ReadableDynamicDataRef&);
    inline DynamicData& operator += (
            const ReadableDynamicDataRef&);
    inline DynamicData& operator -= (
            const ReadableDynamicDataRef&);
    inline DynamicData& operator <<= (
            const ReadableDynamicDataRef&);
    inline DynamicData& operator >>= (
            const ReadableDynamicDataRef&);
    inline DynamicData& operator &= (
            const ReadableDynamicDataRef&);
    inline DynamicData& operator ^= (
            const ReadableDynamicDataRef&);
    inline DynamicData& operator |= (
            const ReadableDynamicDataRef&);
    template <typename B>
    using isBaseDynamicDataCRef = std::enable_if<!std::is_base_of<ReadableDynamicDataRef, B>::value>;

    template <typename T, typename = typename isBaseDynamicDataCRef<T>::type>
    inline DynamicData& operator *= (
            const T&);
    template <typename T, typename = typename isBaseDynamicDataCRef<T>::type>
    inline DynamicData& operator /= (
            const T&);
    template <typename T, typename = typename isBaseDynamicDataCRef<T>::type>
    inline DynamicData& operator %= (
            const T&);
    template <typename T, typename = typename isBaseDynamicDataCRef<T>::type>
    inline DynamicData& operator += (
            const T&);
    template <typename T, typename = typename isBaseDynamicDataCRef<T>::type>
    inline DynamicData& operator -= (
            const T&);
    template <typename T, typename = typename isBaseDynamicDataCRef<T>::type>
    inline DynamicData& operator <= (
            const T&);
    template <typename T, typename = typename isBaseDynamicDataCRef<T>::type>
    inline DynamicData& operator >= (
            const T&);
    template <typename T, typename = typename isBaseDynamicDataCRef<T>::type>
    inline DynamicData& operator <<= (
            const T&);
    template <typename T, typename = typename isBaseDynamicDataCRef<T>::type>
    inline DynamicData& operator >>= (
            const T&);
    template <typename T, typename = typename isBaseDynamicDataCRef<T>::type>
    inline DynamicData& operator &= (
            const T&);
    template <typename T, typename = typename isBaseDynamicDataCRef<T>::type>
    inline DynamicData& operator ^= (
            const T&);
    template <typename T, typename = typename isBaseDynamicDataCRef<T>::type>
    inline DynamicData& operator |= (
            const T&);

    virtual ~DynamicData() override
    {
        if(initialize_)
        {
            type_->destroy_instance(instance_);
        }
        delete[] instance_;
    }

    /// \brief Request a writable reference from this DynamicData.
    /// \returns a WritableDynamicDataRef identifying the DynamicData.
    WritableDynamicDataRef ref() const
    {
        return WritableDynamicDataRef(*this);
    }

};

/// \brief Ostream operator overload.
/// \pre The DynamicData to be sent to ostream.
inline std::ostream& operator << (
        std::ostream& os,
        const DynamicData& data)
{
    os << data.to_string();
    return os;
}

} //namespace xtypes
} //namespace eprosima

#include <xtypes/DynamicDataImpl.hpp>

#endif //EPROSIMA_XTYPES_DYNAMIC_DATA_HPP_
