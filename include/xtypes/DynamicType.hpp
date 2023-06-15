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

#ifndef EPROSIMA_XTYPES_DYNAMIC_TYPE_HPP_
#define EPROSIMA_XTYPES_DYNAMIC_TYPE_HPP_

#include <xtypes/Assert.hpp>
#include <xtypes/Instanceable.hpp>
#include <xtypes/TypeKind.hpp>
#include <xtypes/TypeConsistency.hpp>

#include <string>
#include <memory>

namespace eprosima {
namespace xtypes {

class ReadableDynamicDataRef;
class SequenceInstance;

/// \brief Abstract base class for all dynamic types.
class DynamicType : public Instanceable,
                    public std::enable_shared_from_this<DynamicType>
{
public:

    virtual ~DynamicType() = default;

    /// \brief Name of the DynamicType.
    /// \returns DynamicType's name.
    const std::string& name() const
    {
        return name_;
    }

    /// \brief type kind this DynamicType. (see TypeKind)
    /// \returns type kind corresponding to this DynamicType
    const TypeKind& kind() const
    {
        return kind_;
    }

    /// \brief check if this type is primitive
    /// (has the corresponding bit of TypeKind::PRIMITIVE_TYPE in its kind).
    /// \returns true if is primitive
    bool is_primitive_type() const
    {
        return (kind_& TypeKind::PRIMITIVE_TYPE) != TypeKind::NO_TYPE;
    }

    /// \brief check if this type is a collection
    /// (has the corresponding bit of TypeKind::COLLECTION_TYPE in its kind).
    /// \returns true if is a collection
    bool is_collection_type() const
    {
        return (kind_& TypeKind::COLLECTION_TYPE) != TypeKind::NO_TYPE;
    }

    /// \brief check if this type is an aggregation
    /// (has the corresponding bit of TypeKind::AGGREGATION_TYPE in its kind).
    /// \returns true if is an aggregation
    bool is_aggregation_type() const
    {
        return (kind_& TypeKind::AGGREGATION_TYPE) != TypeKind::NO_TYPE;
    }

    /// \brief check if this type is constructed
    /// (has the corresponding bit of TypeKind::CONSTRUCTED_TYPE in its kind).
    /// \returns true if is constructed
    bool is_constructed_type() const
    {
        return (kind_& TypeKind::CONSTRUCTED_TYPE) != TypeKind::NO_TYPE;
    }

    /// \brief check if this type is enumerated
    /// (has the corresponding bit of TypeKind::ENUMERATED_TYPE in its kind).
    /// \returns true if is enumerated
    bool is_enumerated_type() const
    {
        return (kind_& TypeKind::ENUMERATED_TYPE) != TypeKind::NO_TYPE;
    }

    /// \brief check the compatibility with other DynamicType.
    /// returns The needed consistency required for the types can be compatibles.
    ///   TypeConsistency::EQUALS means that the types are identically.
    ///   TypeConsistency::NONE means that no conversion is known to be compatibles both types.
    ///   Otherwise, a level of consistency was found for convert both types between them.
    ///   See TypeConsistency for more information.
    virtual TypeConsistency is_compatible(
            const DynamicType& other) const = 0;

    /// \brief Internal structure used to iterate the DynamicType tree.
    class TypeNode
    {
        const TypeNode* parent_;
        const DynamicType& type_;
        size_t deep_;
        size_t from_index_;
        const Member* from_member_;

public:

        const TypeNode& parent() const
        {
            return *parent_;
        }

        bool has_parent() const
        {
            return parent_ != nullptr;
        }

        const DynamicType& type() const
        {
            return type_;
        }

        size_t deep() const
        {
            return deep_;
        }

        size_t from_index() const
        {
            return from_index_;
        }

        const Member* from_member() const
        {
            return from_member_;
        }

        TypeNode(
                const DynamicType& type)
            : parent_(nullptr)
            , type_(type)
            , deep_(0)
            , from_index_(0)
            , from_member_(nullptr)
        {
        }

        TypeNode(
                const TypeNode& parent,
                const DynamicType& type,
                size_t from_index,
                const Member* from_member)
            : parent_(&parent)
            , type_(type)
            , deep_(parent.deep_ + 1)
            , from_index_(from_index)
            , from_member_(from_member)
        {
        }

    };

    using TypeVisitor = std::function<void (const TypeNode& node)>;

    /// \brief Function used to iterate the DynamicType tree.
    /// The iteration will go through the tree in deep, calling the visitor function for each type.
    /// \param[in] node Relative information about the current type iteration.
    /// \param[in] visitor Function called each time a new node in the tree is visited.
    virtual void for_each_type(
            const TypeNode& node,
            TypeVisitor visitor) const = 0;

    /// \brief Iterate the DynamicType in deep. Each node visited will call to the user visitor function.
    /// \param[in] visitor User visitor function.
    /// \returns true if no exceptions by the user were throw. Otherwise, the user boolean exception value.
    bool for_each(
            TypeVisitor visitor) const
    {
        TypeNode root(*this);
        try
        {
            for_each_type(root, visitor);
            return true;
        }
        catch (bool value){ return value; }
    }

    /// \brief Set the name of the DynamicType.
    /// param[in] name New name for the dynamic type
    void name(
            const std::string& name)
    {
        name_ = name;
    }

protected:

    DynamicType(
            TypeKind kind,
            const std::string& name)
        : kind_(kind)
        , name_(name)
    {
    }

    DynamicType(
            const DynamicType& other) = default;
    DynamicType(
            DynamicType&& other) = default;

    /// \brief Deep clone of the DynamicType.
    /// \returns a new DynamicType without managing.
    virtual std::shared_ptr<DynamicType> clone() const = 0;

    TypeKind kind_;
    std::string name_;

    friend class ReadableDynamicDataRef;
    friend class SequenceInstance;

public:

    /// \brief Special managed pointer for DynamicTypes.
    /// It performs some performances to avoid copies for some internal types.
    class Ptr
    {
public:

        /// \brief Default initialization without pointer any type.
        Ptr() = default;

        /// \brief Creates a copy of a DynamicType that will be managed.
        /// The copy is avoid if DynamnicType is primitive.
        Ptr(const DynamicType& type)
        {
            try
            {
                if(dont_clone_type(&type))
                {
                    type_ = type.shared_from_this();
                    return;
                }
            }
            catch(const std::bad_weak_ptr&) {}

            // make a copy
            type_ = type.clone();
        }

        /// \brief Copy constructor.
        /// Makes an internal copy of the managed DynamicType.
        /// The copy is avoid if DynamicType is primitive.
        Ptr(const Ptr& ptr)
        {
            if(!ptr.type_)
                return;

            new (this) Ptr(*ptr.type_);
        }

        Ptr(Ptr&& ptr) = default;

        Ptr& operator = (
                const Ptr& ptr)
        {
            if (type_ == ptr.type_)
            {
                return *this;
            }

            type_ = dont_clone_type(ptr.type_.get()) ? ptr.type_ : ptr.type_->clone();
            return *this;
        }

        Ptr& operator = ( Ptr&& ptr) = default;

        bool operator == (
                const DynamicType::Ptr& ptr) const
        {
            return ptr.type_ == type_;
        }

        /// \brief Free the internal managed DynamicType and points to nothing.
        std::shared_ptr<const DynamicType> free()
        {
            std::shared_ptr<const DynamicType> freed;
            type_.swap(freed);
            return freed;
        }

        /// \brief Returns a pointer of the internal managed DynamicType.
        /// \returns A pointer of the internal managed DynamicType.
        const DynamicType* get() const
        {
            return type_.get();
        }

        /// \brief Returns a pointer of the internal managed DynamicType.
        /// \returns A pointer of the internal managed DynamicType.
        const DynamicType* operator ->() const
        {
            return type_.get();
        }

        /// \brief Get a non-const pointer of the internal managed DynamicType.
        /// \returns A non-const pointer of the interal managed DynamicType
        DynamicType* operator ->()
        {
            return const_cast<DynamicType*>(type_.get());
        }

        /// \brief Returns a reference of the intenral managed DynamicType.
        /// \returns A reference of the internal managed DynamicType.
        const DynamicType& operator *() const
        {
            return *type_;
        }

private:

        std::shared_ptr<const DynamicType> type_;

        static bool dont_clone_type(
                const DynamicType* type)
        {
            return
                type == nullptr ||
                (type->is_primitive_type() && !type->is_enumerated_type());
        }

    };
};

} //namespace xtypes
} //namespace eprosima

#endif //EPROSIMA_XTYPES_DYNAMIC_TYPE_HPP_
