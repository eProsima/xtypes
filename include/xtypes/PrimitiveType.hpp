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

#ifndef EPROSIMA_XTYPES_PRIMITIVE_TYPE_HPP_
#define EPROSIMA_XTYPES_PRIMITIVE_TYPE_HPP_

#include <xtypes/DynamicType.hpp>
#include <xtypes/AliasType.hpp>
#include <xtypes/StructType.hpp>

#include <cstring>

namespace eprosima {
namespace xtypes {

/// \brief Internal struct used for enable primitive types
template<typename>
struct PrimitiveTypeKindTrait
{
    static constexpr TypeKind kind = TypeKind::NO_TYPE;
    static constexpr const char* name = "no_type";
};

#define DDS_CORE_XTYPES_PRIMITIVE(TYPE, KIND) \
    template<> \
    struct PrimitiveTypeKindTrait<TYPE> \
    { \
        static constexpr TypeKind kind = TypeKind::KIND; \
        static constexpr const char* name = #TYPE; \
    }; \

DDS_CORE_XTYPES_PRIMITIVE(bool, BOOLEAN_TYPE)
DDS_CORE_XTYPES_PRIMITIVE(int8_t, INT_8_TYPE)
DDS_CORE_XTYPES_PRIMITIVE(uint8_t, UINT_8_TYPE)
DDS_CORE_XTYPES_PRIMITIVE(int16_t, INT_16_TYPE)
DDS_CORE_XTYPES_PRIMITIVE(uint16_t, UINT_16_TYPE)
DDS_CORE_XTYPES_PRIMITIVE(int32_t, INT_32_TYPE)
DDS_CORE_XTYPES_PRIMITIVE(uint32_t, UINT_32_TYPE)
DDS_CORE_XTYPES_PRIMITIVE(int64_t, INT_64_TYPE)
DDS_CORE_XTYPES_PRIMITIVE(uint64_t, UINT_64_TYPE)
DDS_CORE_XTYPES_PRIMITIVE(float, FLOAT_32_TYPE)
DDS_CORE_XTYPES_PRIMITIVE(double, FLOAT_64_TYPE)
DDS_CORE_XTYPES_PRIMITIVE(long double, FLOAT_128_TYPE)
DDS_CORE_XTYPES_PRIMITIVE(char, CHAR_8_TYPE)
DDS_CORE_XTYPES_PRIMITIVE(char16_t, CHAR_16_TYPE)
DDS_CORE_XTYPES_PRIMITIVE(wchar_t, WIDE_CHAR_TYPE)

#define DDS_CORE_XTYPES_PRIMITIVE_ALIASES(ALIAS_TYPE, TYPE) \
    template<> \
    struct PrimitiveTypeKindTrait<ALIAS_TYPE> \
    { \
        static constexpr TypeKind kind = PrimitiveTypeKindTrait<TYPE>::kind; \
        static constexpr const char* name = PrimitiveTypeKindTrait<TYPE>::name; \
    }; \

// Platform specific workarounds (stdint.h may miss some typedefs)
#ifdef _MSC_VER
DDS_CORE_XTYPES_PRIMITIVE_ALIASES(long, int32_t)
DDS_CORE_XTYPES_PRIMITIVE_ALIASES(unsigned long, uint32_t)
#endif

/// \brief DynamicType representing a primitive type.
/// Primitive types can be the following: bool char wchar_t uint8_t int16_t
/// uint16_t int32_t uint32_t int64_t uint64_t float double long double.
/// A PrimitiveType represents a TypeKind::PRIMITIVE_TYPE.
template<typename T>
class PrimitiveType : public DynamicType
{
protected:
    // Avoid direct use of the class (there is a factory)
    // Allow the use in subclasses
    struct use_function_primitive_type {};

public:

    PrimitiveType(use_function_primitive_type)
        : DynamicType(PrimitiveTypeKindTrait<T>::kind, PrimitiveTypeKindTrait<T>::name)
    {
    }

    PrimitiveType(
            use_function_primitive_type,
            TypeKind kind,
            const std::string& name)
        : DynamicType(kind, name)
    {
    }

protected:

    template<typename R>
    friend const DynamicType& primitive_type();

    PrimitiveType(
            const PrimitiveType& other) = delete;
    PrimitiveType(
            PrimitiveType&& other) = delete;

    virtual size_t memory_size() const override
    {
        return sizeof(T);
    }

    virtual void construct_instance(
            uint8_t* instance) const override
    {
        *reinterpret_cast<T*>(instance) = T(0);
    }

    virtual void destroy_instance(
            uint8_t* /*instance*/) const override
    { //Default does nothing
    }

    virtual void copy_instance(
            uint8_t* target,
            const uint8_t* source) const override
    {
        *reinterpret_cast<T*>(target) = *reinterpret_cast<const T*>(source);
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

        xtypes_assert(other.is_primitive_type() || other.is_enumerated_type(),
                "Cannot copy data from type '" + other.name() + "' to type '" + name() + "'.");

        (void) other;
        switch (other.kind())
        {
            case TypeKind::BOOLEAN_TYPE:
                *reinterpret_cast<T*>(target) = *reinterpret_cast<const bool*>(source);
                break;
            case TypeKind::INT_8_TYPE:
                *reinterpret_cast<T*>(target) = *reinterpret_cast<const int8_t*>(source);
                break;
            case TypeKind::UINT_8_TYPE:
                *reinterpret_cast<T*>(target) = *reinterpret_cast<const uint8_t*>(source);
                break;
            case TypeKind::UINT_16_TYPE:
                *reinterpret_cast<T*>(target) = *reinterpret_cast<const uint16_t*>(source);
                break;
            case TypeKind::UINT_32_TYPE:
            case TypeKind::ENUMERATION_TYPE:
                *reinterpret_cast<T*>(target) = *reinterpret_cast<const uint32_t*>(source);
                break;
            case TypeKind::UINT_64_TYPE:
                *reinterpret_cast<T*>(target) = *reinterpret_cast<const uint64_t*>(source);
                break;
            case TypeKind::INT_16_TYPE:
                *reinterpret_cast<T*>(target) = *reinterpret_cast<const int16_t*>(source);
                break;
            case TypeKind::INT_32_TYPE:
                *reinterpret_cast<T*>(target) = *reinterpret_cast<const int32_t*>(source);
                break;
            case TypeKind::INT_64_TYPE:
                *reinterpret_cast<T*>(target) = *reinterpret_cast<const int64_t*>(source);
                break;
            case TypeKind::CHAR_8_TYPE:
                *reinterpret_cast<T*>(target) = *reinterpret_cast<const char*>(source);
                break;
            case TypeKind::CHAR_16_TYPE:
                *reinterpret_cast<T*>(target) = *reinterpret_cast<const char16_t*>(source);
                break;
            case TypeKind::WIDE_CHAR_TYPE:
                *reinterpret_cast<T*>(target) = *reinterpret_cast<const wchar_t*>(source);
                break;
            case TypeKind::FLOAT_32_TYPE:
                *reinterpret_cast<T*>(target) = *reinterpret_cast<const float*>(source);
                break;
            case TypeKind::FLOAT_64_TYPE:
                *reinterpret_cast<T*>(target) = *reinterpret_cast<const double*>(source);
                break;
            case TypeKind::FLOAT_128_TYPE:
                *reinterpret_cast<T*>(target) = *reinterpret_cast<const long double*>(source);
                break;
            default:
                xtypes_assert(false, "Primitive DynamicData of an unknown type: '" << name() << "'."); //Must not reached
        }
    }

    virtual void move_instance(
            uint8_t* target,
            uint8_t* source,
            bool) const override
    {
        copy_instance(target, source);
    }

    virtual bool compare_instance(
            const uint8_t* instance,
            const uint8_t* other_instance) const override
    {
        return *reinterpret_cast<const T*>(instance) == *reinterpret_cast<const T*>(other_instance);
    }

    virtual TypeConsistency is_compatible(
            const DynamicType& other) const override
    {
        if (other.kind() == TypeKind::ALIAS_TYPE)
        {
            const AliasType& other_alias = static_cast<const AliasType&>(other);
            return other_alias.is_compatible(*this);
        }

        if (other.kind() == TypeKind::STRUCTURE_TYPE) // Resolve one-member structs
        {
            return other.is_compatible(*this);
        }

        if (!other.is_primitive_type())
        {
            return TypeConsistency::NONE;
        }

        if (kind() == other.kind())
        {
            return TypeConsistency::EQUALS;
        }

        TypeConsistency consistency = TypeConsistency::EQUALS;
        if (memory_size() != other.memory_size())
        {
            consistency |= TypeConsistency::IGNORE_TYPE_WIDTH;
        }

        if ((kind() & TypeKind::UNSIGNED_TYPE) != (other.kind() & TypeKind::UNSIGNED_TYPE))
        {
            consistency |= TypeConsistency::IGNORE_TYPE_SIGN;
        }
        return consistency;
    }

    virtual void for_each_instance(
            const InstanceNode& node,
            InstanceVisitor visitor) const override
    {
        visitor(node);
    }

    virtual void for_each_type(
            const TypeNode& node,
            TypeVisitor visitor) const override
    {
        visitor(node);
    }

    std::shared_ptr<DynamicType> clone() const override
    {
        return std::make_shared<PrimitiveType<T>>(use_function_primitive_type{}, this->kind(), this->name());

    }
};

/// \brief Helper function to create a PrimitiveType.
/// The creation of a PrimitiveType
/// must be always done by this function.
/// \returns A DynamicType representing a PrimitiveType<T>
template<typename T>
const DynamicType& primitive_type()
{
    // The creation of PrimitiveType must be always done
    // by this function in order to not broken the DynamicType::Ptr
    // optimizations for PrimitiveType
    static PrimitiveType<T> p(typename PrimitiveType<T>::use_function_primitive_type{});
    return p;
}

} //namespace xtypes
} //namespace eprosima

#endif //EPROSIMA_XTYPES_PRIMITIVE_TYPE_HPP_
